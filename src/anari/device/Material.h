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
#if 0
  void connectAttributes(ccl::ShaderNode *bsdf,
                         const std::string &mode,
                         const char *input,
                         const float3 &v,
                         bool singleComponent = true);
#endif

  ccl::Shader m_shader;
  ccl::ShaderGraph *m_graph{nullptr};

#if 0
  struct Nodes {
    ccl::AttributeNode *attrC{nullptr};
    ccl::AttributeNode *attr0{nullptr};
    ccl::AttributeNode *attr1{nullptr};
    ccl::AttributeNode *attr2{nullptr};
    ccl::AttributeNode *attr3{nullptr};
    ccl::SeparateColorNode *attrC_sc{nullptr};
    ccl::SeparateColorNode *attr0_sc{nullptr};
    ccl::SeparateColorNode *attr1_sc{nullptr};
    ccl::SeparateColorNode *attr2_sc{nullptr};
    ccl::SeparateColorNode *attr3_sc{nullptr};
  } m_attributeNodes;
#endif
};

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_SPECIALIZATION(cycles::Material *, ANARI_MATERIAL);
