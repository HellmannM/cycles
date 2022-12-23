// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"

namespace cycles {

struct Camera : public Object {

  Camera(CyclesGlobalState *s);
  ~Camera() override;

  static Camera *createInstance(std::string_view type, CyclesGlobalState *state);

  virtual void commit() override;

  ccl::Transform getMatrix() const;

 protected:
  float3 m_pos;
  float3 m_dir;
  float3 m_up;
};

struct Perspective : public Camera
{
  Perspective(CyclesGlobalState *s);

  void commit() override;

 private:
   float m_fovy{radians(60.f)};
   float m_aspect{1.f};
};

}  // namespace cycles
