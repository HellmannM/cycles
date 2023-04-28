// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Surface.h"

namespace cycles {

Surface::Surface(CyclesGlobalState *s) : Object(ANARI_SURFACE, s)
{
  s->objectCounts.surfaces++;
}

Surface::~Surface()
{
  deviceState()->objectCounts.surfaces--;
}

void Surface::commit()
{
  m_geometry = getParamObject<Geometry>("geometry");
  m_material = getParamObject<Material>("material");

  if (!m_geometry)
    reportMessage(ANARI_SEVERITY_WARNING, "missing 'geometry' on ANARISurface");

  if (!m_material)
    reportMessage(ANARI_SEVERITY_WARNING, "missing 'material' on ANARISurface");

  if (!m_geometry || !m_material)
    return;
}

const Geometry *Surface::geometry() const
{
  return m_geometry.ptr;
}

const Material *Surface::material() const
{
  return m_material.ptr;
}

ccl::Geometry *Surface::makeCyclesGeometry()
{
  auto *g = m_geometry->makeCyclesGeometry();
  ccl::array<ccl::Node *> used_shaders;
  used_shaders.push_back_slow(m_material->cyclesShader());
  g->set_used_shaders(used_shaders);
  return g;
}

void Surface::markCommitted()
{
  Object::markCommitted();
  deviceState()->objectUpdates.lastBLSReconstructSceneRequest = helium::newTimeStamp();
}

bool Surface::isValid() const
{
  return m_geometry && m_geometry->isValid();
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Surface *);
