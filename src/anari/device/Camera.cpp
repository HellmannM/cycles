// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Camera.h"

namespace cycles {

Camera::Camera(CyclesGlobalState *s) : Object(ANARI_CAMERA, s)
{
  s->objectCounts.cameras++;
}

Camera::~Camera()
{
  deviceState()->objectCounts.cameras--;
}

Camera *Camera::createInstance(std::string_view type, CyclesGlobalState *s)
{
  if (type == "perspective")
    return new Perspective(s);
#if 0
  else if (type == "orthographic")
    return new Orthographic(s);
#endif
  else
    return (Camera *)new UnknownObject(ANARI_CAMERA, s);
}

void Camera::commit()
{
  m_pos = getParam<float3>("position", make_float3(0.f));
  m_dir = normalize(getParam<float3>("direction", make_float3(0.f, 0.f, 1.f)));
  m_up = normalize(getParam<float3>("up", make_float3(0.f, 1.f, 0.f)));
}

ccl::Transform Camera::getMatrix() const
{
  ccl::Transform retval;
  const auto s = ccl::normalize(ccl::cross(m_dir, m_up));
  const auto u = ccl::cross(s, m_dir);
  retval.x[0] = s.x;
  retval.x[1] = u.x;
  retval.x[2] = m_dir.x;
  retval.y[0] = s.y;
  retval.y[1] = u.y;
  retval.y[2] = m_dir.y;
  retval.z[0] = s.z;
  retval.z[1] = u.z;
  retval.z[2] = m_dir.z;
  retval.x[3] = m_pos.x;
  retval.y[3] = m_pos.y;
  retval.z[3] = m_pos.z;
  return retval;
}

// Perspective definitions ////////////////////////////////////////////////////

Perspective::Perspective(CyclesGlobalState *s) : Camera(s)
{
}

void Perspective::commit()
{
  Camera::commit();
  m_fovy = getParam<float>("fovy", radians(60.f));
  m_aspect = getParam<float>("aspect", 1.f);
}

}  // namespace cycles
