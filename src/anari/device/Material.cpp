// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Material.h"

namespace cycles {

Material::Material(CyclesGlobalState *s) : Object(ANARI_SURFACE, s)
{
  auto &state = *deviceState();

  state.objectCounts.materials++;

  auto *graph = new ccl::ShaderGraph();

  auto *vertexColor = graph->create_node<ccl::AttributeNode>();
  vertexColor->set_attribute(ccl::ustring("vertex.color"));
  graph->add(vertexColor);

  auto *attr0 = graph->create_node<ccl::AttributeNode>();
  attr0->set_attribute(ccl::ustring("vertex.attribute0"));
  graph->add(attr0);

  auto *attr1 = graph->create_node<ccl::AttributeNode>();
  attr1->set_attribute(ccl::ustring("vertex.attribute1"));
  graph->add(attr1);

  auto *attr2 = graph->create_node<ccl::AttributeNode>();
  attr2->set_attribute(ccl::ustring("vertex.attribute2"));
  graph->add(attr2);

  auto *attr3 = graph->create_node<ccl::AttributeNode>();
  attr3->set_attribute(ccl::ustring("vertex.attribute3"));
  graph->add(attr3);

  auto *bsdf = new ccl::PrincipledBsdfNode();
  bsdf->set_owner(graph);
  // bsdf->input("Base Color")->set(ccl::make_float3(1, 0, 0));
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

  state.scene->shaders.push_back(&m_shader);

  m_graph = graph;
  m_nodes.bsdf = bsdf;
  m_nodes.colorAttr = vertexColor;
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
  auto colorMode = getParamString("color", getParamString("baseColor", ""));
  auto color = getParam<float3>("color",
                                getParam<float3>("baseColor", make_float3(1.f, 1.f, 1.f)));

  if (colorMode == "color")
    m_graph->connect(m_nodes.colorAttr->output("Color"), m_nodes.bsdf->input("Base Color"));
  else {
    m_graph->disconnect(m_nodes.bsdf->input("Base Color"));
    m_nodes.bsdf->input("Base Color")->set(color);
  }

  auto opacity = getParam<float>("opacity", 1.f);
  m_nodes.bsdf->input("Alpha")->set(opacity);

  m_shader.tag_update(deviceState()->scene);
}

ccl::Shader *Material::cyclesShader()
{
  return &m_shader;
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Material *);
