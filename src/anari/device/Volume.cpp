// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Volume.h"
// std
#include <numeric>
// cycles
#include "scene/volume.h"
#include "graph/node_xml.h"
#include "util/foreach.h"
#include "util/path.h"

namespace cycles {

Volume::Volume(CyclesGlobalState *s) : Object(ANARI_VOLUME, s) {}

Volume::~Volume() = default;

Volume *Volume::createInstance(std::string_view subtype, CyclesGlobalState *s)
{
  if (subtype == "transferFunction1D")
    return new TransferFunction1D(s);
  else
    return (Volume *)new UnknownObject(ANARI_VOLUME, subtype, s);
}

//void Volume::markCommitted()
//{
//  deviceState()->markSceneChanged();
//  Object::markCommitted();
//}

// Subtypes ///////////////////////////////////////////////////////////////////

TransferFunction1D::TransferFunction1D(CyclesGlobalState* s)
    : Volume(s)
{
    auto& state = *deviceState();
    state.objectCounts.materials++;
    state.scene->shaders.push_back(&m_shader);

    m_graph = new ccl::ShaderGraph();

    const char* c_volume_path = getenv("CYCLES_GRAPH_VOLUME");
    xml_read_shader(c_volume_path, m_graph);

    foreach(ccl::ShaderNode *node, m_graph->nodes) {
        if (node->type->name == "map_range") {
            m_mapRangeNode = (ccl::MapRangeNode*)node;
        }

        if (node->type->name == "rgb_ramp") {
            m_rgbRampNode = (ccl::RGBRampNode*)node;
        }

        if (node->type->name == "math") {
            m_mathNode = (ccl::MathNode*)node;
        }
    }

    m_graph->simplified = true; // TODO

#if 0

    m_attributeNode = m_graph->create_node<ccl::AttributeNode>();
    m_attributeNode->set_attribute(ustring("density"));
    m_graph->add(m_attributeNode);

    m_mapRangeNode = m_graph->create_node<ccl::MapRangeNode>();
    m_mapRangeNode->set_clamp(true);
    m_graph->add(m_mapRangeNode);

    m_rgbRampNode = m_graph->create_node<ccl::RGBRampNode>();
    m_graph->add(m_rgbRampNode);

    m_mathNode = m_graph->create_node<ccl::MathNode>();
    m_mathNode->set_math_type(ccl::NODE_MATH_MULTIPLY);
    m_graph->add(m_mathNode);

    m_emissionNode = m_graph->create_node<ccl::EmissionNode>();
    m_graph->add(m_emissionNode);
    m_absorptionVolumeNode = m_graph->create_node<ccl::AbsorptionVolumeNode>();
    m_absorptionVolumeNode->set_color(ccl::make_float3(0.3, 0.3, 0.3));
    m_graph->add(m_absorptionVolumeNode);

    m_addShaderNode = m_graph->create_node<ccl::AddClosureNode>();
    m_graph->add(m_addShaderNode);

    m_graph->connect(m_attributeNode->output("Fac"), m_mapRangeNode->input("Value"));
    m_graph->connect(m_mapRangeNode->output("Result"), m_rgbRampNode->input("Fac"));
    m_graph->connect(m_rgbRampNode->output("Color"), m_emissionNode->input("Color"));
    m_graph->connect(m_rgbRampNode->output("Alpha"), m_mathNode->input("Value1"));
    m_graph->connect(m_mathNode->output("Value"), m_emissionNode->input("Strength"));
    m_graph->connect(m_rgbRampNode->output("Alpha"), m_absorptionVolumeNode->input("Density"));
    m_graph->connect(m_emissionNode->output("Emission"), m_addShaderNode->input("Closure1"));
    m_graph->connect(m_absorptionVolumeNode->output("Volume"), m_addShaderNode->input("Closure2"));
    m_graph->connect(m_addShaderNode->output("Closure"), m_graph->output()->input("Volume"));
#endif

    m_shader.set_graph(m_graph);
    m_shader.tag_update(state.scene);
    
}

TransferFunction1D::~TransferFunction1D()
{
    auto& state = *deviceState();
    state.objectCounts.materials--;
    state.scene->shaders.erase(
        std::find(state.scene->shaders.begin(), state.scene->shaders.end(), cyclesShader()));

    if (m_graph != nullptr)
        delete m_graph;

    //if (m_graph_final != nullptr)
    //    delete m_graph_final;
}

ccl::Shader* TransferFunction1D::cyclesShader()
{
    return &m_shader;
}

bool TransferFunction1D::isValid() const
{
  return m_field && m_field->isValid() && m_colorData && m_opacityData;
}

void TransferFunction1D::commit()
{
  Volume::commit();

  m_field = getParamObject<SpatialField>("value");
  if (!m_field) {
    reportMessage(ANARI_SEVERITY_WARNING,
        "no spatial field provided to transferFunction1D volume");
    return;
  }

  m_bounds = m_field->bounds();

  m_valueRange = getParam<helium::box1>("valueRange", helium::box1{0.f, 1.f});

  m_colorData = getParamObject<helium::Array1D>("color");
  m_opacityData = getParamObject<helium::Array1D>("opacity");
  m_densityScale = getParam<float>("unitDistance", 1.f);

  if (!m_colorData) {
    reportMessage(ANARI_SEVERITY_WARNING,
        "no color data provided to transferFunction1D volume");
    return;
  }

  if (!m_opacityData) {
    reportMessage(ANARI_SEVERITY_WARNING,
        "no opacity data provided to transfer function");
    return;
  }

  if (m_mapRangeNode != nullptr) {
      m_mapRangeNode->set_from_min(m_valueRange.lower);
      m_mapRangeNode->set_from_max(m_valueRange.upper);
  }

  if (m_mathNode != nullptr) {
      m_mathNode->set_value2(m_densityScale);
  }

  //m_colorData, m_opacityData
  auto* colorData = m_colorData->beginAs<anari_vec::float3>();
  auto* opacityData = m_opacityData->beginAs<float>();

  if (m_rgbRampNode != nullptr) {
      m_rgbRampNode->get_ramp().resize(m_colorData->size());
      m_rgbRampNode->get_ramp_alpha().resize(m_opacityData->size());

      for (size_t i = 0; i < m_colorData->size(); ++i) {
          m_rgbRampNode->get_ramp()[i] = (ccl::make_float3(colorData[i][0], colorData[i][1], colorData[i][2]));
      }

      for (size_t i = 0; i < m_opacityData->size(); ++i) {
          m_rgbRampNode->get_ramp_alpha()[i] = opacityData[i];
      }
  }

  m_shader.tag_update(deviceState()->scene);

#if 0

  // extract combined RGB+A map from color and opacity arrays (whose
  // sizes are allowed to differ..)
  auto *colorData = m_colorData->beginAs<anari_vec::float3>();
  auto *opacityData = m_opacityData->beginAs<float>();

  size_t tfSize = std::max(m_colorData->size(), m_opacityData->size());

  m_rgbaMap.resize(tfSize);
  for (size_t i = 0; i < tfSize; ++i) {
    float colorPos = tfSize > 1
        ? (float(i) / (tfSize - 1)) * (m_colorData->size() - 1)
        : 0.f;
    float colorFrac = colorPos - floorf(colorPos);

    anari_vec::float3 color0 = colorData[int(floorf(colorPos))];
    anari_vec::float3 color1 = colorData[int(ceilf(colorPos))];
    anari_vec::float3 color = anari_vec::float3(anari_vec::lerp(color0.x, color1.x, colorFrac),
        anari_vec::lerp(color0.y, color1.y, colorFrac),
        anari_vec::lerp(color0.z, color1.z, colorFrac));

    float alphaPos = tfSize > 1
        ? (float(i) / (tfSize - 1)) * (m_opacityData->size() - 1)
        : 0.f;
    float alphaFrac = alphaPos - floorf(alphaPos);

    float alpha0 = opacityData[int(floorf(alphaPos))];
    float alpha1 = opacityData[int(ceilf(alphaPos))];
    float alpha = anari_vec::lerp(alpha0, alpha1, alphaFrac);

    m_rgbaMap[i] = anari_vec::float4(color.x, color.y, color.z, alpha);
  }
  
  BNModel model = trackedModel();
  if (!model) return;
  int slot = trackedSlot();
  
  BNVolume vol = getBarneyVolume(model,slot);
  bnVolumeSetXF(vol,
                (float2 &)m_valueRange,
                (const float4 *)m_rgbaMap.data(),
                m_rgbaMap.size(),
                m_densityScale);
  bnCommit(vol);
#endif
}

ccl::Geometry* TransferFunction1D::makeCyclesGeometry()
{
    //auto* volume = new ccl::Volume();
    //return volume;

    auto* g = m_field->makeCyclesGeometry();
    ccl::array<ccl::Node*> used_shaders;
    used_shaders.push_back_slow(cyclesShader());
    g->set_used_shaders(used_shaders);
    return g;
}
  
//BNVolume TransferFunction1D::createBarneyVolume(BNModel model, int slot) 
//{
//  BNVolume vol =
//    bnVolumeCreate(model, slot, m_field->getBarneyScalarField(model, slot));
//  // bnVolumeSetXF(vol,
//  //               (float2 &)m_valueRange,
//  //               (const float4 *)m_rgbaMap.data(),
//  //               m_rgbaMap.size(),
//  //               m_densityScale);
//  return vol;
//}

box3 TransferFunction1D::bounds() const
{
  return m_bounds;
}

// void TransferFunction1D::cleanup() {}

} // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Volume *);
