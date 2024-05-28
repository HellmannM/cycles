// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Material.h"

namespace cycles {

// MatteMaterial definitions //////////////////////////////////////////////////

struct MatteMaterial : public Material {
  MatteMaterial(CyclesGlobalState *s);
  ~MatteMaterial() override = default;
  void commit() override;

 private:
  ccl::DiffuseBsdfNode *m_bsdf{nullptr};
};

MatteMaterial::MatteMaterial(CyclesGlobalState *s) : Material(s)
{
  auto *bsdf = new ccl::DiffuseBsdfNode();
  bsdf->set_owner(m_graph);
  m_graph->add(bsdf);

  m_graph->connect(bsdf->output("BSDF"), m_graph->output()->input("Surface"));

  m_bsdf = bsdf;
}

void MatteMaterial::commit()
{
  auto colorMode = getParamString("color", "");
  auto color = getParam<float3>("color", make_float3(1.f, 1.f, 1.f));

  connectAttributes(m_bsdf, colorMode, "Color", color, false);

  m_shader.tag_update(deviceState()->scene);
}

// PhysicallyBasedMaterial ////////////////////////////////////////////////////

struct PhysicallyBasedMaterial : public Material {
  PhysicallyBasedMaterial(CyclesGlobalState *s);
  ~PhysicallyBasedMaterial() override = default;
  void commit() override;

 private:
  ccl::PrincipledBsdfNode *m_bsdf{nullptr};
};

PhysicallyBasedMaterial::PhysicallyBasedMaterial(CyclesGlobalState *s) : Material(s)
{
  auto *bsdf = new ccl::PrincipledBsdfNode();
  bsdf->set_owner(m_graph);
  m_graph->add(bsdf);

  m_graph->connect(bsdf->output("BSDF"), m_graph->output()->input("Surface"));

  m_bsdf = bsdf;
}

void PhysicallyBasedMaterial::commit()
{
  auto colorMode = getParamString("baseColor", "");
  auto color = getParam<float3>("baseColor", make_float3(1.f, 1.f, 1.f));
  connectAttributes(m_bsdf, colorMode, "Base Color", color, false);

  auto opacityMode = getParamString("opacity", "");
  auto opacity = getParam<float>("opacity", 1.f);
  connectAttributes(m_bsdf, opacityMode, "Alpha", make_float3(opacity));

#if 0
  auto specularMode = getParamString("specular", "");
  auto specular = getParam<float>("specular", 0.f);
  connectAttributes(m_bsdf, specularMode, "Specular", make_float3(specular));
#endif

  auto roughnessMode = getParamString("roughness", "");
  auto roughness = getParam<float>("roughness", 1.f);
  connectAttributes(m_bsdf, roughnessMode, "Roughness", make_float3(roughness));

  auto metallicMode = getParamString("metallic", "");
  auto metallic = getParam<float>("metallic", 1.f);
  connectAttributes(m_bsdf, metallicMode, "Metallic", make_float3(metallic));

#if 0
  auto transmissionMode = getParamString("transmission", "");
  auto transmission = getParam<float>("transmission", 0.f);
  connectAttributes(m_bsdf, transmissionMode, "Transmission", make_float3(transmission));

  auto clearcoatMode = getParamString("clearcoat", "");
  auto clearcoat = getParam<float>("clearcoat", 0.f);
  connectAttributes(m_bsdf, clearcoatMode, "Clearcoat", make_float3(clearcoat));

  auto clearcoatRoughnessMode = getParamString("clearcoatRoughness", "");
  auto clearcoatRoughness = getParam<float>("clearcoatRoughness", 0.f);
  connectAttributes(
      m_bsdf, clearcoatRoughnessMode, "Clearcoat Roughness", make_float3(clearcoatRoughness));
#endif

  auto ior = getParam<float>("ior", 1.5f);
  m_bsdf->input("IOR")->set(ior);

  m_shader.tag_update(deviceState()->scene);
}

// Material definitions ///////////////////////////////////////////////////////

Material::Material(CyclesGlobalState *s) : Object(ANARI_SURFACE, s)
{
  auto &state = *deviceState();
  state.objectCounts.materials++;
  state.scene->shaders.push_back(&m_shader);

  m_graph = new ccl::ShaderGraph();

  auto *vertexColor = m_graph->create_node<ccl::AttributeNode>();
  vertexColor->set_attribute(ccl::ustring("vertex.color"));
  m_graph->add(vertexColor);

  auto *attr0 = m_graph->create_node<ccl::AttributeNode>();
  attr0->set_attribute(ccl::ustring("vertex.attribute0"));
  m_graph->add(attr0);

  auto *attr1 = m_graph->create_node<ccl::AttributeNode>();
  attr1->set_attribute(ccl::ustring("vertex.attribute1"));
  m_graph->add(attr1);

  auto *attr2 = m_graph->create_node<ccl::AttributeNode>();
  attr2->set_attribute(ccl::ustring("vertex.attribute2"));
  m_graph->add(attr2);

  auto *attr3 = m_graph->create_node<ccl::AttributeNode>();
  attr3->set_attribute(ccl::ustring("vertex.attribute3"));
  m_graph->add(attr3);

  auto *vertexColor_sc = m_graph->create_node<ccl::SeparateColorNode>();
  m_graph->add(vertexColor_sc);
  m_graph->connect(vertexColor->output("Color"), vertexColor_sc->input("Color"));

  auto *attr0_sc = m_graph->create_node<ccl::SeparateColorNode>();
  m_graph->add(attr0_sc);
  m_graph->connect(attr0->output("Color"), attr0_sc->input("Color"));

  auto *attr1_sc = m_graph->create_node<ccl::SeparateColorNode>();
  m_graph->add(attr1_sc);
  m_graph->connect(attr1->output("Color"), attr1_sc->input("Color"));

  auto *attr2_sc = m_graph->create_node<ccl::SeparateColorNode>();
  m_graph->add(attr2_sc);
  m_graph->connect(attr2->output("Color"), attr2_sc->input("Color"));

  auto *attr3_sc = m_graph->create_node<ccl::SeparateColorNode>();
  m_graph->add(attr3_sc);
  m_graph->connect(attr3->output("Color"), attr3_sc->input("Color"));

  m_shader.set_graph(m_graph);

  m_attributeNodes.attrC = vertexColor;
  m_attributeNodes.attr0 = attr0;
  m_attributeNodes.attr1 = attr1;
  m_attributeNodes.attr2 = attr2;
  m_attributeNodes.attr3 = attr3;
  m_attributeNodes.attrC_sc = vertexColor_sc;
  m_attributeNodes.attr0_sc = attr0_sc;
  m_attributeNodes.attr1_sc = attr1_sc;
  m_attributeNodes.attr2_sc = attr2_sc;
  m_attributeNodes.attr3_sc = attr3_sc;
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

void Material::connectAttributes(ccl::ShaderNode *bsdf,
                                 const std::string &mode,
                                 const char *input,
                                 const float3 &v,
                                 bool singleComponent)
{
  if (mode == "color") {
    m_graph->connect(singleComponent ? m_attributeNodes.attrC_sc->output("Red") :
                                       m_attributeNodes.attrC->output("Color"),
                     bsdf->input(input));
  }
  else if (mode == "attribute0") {
    m_graph->connect(singleComponent ? m_attributeNodes.attr0_sc->output("Red") :
                                       m_attributeNodes.attr0->output("Color"),
                     bsdf->input(input));
  }
  else if (mode == "attribute1") {
    m_graph->connect(singleComponent ? m_attributeNodes.attr1_sc->output("Red") :
                                       m_attributeNodes.attr1->output("Color"),
                     bsdf->input(input));
  }
  else if (mode == "attribute2") {
    m_graph->connect(singleComponent ? m_attributeNodes.attr2_sc->output("Red") :
                                       m_attributeNodes.attr2->output("Color"),
                     bsdf->input(input));
  }
  else if (mode == "attribute3") {
    m_graph->connect(singleComponent ? m_attributeNodes.attr3_sc->output("Red") :
                                       m_attributeNodes.attr3->output("Color"),
                     bsdf->input(input));
  }
  else {
    m_graph->disconnect(bsdf->input(input));
    if (singleComponent)
      bsdf->input(input)->set(v.x);
    else
      bsdf->input(input)->set(v);
  }
}

ccl::Shader *Material::cyclesShader()
{
  return &m_shader;
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Material *);
