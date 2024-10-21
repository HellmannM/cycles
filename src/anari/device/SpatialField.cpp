// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

// std
#include <cfloat>
// ours
//#include "Array.h"
#include "SpatialField.h"
// cycles
#include "scene/volume.h"
#include "util/hash.h"

namespace cycles {

	SpatialField::SpatialField(CyclesGlobalState* s)
		: Object(ANARI_SPATIAL_FIELD, s)
	{}

	SpatialField::~SpatialField() = default;

	SpatialField* SpatialField::createInstance(
		std::string_view subtype, CyclesGlobalState* s)
	{
		//if (subtype == "unstructured")
		//  return new UnstructuredField(s);
		//if (subtype == "amr")
		//  return new BlockStructuredField(s);
		if (subtype == "structuredRegular")
			return new StructuredRegularField(s);
		else
			return (SpatialField*)new UnknownObject(ANARI_SPATIAL_FIELD, subtype, s);
	}

	//void SpatialField::markCommitted()
	//{
	//  deviceState()->markSceneChanged();
	//  Object::markCommitted();
	//}

	// Subtypes ///////////////////////////////////////////////////////////////////

	// StructuredRegularField //

	StructuredRegularField::StructuredRegularField(CyclesGlobalState* s)
		: SpatialField(s)
	{}

	void StructuredRegularField::commit()
	{
		Object::commit();
		m_data = getParamObject<helium::Array3D>("data");

		if (!m_data) {
			reportMessage(ANARI_SEVERITY_WARNING,
				"missing required parameter 'data' on 'structuredRegular' field");
			return;
		}

		m_origin = getParam<helium::float3>("origin", helium::float3(0.f));
		m_spacing = getParam<helium::float3>("spacing", helium::float3(1.f));
		m_dims = m_data->size();

		const auto dims = m_data->size();
		m_coordUpperBound = helium::float3(std::nextafter(dims.x - 1, 0),
			std::nextafter(dims.y - 1, 0),
			std::nextafter(dims.z - 1, 0));
	}

	bool StructuredRegularField::isValid() const
	{
		return m_data;
	}

	ccl::Geometry* StructuredRegularField::makeCyclesGeometry()
	{
		auto* volume = new ccl::Volume();
		return volume;
	}

	box3 StructuredRegularField::bounds() const
	{
		if (!isValid())
			return empty_box3();

		box3 b;
		b.lower[0] = m_origin[0];
		b.lower[1] = m_origin[1];
		b.lower[2] = m_origin[2];

		ccl::float3 upper;
		b.upper[0] = (m_dims[0] - 1.f) * m_spacing[0];
		b.upper[1] = (m_dims[1] - 1.f) * m_spacing[1];
		b.upper[2] = (m_dims[2] - 1.f) * m_spacing[2];

		return b;
	}


} // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::SpatialField*);
