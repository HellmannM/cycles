// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Surface.h"
#include "array/ObjectArray.h"

namespace cycles {

struct World : public Object {
  World(CyclesGlobalState *s);
  ~World() override;

  void commit() override;

  void setWorldObjectsCurrent();

 private:
  void cleanup();

  helium::IntrusivePtr<ObjectArray> m_zeroSurfaceData;
};

}  // namespace cycles
