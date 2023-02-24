// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Surface.h"

namespace cycles {

Surface::Surface(CyclesGlobalState *s) : Object(ANARI_SURFACE, s)
{
  s->objectCounts.surfaces++;

  m_cyclesObject = std::make_unique<ccl::Object>();
}

Surface::~Surface()
{
  deviceState()->objectCounts.surfaces--;
}

void Surface::commit()
{
  m_cyclesGeometry.reset();

  m_geometry = getParamObject<Geometry>("geometry");
  m_material = getParamObject<Material>("material");

  if (!m_geometry)
    reportMessage(ANARI_SEVERITY_WARNING, "missing 'geometry' on ANARISurface");

  if (!m_material)
    reportMessage(ANARI_SEVERITY_WARNING, "missing 'material' on ANARISurface");

  if (!m_geometry || !m_material)
    return;

  m_cyclesGeometry.reset(m_geometry->makeCyclesGeometry());

  ccl::array<ccl::Node *> used_shaders;
  used_shaders.push_back_slow(m_material->cyclesShader());
  m_cyclesGeometry->set_used_shaders(used_shaders);

  m_cyclesObject->set_geometry(cyclesGeometry());
  m_cyclesObject->set_tfm(ccl::transform_identity());
}

const Geometry *Surface::geometry() const
{
  return m_geometry.ptr;
}

const Material *Surface::material() const
{
  return m_material.ptr;
}

ccl::Geometry *Surface::cyclesGeometry() const
{
  return m_cyclesGeometry.get();
}

ccl::Object *Surface::cyclesObject() const
{
  return m_cyclesObject.get();
}

void Surface::markCommitted()
{
  Object::markCommitted();
  deviceState()->objectUpdates.lastBLSReconstructSceneRequest =
      helium::newTimeStamp();
}

bool Surface::isValid() const
{
  return m_geometry && m_geometry->isValid();
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Surface *);
