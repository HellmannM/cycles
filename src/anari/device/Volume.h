// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

//#include "Array.h"
#include "Object.h"
#include "SpatialField.h"

namespace cycles {

	struct Volume : public Object
	{
		Volume(CyclesGlobalState* s);
		~Volume() override;

		static Volume* createInstance(std::string_view subtype, CyclesGlobalState* s);

		//void markCommitted() override;

		virtual ccl::Geometry* makeCyclesGeometry() = 0;
		//virtual BNVolume createBarneyVolume(BNModel model, int slot) = 0;
		//BNVolume getBarneyVolume(BNModel model, int slot)
		//{
		//  if (!isValid())
		//    return {};
		//  if (!isModelTracked(model, slot)) {
		//    cleanup();
		//    trackModel(model, slot);
		//  }
		//  if (!m_bnVolume) 
		//    m_bnVolume = createBarneyVolume(model,slot);
		//  return m_bnVolume;
		//}
		//  
		//void cleanup()
		//{
		//  if (m_bnVolume) {
		//    bnRelease(m_bnVolume);
		//    m_bnVolume = nullptr;
		//  }
		//}


		virtual box3 bounds() const = 0;

		//BNVolume m_bnVolume = 0;
	};

	// Subtypes ///////////////////////////////////////////////////////////////////

	struct TransferFunction1D : public Volume
	{
		TransferFunction1D(CyclesGlobalState* s);
		virtual ~TransferFunction1D() override;

		void commit() override;
		bool isValid() const override;

		ccl::Geometry* makeCyclesGeometry() override;
		//BNVolume createBarneyVolume(BNModel model, int slot) override;

		box3 bounds() const override;

	private:
		helium::IntrusivePtr<SpatialField> m_field;

		box3 m_bounds;

		helium::box1 m_valueRange{ 0.f, 1.f };
		float m_densityScale{ 1.f };

		helium::IntrusivePtr<Array1D> m_colorData;
		helium::IntrusivePtr<Array1D> m_opacityData;

		std::vector<anari_vec::float4> m_rgbaMap;

		ccl::Shader m_shader;
		ccl::ShaderGraph* m_graph{ nullptr };

		// Nodes
		//ccl::AttributeNode* m_attributeNode{ nullptr };
		ccl::MapRangeNode* m_mapRangeNode{ nullptr };
		ccl::RGBRampNode* m_rgbRampNode{ nullptr };
		ccl::MathNode* m_mathNode{ nullptr };
		////ccl::PrincipledVolumeNode* m_volumeNode{ nullptr };
		//ccl::EmissionNode* m_emissionNode{ nullptr };
		//ccl::AbsorptionVolumeNode* m_absorptionVolumeNode{ nullptr };
		//ccl::AddClosureNode* m_addShaderNode{ nullptr };

		ccl::Shader* cyclesShader();

	};

} // namespace cycles

CYCLES_ANARI_TYPEFOR_SPECIALIZATION(cycles::Volume*, ANARI_VOLUME);
