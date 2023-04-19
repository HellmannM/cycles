// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Material.h"

namespace cycles {

Material::Material(CyclesGlobalState *s) : Object(ANARI_SURFACE, s)
{
  auto &state = *deviceState();

  state.objectCounts.materials++;

  auto *graph = new ccl::ShaderGraph();
  auto *bsdf = new ccl::PrincipledBsdfNode();

  bsdf->set_owner(graph);
  bsdf->input("Base Color")->set(ccl::make_float3(1, 0, 0));
  bsdf->input("Roughness")->set(1.f);
  bsdf->input("Clearcoat Roughness")->set(1.f);
  bsdf->input("Specular")->set(0.f);
  bsdf->input("Alpha")->set(1.f);
  graph->add(bsdf);

  ccl::ShaderOutput *output = bsdf->output("BSDF");
  ccl::ShaderInput *input = graph->output()->input("Surface");

  if (output && input)
    graph->connect(output, input);

  m_shader.set_graph(graph);
  m_shader.tag_update(state.scene);

  state.scene->shaders.push_back(&m_shader);

  m_bsdf = bsdf;
}

Material::~Material()
{
  auto &state = *deviceState();
  state.objectCounts.materials--;
  state.scene->shaders.erase(
      std::find(state.scene->shaders.begin(), state.scene->shaders.end(), cyclesShader()));
}

void Material::commit()
{
  auto color = getParam<float3>("color",
                                getParam<float3>("baseColor", make_float3(1.f, 1.f, 1.f)));
  m_bsdf->input("Base Color")->set(color);

  auto opacity = getParam<float>("opacity", 1.f);
  m_bsdf->input("Alpha")->set(opacity);

  m_shader.tag_update(deviceState()->scene);
}

ccl::Shader *Material::cyclesShader()
{
  return &m_shader;
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Material *);
