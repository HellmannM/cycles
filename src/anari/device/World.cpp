// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "World.h"
// std
#include <algorithm>

namespace cycles {

World::World(CyclesGlobalState *s) : Object(ANARI_WORLD, s)
{
  s->objectCounts.worlds++;
}

World::~World()
{
  cleanup();
  deviceState()->objectCounts.worlds--;
}

void World::commit()
{
  cleanup();
  m_zeroSurfaceData = getParamObject<ObjectArray>("surface");
  m_zeroLightData = getParamObject<ObjectArray>("light");
}

void World::setWorldObjectsCurrent()
{
  auto &state = *deviceState();

  state.scene->geometry.clear();
  state.scene->objects.clear();
  state.scene->lights.clear();

  if (m_zeroSurfaceData) {
    auto **surfacesBegin = (Surface **)m_zeroSurfaceData->handlesBegin();
    auto **surfacesEnd = (Surface **)m_zeroSurfaceData->handlesEnd();

    std::for_each(surfacesBegin, surfacesEnd, [&](Surface *s) {
      if (!s->isValid())
        return;
      auto *g = s->cyclesGeometry();
      g->tag_update(state.scene, true);
      state.scene->geometry.push_back(g);

      auto *o = s->cyclesObject();
      state.scene->objects.push_back(o);
    });
  }

  if (m_zeroLightData) {
    auto **lightsBegin = (Light **)m_zeroLightData->handlesBegin();
    auto **lightsEnd = (Light **)m_zeroLightData->handlesEnd();

    std::for_each(lightsBegin, lightsEnd, [&](Light *l) {
      if (!l->isValid())
        return;
      auto *cl = l->cyclesLight();
      state.scene->lights.push_back(cl);
      cl->tag_update(state.scene);
    });
  }
}

void World::cleanup()
{
  if (m_zeroSurfaceData)
    m_zeroSurfaceData->removeCommitObserver(this);
}

}  // namespace cycles
