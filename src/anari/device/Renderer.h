// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"

namespace cycles {

struct Renderer : public Object {
  Renderer(CyclesGlobalState *s);
  ~Renderer() override;

  void commit() override;

  void makeRendererCurrent() const;

 private:
  anari_vec::float4 m_backgroundColor;
};

}  // namespace cycles
