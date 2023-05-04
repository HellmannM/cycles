// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Geometry.h"
// cycles
#include "scene/shader.h"
#include "scene/shader_graph.h"
#include "scene/shader_nodes.h"

namespace cycles {

struct Material : public Object {
  Material(CyclesGlobalState *s);
  virtual ~Material() override;

  static Material *createInstance(std::string_view type, CyclesGlobalState *state);

  ccl::Shader *cyclesShader();

 protected:
  ccl::Shader m_shader;
};

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_SPECIALIZATION(cycles::Material *, ANARI_MATERIAL);
