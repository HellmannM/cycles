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
  ccl::ShaderGraph *m_graph{nullptr};
  struct Nodes {
    ccl::PrincipledBsdfNode *bsdf{nullptr};
    ccl::AttributeNode *colorAttr{nullptr};
  } m_nodes;
};

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_SPECIALIZATION(cycles::Material *, ANARI_MATERIAL);
