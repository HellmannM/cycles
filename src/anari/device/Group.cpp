// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Group.h"
// cycles
#include "scene/object.h"

namespace cycles {

Group::Group(CyclesGlobalState *s) : Object(ANARI_GROUP, s)
{
  s->objectCounts.groups++;
}

Group::~Group()
{
  cleanup();
  deviceState()->objectCounts.groups--;
}

void Group::commit()
{
  cleanup();
  m_surfaceData = getParamObject<ObjectArray>("surface");
  m_lightData = getParamObject<ObjectArray>("light");

  if (m_surfaceData)
    m_surfaceData->addCommitObserver(this);
  if (m_lightData)
    m_lightData->addCommitObserver(this);
}

void Group::addGroupToCurrentWorld(const ccl::Transform &xfm) const
{
  auto &state = *deviceState();

  if (m_surfaceData) {
    auto **surfacesBegin = (Surface **)m_surfaceData->handlesBegin();
    auto **surfacesEnd = (Surface **)m_surfaceData->handlesEnd();

    std::for_each(surfacesBegin, surfacesEnd, [&](Surface *s) {
      if (!s->isValid()) {
        s->warnIfUnknownObject();
        return;
      }

      auto *g = s->makeCyclesGeometry();
      g->tag_update(state.scene, true);
      state.scene->geometry.push_back(g);

      auto *o = new ccl::Object();
      o->set_geometry(g);
      o->set_tfm(xfm);
      state.scene->objects.push_back(o);
    });
  }

  if (m_lightData) {
    auto **lightsBegin = (Light **)m_lightData->handlesBegin();
    auto **lightsEnd = (Light **)m_lightData->handlesEnd();

    std::for_each(lightsBegin, lightsEnd, [&](Light *l) {
      if (!l->isValid()) {
        l->warnIfUnknownObject();
        return;
      }

      auto *cl = l->cyclesLight();
      state.scene->lights.push_back(cl);
      cl->tag_update(state.scene);
    });
  }
}

box3 Group::bounds() const
{
  box3 b = empty_box3();
  if (m_surfaceData) {
    auto **surfacesBegin = (Surface **)m_surfaceData->handlesBegin();
    auto **surfacesEnd = (Surface **)m_surfaceData->handlesEnd();

    std::for_each(surfacesBegin, surfacesEnd, [&](Surface *s) {
      if (s->isValid())
        extend(b, s->geometry()->bounds());
      else
        s->warnIfUnknownObject();
    });
  }
  return b;
}

void Group::cleanup()
{
  if (m_surfaceData)
    m_surfaceData->removeCommitObserver(this);
  if (m_lightData)
    m_lightData->removeCommitObserver(this);
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Group *);
