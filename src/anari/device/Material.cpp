// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Material.h"

namespace cycles {

struct MatteMaterial : public Material {
  MatteMaterial(CyclesGlobalState *s);
  ~MatteMaterial() override = default;
  void commit() override;

 private:
  ccl::ShaderGraph *m_graph{nullptr};
  struct Nodes {
    ccl::DiffuseBsdfNode *bsdf{nullptr};
    ccl::AttributeNode *colorAttr{nullptr};
  } m_nodes;
};

MatteMaterial::MatteMaterial(CyclesGlobalState *s) : Material(s)
{
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

  auto *bsdf = new ccl::DiffuseBsdfNode();
  bsdf->set_owner(graph);
  graph->add(bsdf);

  ccl::ShaderOutput *output = bsdf->output("BSDF");
  ccl::ShaderInput *input = graph->output()->input("Surface");

  if (output && input)
    graph->connect(output, input);

  m_shader.set_graph(graph);

  m_graph = graph;
  m_nodes.bsdf = bsdf;
  m_nodes.colorAttr = vertexColor;
}

void MatteMaterial::commit()
{
  auto colorMode = getParamString("color", "");
  auto color = getParam<float3>("color", make_float3(1.f, 1.f, 1.f));

  if (colorMode == "color")
    m_graph->connect(m_nodes.colorAttr->output("Color"), m_nodes.bsdf->input("Color"));
  else {
    m_graph->disconnect(m_nodes.bsdf->input("Color"));
    m_nodes.bsdf->input("Color")->set(color);
  }

  m_shader.tag_update(deviceState()->scene);
}

// PhysicallyBasedMaterial ////////////////////////////////////////////////////

struct PhysicallyBasedMaterial : public Material {
  PhysicallyBasedMaterial(CyclesGlobalState *s);
  ~PhysicallyBasedMaterial() override = default;
  void commit() override;

 private:
  ccl::ShaderGraph *m_graph{nullptr};
  struct Nodes {
    ccl::PrincipledBsdfNode *bsdf{nullptr};
    ccl::AttributeNode *colorAttr{nullptr};
  } m_nodes;
};

PhysicallyBasedMaterial::PhysicallyBasedMaterial(CyclesGlobalState *s) : Material(s)
{
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
  bsdf->input("Clearcoat Roughness")->set(1.f);
  graph->add(bsdf);

  ccl::ShaderOutput *output = bsdf->output("BSDF");
  ccl::ShaderInput *input = graph->output()->input("Surface");

  if (output && input)
    graph->connect(output, input);

  m_shader.set_graph(graph);

  m_graph = graph;
  m_nodes.bsdf = bsdf;
  m_nodes.colorAttr = vertexColor;
}

void PhysicallyBasedMaterial::commit()
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

  auto specular = getParam<float>("specular", 1.f);
  m_nodes.bsdf->input("Specular")->set(specular);

  auto roughness = getParam<float>("roughness", 1.f);
  m_nodes.bsdf->input("Roughness")->set(roughness);

  m_shader.tag_update(deviceState()->scene);
}

// Material definitions ///////////////////////////////////////////////////////

Material::Material(CyclesGlobalState *s) : Object(ANARI_SURFACE, s)
{
  auto &state = *deviceState();
  state.objectCounts.materials++;
  state.scene->shaders.push_back(&m_shader);
}

Material::~Material()
{
  auto &state = *deviceState();
  state.objectCounts.materials--;
  state.scene->shaders.erase(
      std::find(state.scene->shaders.begin(), state.scene->shaders.end(), cyclesShader()));
}

Material *Material::createInstance(std::string_view type, CyclesGlobalState *s)
{
  if (type == "matte")
    return new MatteMaterial(s);
  else if (type == "physicallyBased")
    return new PhysicallyBasedMaterial(s);
  else
    return (Material *)new UnknownObject(ANARI_MATERIAL, type, s);
}

ccl::Shader *Material::cyclesShader()
{
  return &m_shader;
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Material *);
