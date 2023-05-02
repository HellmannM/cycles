// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "World.h"
// std
#include <algorithm>
// cycles
#include "scene/object.h"

namespace cycles {

World::World(CyclesGlobalState *s) : Object(ANARI_WORLD, s)
{
  s->objectCounts.worlds++;

  m_zeroGroup = new Group(s);
  m_zeroInstance = new Instance(s);
  m_zeroInstance->setParamDirect("group", m_zeroGroup.ptr);

  // never any public ref to these objects
  m_zeroGroup->refDec(helium::RefType::PUBLIC);
  m_zeroInstance->refDec(helium::RefType::PUBLIC);

  m_zeroGroup->commit();
  m_zeroInstance->commit();
}

World::~World()
{
  cleanup();
  deviceState()->objectCounts.worlds--;
}

bool World::getProperty(const std::string_view &name,
                        ANARIDataType type,
                        void *ptr,
                        uint32_t flags)
{
  if (name == "bounds" && type == ANARI_FLOAT32_BOX3) {
    if (flags & ANARI_WAIT)
      deviceState()->commitBuffer.flush();
    auto b = bounds();
    anari_vec::float3 r[2];
    r[0] = {b.lower.x, b.lower.y, b.lower.z};
    r[1] = {b.upper.x, b.upper.y, b.upper.z};
    std::memcpy(ptr, &r[0], anari::sizeOf(type));
    return true;
  }

  return Object::getProperty(name, type, ptr, flags);
}

void World::commit()
{
  cleanup();
  m_zeroSurfaceData = getParamObject<ObjectArray>("surface");
  m_zeroLightData = getParamObject<ObjectArray>("light");

  const bool addZeroInstance = m_zeroSurfaceData || m_zeroLightData;

  if (addZeroInstance)
    reportMessage(ANARI_SEVERITY_DEBUG, "cycles::World will add zero instance");

  if (m_zeroSurfaceData) {
    reportMessage(ANARI_SEVERITY_DEBUG, "cycles::World found surfaces in zero instance");
    m_zeroGroup->setParamDirect("surface", getParamDirect("surface"));
  }
  else
    m_zeroGroup->removeParam("surface");

  if (m_zeroLightData) {
    reportMessage(ANARI_SEVERITY_DEBUG, "cycles::World found lights in zero instance");
    m_zeroGroup->setParamDirect("light", getParamDirect("light"));
  }
  else
    m_zeroGroup->removeParam("light");

  m_zeroGroup->commit();

  m_instanceData = getParamObject<ObjectArray>("instance");

  if (m_instanceData)
    m_instanceData->addCommitObserver(this);
  if (m_zeroSurfaceData)
    m_zeroSurfaceData->addCommitObserver(this);
  if (m_zeroLightData)
    m_zeroLightData->addCommitObserver(this);
}

void World::setWorldObjectsCurrent()
{
  auto &state = *deviceState();
  auto *scene = state.scene;

  for (auto *o : scene->objects)
    delete o;
  scene->objects.clear();

  for (auto *g : scene->geometry)
    delete g;
  scene->geometry.clear();

  scene->lights.clear();

  scene->object_manager->tag_update(scene, ObjectManager::UPDATE_ALL);
  scene->geometry_manager->tag_update(scene, GeometryManager::UPDATE_ALL);
  scene->light_manager->tag_update(scene, LightManager::UPDATE_ALL);

  m_zeroInstance->addInstanceObjectsToCurrentWorld();

  if (m_instanceData) {
    auto **instancesBegin = (Instance **)m_instanceData->handlesBegin();
    auto **instancesEnd = (Instance **)m_instanceData->handlesEnd();

    std::for_each(
        instancesBegin, instancesEnd, [](Instance *i) { i->addInstanceObjectsToCurrentWorld(); });
  }
}

box3 World::bounds() const
{
  box3 b = empty_box3();

  if (m_zeroSurfaceData)
    extend(b, m_zeroInstance->bounds());

  if (m_instanceData) {
    auto **instancesBegin = (Instance **)m_instanceData->handlesBegin();
    auto **instancesEnd = (Instance **)m_instanceData->handlesEnd();

    std::for_each(instancesBegin, instancesEnd, [&](Instance *i) { extend(b, i->bounds()); });
  }

  return b;
}

void World::cleanup()
{
  if (m_instanceData)
    m_instanceData->removeCommitObserver(this);
  if (m_zeroSurfaceData)
    m_zeroSurfaceData->removeCommitObserver(this);
  if (m_zeroLightData)
    m_zeroLightData->removeCommitObserver(this);
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::World *);
