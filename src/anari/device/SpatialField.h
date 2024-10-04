// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

// anari
//#include "helium/array/Array1D.h"
//#include "helium/array/Array3D.h"
//#include "helium/array/ObjectArray.h"
#include "Object.h"
#include "array/Array1D.h"
#include "array/Array3D.h"
#include "array/ObjectArray.h"

#include "Material.h"
// ours
//#include "Object.h"
#include "scene/geometry.h"

namespace cycles {

struct SpatialField : public Object
{
  SpatialField(CyclesGlobalState *s);
  ~SpatialField() override;

  static SpatialField *createInstance(
      std::string_view subtype, CyclesGlobalState *s);

  //void markCommitted() override;

  virtual ccl::Geometry* makeCyclesGeometry() = 0;

  //virtual BNScalarField createBarneyScalarField(BNModel model, int slot) const = 0;

  //void cleanup()
  //{
  //  if (m_bnField) {
  //    bnRelease(m_bnField);
  //    m_bnField = nullptr;
  //  }
  //}
  //
  //BNScalarField getBarneyScalarField(BNModel model, int slot)
  //{
  //  if (!isValid())
  //    return {};
  //  if (!isModelTracked(model, slot)) {
  //    cleanup();
  //    trackModel(model, slot);
  //  }
  //  if (!m_bnField) 
  //    m_bnField = createBarneyScalarField(model,slot);
  //  return m_bnField;
  //}
  
  virtual box3 bounds() const = 0;

  //BNScalarField m_bnField = 0;
};

// Subtypes ///////////////////////////////////////////////////////////////////
//
//struct UnstructuredField : public SpatialField
//{
//  UnstructuredField(CyclesGlobalState *s);
//  void commit() override;
//
//  ccl::Geometry* makeCyclesGeometry() override;
//  //BNScalarField createBarneyScalarField(BNModel model, int slot) const;
//
//  box3 bounds() const override;
//
// private:
//  struct Parameters
//  {
//    helium::IntrusivePtr<Array1D> vertexPosition;
//    helium::IntrusivePtr<Array1D> vertexData;
//    helium::IntrusivePtr<Array1D> index;
//    helium::IntrusivePtr<Array1D> cellIndex;
//    // "stitcher" extensions
//    // helium::IntrusivePtr<ObjectArray> gridData;
//    // helium::IntrusivePtr<Array1D> gridDomains;
//  } m_params;
//
//  std::vector<anari_vec::float4> m_vertices;
//  std::vector<int>          m_indices;
//  std::vector<int>          m_elementOffsets;
//  // std::vector<float> m_generatedVertices;
//  // std::vector<int> m_generatedTets;
//  // std::vector<int> m_generatedPyrs;
//  // std::vector<int> m_generatedWedges;
//  // std::vector<int> m_generatedHexes;
//  // // for stitcher
//  // std::vector<int> m_generatedGridOffsets;
//  // std::vector<int> m_generatedGridDims;
//  // std::vector<float> m_generatedGridDomains;
//  // std::vector<float> m_generatedGridScalars;
//
//  box3 m_bounds;
//};

//struct BlockStructuredField : public SpatialField
//{
//  BlockStructuredField(CyclesGlobalState *s);
//  void commit() override;
//
//  ccl::Geometry* makeCyclesGeometry() override;
//  //BNScalarField createBarneyScalarField(BNModel model, int slot) const;
//
//  box3 bounds() const override;
//
//  struct Parameters
//  {
//    helium::IntrusivePtr<Array1D> cellWidth;
//    helium::IntrusivePtr<Array1D> blockBounds;
//    helium::IntrusivePtr<Array1D> blockLevel;
//    helium::IntrusivePtr<ObjectArray> blockData;
//  } m_params;
//
//  std::vector<int> m_generatedBlockBounds;
//  std::vector<int> m_generatedBlockLevels;
//  std::vector<int> m_generatedBlockOffsets;
//  std::vector<float> m_generatedBlockScalars;
//
//  box3 m_bounds;
//};

struct StructuredRegularField : public SpatialField
{
  StructuredRegularField(CyclesGlobalState *s);
  void commit() override;

  ccl::Geometry* makeCyclesGeometry() override;
  //BNScalarField createBarneyScalarField(BNModel model, int slot) const;

  box3 bounds() const override;
  bool isValid() const override;

  anari_vec::uint3 m_dims{0u};
  anari_vec::float3 m_origin;
  anari_vec::float3 m_spacing;
  anari_vec::float3 m_coordUpperBound;

  std::vector<float> m_generatedCellWidths;
  std::vector<int> m_generatedBlockBounds;
  std::vector<int> m_generatedBlockLevels;
  std::vector<int> m_generatedBlockOffsets;
  std::vector<float> m_generatedBlockScalars;

  helium::IntrusivePtr<Array3D> m_data;
};

struct NanoVDBField : public SpatialField
{
  NanoVDBField(CyclesGlobalState *s);
  void commit() override;

  ccl::Geometry* makeCyclesGeometry() override;
  //BNScalarField createBarneyScalarField(BNModel model, int slot) const;

  box3 bounds() const override;
  bool isValid() const override;

  anari_vec::ivec3 m_dims{0};
  anari_vec::float3 m_origin;
  anari_vec::float3 m_spacing;
  uint64_t m_nanovdbSize;
  anari_vec::float3 m_coordUpperBound;

  // std::vector<float> m_generatedCellWidths;
  // std::vector<int> m_generatedBlockBounds;
  // std::vector<int> m_generatedBlockLevels;
  // std::vector<int> m_generatedBlockOffsets;
  // std::vector<float> m_generatedBlockScalars;

  helium::IntrusivePtr<Array1D> m_data;

private:
	struct QuadData {
		int v0, v1, v2, v3;

		float3 normal;
	};

	enum {
		QUAD_X_MIN = 0,
		QUAD_X_MAX = 1,
		QUAD_Y_MIN = 2,
		QUAD_Y_MAX = 3,
		QUAD_Z_MIN = 4,
		QUAD_Z_MAX = 5,
	};

	const int quads_indices[6][4] = {
		/* QUAD_X_MIN */
		{4, 0, 3, 7},
		/* QUAD_X_MAX */
		{1, 5, 6, 2},
		/* QUAD_Y_MIN */
		{4, 5, 1, 0},
		/* QUAD_Y_MAX */
		{3, 2, 6, 7},
		/* QUAD_Z_MIN */
		{0, 1, 2, 3},
		/* QUAD_Z_MAX */
		{5, 4, 7, 6},
	};

	const float3 quads_normals[6] = {
		/* QUAD_X_MIN */
		make_float3(-1.0f, 0.0f, 0.0f),
		/* QUAD_X_MAX */
		make_float3(1.0f, 0.0f, 0.0f),
		/* QUAD_Y_MIN */
		make_float3(0.0f, -1.0f, 0.0f),
		/* QUAD_Y_MAX */
		make_float3(0.0f, 1.0f, 0.0f),
		/* QUAD_Z_MIN */
		make_float3(0.0f, 0.0f, -1.0f),
		/* QUAD_Z_MAX */
		make_float3(0.0f, 0.0f, 1.0f),
	};

	int add_vertex(int3 v,
		vector<int3>& vertices,
		int3 res,
		unordered_map<size_t, int>& used_verts);

	void create_quad(int3 corners[8],
		vector<int3>& vertices,
		vector<QuadData>& quads,
		int3 res,
		unordered_map<size_t, int>& used_verts,
		int face_index);
};

} // namespace barney_device

CYCLES_ANARI_TYPEFOR_SPECIALIZATION(
    cycles::SpatialField *, ANARI_SPATIAL_FIELD);
