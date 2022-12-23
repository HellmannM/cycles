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
}

void World::setWorldObjectsCurrent()
{
  auto &state = *deviceState();

  state.scene->geometry.clear();
  state.scene->objects.clear();

  if (!m_zeroSurfaceData)
    return;

  auto **surfacesBegin = (Surface **)m_zeroSurfaceData->handlesBegin();
  auto **surfacesEnd = (Surface **)m_zeroSurfaceData->handlesEnd();

  if (!surfacesBegin)
    return;

  std::for_each(surfacesBegin, surfacesEnd, [&](Surface *s) {
    auto *g = s->cyclesGeometry();
    g->tag_update(state.scene, true);
    state.scene->geometry.push_back(g);

    auto *o = s->cyclesObject();
    state.scene->objects.push_back(o);
  });
}

void World::cleanup()
{
  if (m_zeroSurfaceData)
    m_zeroSurfaceData->removeCommitObserver(this);
}

}  // namespace cycles
