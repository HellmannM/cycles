// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Camera.h"
// cycles
#include "scene/camera.h"

namespace cycles {

// Subtype declarations ///////////////////////////////////////////////////////

struct Perspective : public Camera {
  Perspective(CyclesGlobalState *s);

  void commit() override;
  void setCameraCurrent(int width, int height) override;

 private:
  float m_fovy{radians(60.f)};
  float m_aspect{1.f};
};

struct Orthographic : public Camera {
  Orthographic(CyclesGlobalState *s);

  void commit() override;
  void setCameraCurrent(int width, int height) override;

 private:
  float m_height{1.f};
  float m_aspect{1.f};
};

// Camera definitions /////////////////////////////////////////////////////////

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
  else if (type == "orthographic")
    return new Orthographic(s);
  else
    return (Camera *)new UnknownObject(ANARI_CAMERA, s);
}

void Camera::commit()
{
  m_pos = getParam<float3>("position", make_float3(0.f));
  m_dir = normalize(getParam<float3>("direction", make_float3(0.f, 0.f, 1.f)));
  m_up = normalize(getParam<float3>("up", make_float3(0.f, 1.f, 0.f)));
}

void Camera::setCameraCurrent(int width, int height)
{
  auto &state = *deviceState();
  state.scene->camera->set_matrix(getMatrix());
  state.scene->camera->set_full_width(width);
  state.scene->camera->set_full_height(height);
  state.scene->camera->need_flags_update = true;
  state.scene->camera->need_device_update = true;
}

ccl::Transform Camera::getMatrix() const
{
  ccl::Transform retval;
  const auto s = ccl::normalize(ccl::cross(m_dir, m_up));
  const auto u = ccl::normalize(ccl::cross(s, m_dir));
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

void Perspective::setCameraCurrent(int width, int height)
{
  Camera::setCameraCurrent(width, height);
  auto &state = *deviceState();
  state.scene->camera->viewplane.left = -m_aspect;
  state.scene->camera->viewplane.right = m_aspect;
  state.scene->camera->viewplane.bottom = -1.0f;
  state.scene->camera->viewplane.top = 1.0f;
  state.scene->camera->set_fov(m_fovy);
  state.scene->camera->set_camera_type(ccl::CameraType::CAMERA_PERSPECTIVE);
}

// Orthographic definitions ///////////////////////////////////////////////////

Orthographic::Orthographic(CyclesGlobalState *s) : Camera(s)
{
}

void Orthographic::commit()
{
  Camera::commit();
  m_height = getParam<float>("height", 1.f);
  m_aspect = getParam<float>("aspect", 1.f);
}

void Orthographic::setCameraCurrent(int width, int height)
{
  Camera::setCameraCurrent(width, height);
  auto &state = *deviceState();
  state.scene->camera->set_camera_type(ccl::CameraType::CAMERA_ORTHOGRAPHIC);
  auto scale = m_height / 2.f;
  state.scene->camera->viewplane.left = -m_aspect * scale;
  state.scene->camera->viewplane.right = m_aspect * scale;
  state.scene->camera->viewplane.bottom = -scale;
  state.scene->camera->viewplane.top = scale;
}

}  // namespace cycles
