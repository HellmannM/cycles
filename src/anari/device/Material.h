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
  ~Material() override;
  void commit() override;

  ccl::Shader *cyclesShader();

 private:
  ccl::Shader m_shader;
  ccl::PrincipledBsdfNode *m_bsdf{nullptr};
};

}  // namespace cycles
