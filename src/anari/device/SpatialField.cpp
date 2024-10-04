// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

// std
#include <cfloat>
// ours
//#include "Array.h"
#include "SpatialField.h"
// cycles
#include "scene/volume.h"
#include "scene/image_vdb.h"
#include "util/hash.h"

#define NANOVDB_USE_INTRINSICS
#include <nanovdb/NanoVDB.h>

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
		else if (subtype == "nanovdb")
			return new NanoVDBField(s);
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

	//BNScalarField StructuredRegularField::createBarneyScalarField(
	//    BNModel model, int slot) const
	//{
	//  if (!isValid())
	//    return {};
	//  auto ctx = deviceState()->context;
	//  BNScalarType barneyType;
	//  switch (m_data->elementType()) {
	//  case ANARI_FLOAT32:
	//    barneyType = BN_SCALAR_FLOAT;
	//    break;
	//  case ANARI_FLOAT64:
	//    barneyType = BN_SCALAR_DOUBLE;
	//    break;    
	//  case ANARI_UINT8:
	//    barneyType = BN_SCALAR_UINT8;
	//    break;
	//  // case ANARI_FLOAT64:
	//  //   return ((double *)m_data)[i];
	//  // case ANARI_UFIXED8:
	//  //   return ((uint8_t *)m_data)[i] /
	//  //   float(std::numeric_limits<uint8_t>::max());
	//  // case ANARI_UFIXED16:
	//  //   return ((uint16_t *)m_data)[i]
	//  //       / float(std::numeric_limits<uint16_t>::max());
	//  // case ANARI_FIXED16:
	//  //   return ((int16_t *)m_data)[i] /
	//  //   float(std::numeric_limits<int16_t>::max());
	//  default:
	//    throw std::runtime_error("scalar type not implemented ...");
	//  }
	//  auto dims = m_data->size();
	//  auto field = bnStructuredDataCreate(model,
	//      slot,
	//      (const int3 &)dims,
	//      barneyType,
	//      m_data->data(),
	//      (const float3 &)m_origin,
	//      (const float3 &)m_spacing);
	//  return field;
	//}

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

	// NanoVDBField //

	NanoVDBField::NanoVDBField(CyclesGlobalState* s)
		: SpatialField(s)
	{}

	void NanoVDBField::commit()
	{
		Object::commit();
		m_data = getParamObject<helium::Array1D>("data");

		if (!m_data) {
			reportMessage(ANARI_SEVERITY_WARNING,
				"missing required parameter 'data' on 'nanovdb' field");
			return;
		}

		m_origin = getParam<helium::float3>("origin", helium::float3(0.f));
		m_spacing = getParam<helium::float3>("spacing", helium::float3(1.f));
		m_dims = getParam<helium::int3>("dims", helium::int3(0));
		m_nanovdbSize = m_data->size();

		const auto dims = m_dims;
		m_coordUpperBound = helium::float3(std::nextafter(dims[0] - 1, 0),
			std::nextafter(dims[1] - 1, 0),
			std::nextafter(dims[2] - 1, 0));
	}

	bool NanoVDBField::isValid() const
	{
		return m_data;
	}

	int NanoVDBField::add_vertex(int3 v,
		vector<int3>& vertices,
		int3 res,
		unordered_map<size_t, int>& used_verts)
	{
		size_t vert_key = v.x + v.y * (res.x + 1) + v.z * (res.x + 1) * (res.y + 1);
		unordered_map<size_t, int>::iterator it = used_verts.find(vert_key);

		if (it != used_verts.end()) {
			return it->second;
		}

		int vertex_offset = vertices.size();
		used_verts[vert_key] = vertex_offset;
		vertices.push_back(v);
		return vertex_offset;
	}

	void NanoVDBField::create_quad(int3 corners[8],
		vector<int3>& vertices,
		vector<QuadData>& quads,
		int3 res,
		unordered_map<size_t, int>& used_verts,
		int face_index)
	{
		QuadData quad;
		quad.v0 = add_vertex(corners[quads_indices[face_index][0]], vertices, res, used_verts);
		quad.v1 = add_vertex(corners[quads_indices[face_index][1]], vertices, res, used_verts);
		quad.v2 = add_vertex(corners[quads_indices[face_index][2]], vertices, res, used_verts);
		quad.v3 = add_vertex(corners[quads_indices[face_index][3]], vertices, res, used_verts);
		quad.normal = quads_normals[face_index];

		quads.push_back(quad);
	}

	ccl::Geometry* NanoVDBField::makeCyclesGeometry()
	{
		auto* volume = new ccl::Volume();
		//volume->clear(true);
		//volume->set_clipping(b_render.clipping());
		volume->set_clipping(0.0f);		
		//volume->set_step_size(b_render.step_size());
		volume->set_step_size(0.0f);
		//volume->set_object_space((b_render.space() == BL::VolumeRender::space_OBJECT));
		volume->set_object_space(true);
		volume->set_is_nanovdb(true);

		Attribute* attr =
			volume->attributes.add(ccl::ATTR_STD_VOLUME_DENSITY);

		ImageLoader* loader = new ccl::NanoVDBImageLoader((nanovdb::NanoGrid<float>*)m_data->data(), m_nanovdbSize);
		ImageParams params;
		//params.frame = b_volume.grids.frame();
		auto& state = *deviceState();
		attr->data_voxel() = state.scene->image_manager->add_image(loader, params, false);

		//attr->data_voxel() = scene->image_manager->add_image(loader, params, false);

		//void GeometryManager::create_volume_mesh

		//for (Node* node : volume->get_used_shaders()) {
		//    Shader* shader = static_cast<Shader*>(node);

		//    if (!shader->has_volume) {
		//        continue;
		//    }

		//VolumeMeshBuilder builder;

		//openvdb::GridBase::ConstPtr grid;
		//if (vdb_loader) {
		//    grid = vdb_loader->get_grid();
		//builder.add_grid(grid, do_clipping, volume->get_clipping());
		const float face_overlap_avoidance = 0.1f *
			hash_uint_to_float(hash_string(volume->name.c_str()));

		/////////// Create mesh
		vector<float3> vertices;
		vector<int> indices;
		vector<float3> face_normals;
		//  builder.create_mesh(vertices, indices, face_normals, face_overlap_avoidance);

		/* We create vertices in index space (is), and only convert them to object
		* space when done. */
		vector<int3> vertices_is;
		vector<QuadData> quads;

		/* make sure we only have leaf nodes in the tree, as tiles are not handled by
		 * this algorithm */
		 //topology_grid->tree().voxelizeActiveTiles();

		 ////////////////////////////////////generate_vertices_and_quads(vertices_is, quads);

		nanovdb::NanoGrid<float>* nanogrid = (nanovdb::NanoGrid<float> *)m_data->data();
		auto bbox_index = nanogrid->indexBBox();
		auto bbox_world = nanogrid->worldBBox();

		auto vdb_spacing = nanogrid->voxelSize();

		const int3 resolution = make_int3(bbox_index.dim().x(), bbox_index.dim().y(), bbox_index.dim().z());

		unordered_map<size_t, int> used_verts;

		int3 min = make_int3(bbox_index.min().x(), bbox_index.min().y(), bbox_index.min().z());
		int3 max = make_int3(bbox_index.max().x(), bbox_index.max().y(), bbox_index.max().z());

		int3 corners[8] = {
			make_int3(min[0], min[1], min[2]),
			make_int3(max[0], min[1], min[2]),
			make_int3(max[0], max[1], min[2]),
			make_int3(min[0], max[1], min[2]),
			make_int3(min[0], min[1], max[2]),
			make_int3(max[0], min[1], max[2]),
			make_int3(max[0], max[1], max[2]),
			make_int3(min[0], max[1], max[2]),
		};

		create_quad(corners, vertices_is, quads, resolution, used_verts, QUAD_X_MIN);
		create_quad(corners, vertices_is, quads, resolution, used_verts, QUAD_X_MAX);
		create_quad(corners, vertices_is, quads, resolution, used_verts, QUAD_Y_MIN);
		create_quad(corners, vertices_is, quads, resolution, used_verts, QUAD_Y_MAX);
		create_quad(corners, vertices_is, quads, resolution, used_verts, QUAD_Z_MIN);
		create_quad(corners, vertices_is, quads, resolution, used_verts, QUAD_Z_MAX);

		///////////////////////////////convert_object_space(vertices_is, vertices, face_overlap_avoidance);
		  /* compute the offset for the face overlap avoidance */
		//bbox = topology_grid->evalActiveVoxelBoundingBox();
		//openvdb::Coord dim = bbox.dim();

		//float3 cell_size = make_float3(1.0f / bbox.dim()[0], 1.0f / bbox.dim()[1], 1.0f / bbox.dim()[2]);
		float3 cell_size = make_float3(1.0f / bbox_index.dim().x(), 1.0f / bbox_index.dim().y(), 1.0f / bbox_index.dim().z());
		float3 point_offset = cell_size * face_overlap_avoidance;

		vertices.reserve(vertices_is.size());

		for (size_t i = 0; i < vertices_is.size(); ++i) {			
			nanovdb::Vec3d p = nanogrid->indexToWorld(nanovdb::Vec3d(vertices_is[i].x, vertices_is[i].y, vertices_is[i].z));
			float3 vertex = make_float3((float)p[0], (float)p[1], (float)p[2]);
			vertices.push_back(vertex + point_offset);
		}
		///////////////////////////////convert_quads_to_tris(quads, indices, face_normals);
		int index_offset = 0;
		indices.resize(quads.size() * 6);
		face_normals.reserve(quads.size() * 2);

		for (size_t i = 0; i < quads.size(); ++i) {
			indices[index_offset++] = quads[i].v0;
			indices[index_offset++] = quads[i].v2;
			indices[index_offset++] = quads[i].v1;

			face_normals.push_back(quads[i].normal);

			indices[index_offset++] = quads[i].v0;
			indices[index_offset++] = quads[i].v3;
			indices[index_offset++] = quads[i].v2;

			face_normals.push_back(quads[i].normal);
		}

		volume->reserve_mesh(vertices.size(), indices.size() / 3);
		//volume->used_shaders.clear();
		//volume->used_shaders.push_back_slow(volume_shader);

		for (size_t i = 0; i < vertices.size(); ++i) {
			volume->add_vertex(vertices[i]);
		}

		for (size_t i = 0; i < indices.size(); i += 3) {
			volume->add_triangle(indices[i], indices[i + 1], indices[i + 2], 0, false);
		}

		Attribute* attr_fN = volume->attributes.add(ATTR_STD_FACE_NORMAL);
		float3* fN = attr_fN->data_float3();

		for (size_t i = 0; i < face_normals.size(); ++i) {
			fN[i] = face_normals[i];
		}

		return volume;
	}

	//BNScalarField NanoVDBField::createBarneyScalarField(
	//    BNModel model, int slot) const
	//{
	//  if (!isValid())
	//    return {};
	//
	//  auto dims = m_dims;
	//  auto field = bnNanoVDBDataCreate(model,
	//      slot,
	//      (const int3 &)dims,
	//      m_data->data(),
	//      m_nanovdbSize,
	//      (const float3 &)m_origin,
	//      (const float3 &)m_spacing);
	//  return field;
	//}

	box3 NanoVDBField::bounds() const
	{
		//return isValid()
		//    ? box3(m_origin, m_origin + ((helium::float3(m_dims) - 1.f) * m_spacing))
		//    : box3{};

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

	//// UnstructuredField //
	//
	//UnstructuredField::UnstructuredField(CyclesGlobalState *s) : SpatialField(s) {}
	//
	//void UnstructuredField::commit()
	//{
	//  Object::commit();
	//
	//  m_params.vertexPosition = getParamObject<helium::Array1D>("vertex.position");
	//  m_params.vertexData = getParamObject<helium::Array1D>("vertex.data");
	//  m_params.index = getParamObject<helium::Array1D>("index");
	//  m_params.cellIndex = getParamObject<helium::Array1D>("cell.index");
	//
	//  if (!m_params.vertexPosition) {
	//    reportMessage(ANARI_SEVERITY_WARNING,
	//        "missing required parameter 'vertex.position' on unstructured spatial field");
	//    return;
	//  }
	//
	//  if (!m_params.vertexData) { // currently vertex data only!
	//    reportMessage(ANARI_SEVERITY_WARNING,
	//        "missing required parameter 'vertex.data' on unstructured spatial field");
	//    return;
	//  }
	//
	//  if (!m_params.index) {
	//    reportMessage(ANARI_SEVERITY_WARNING,
	//        "missing required parameter 'index' on unstructured spatial field");
	//    return;
	//  }
	//
	//  if (!m_params.cellIndex) {
	//    reportMessage(ANARI_SEVERITY_WARNING,
	//        "missing required parameter 'cell.index' on unstructured spatial field");
	//    return;
	//  }
	//
	//  // m_params.gridData = getParamObject<helium::ObjectArray>("grid.data");
	//  // m_params.gridDomains = getParamObject<helium::Array1D>("grid.domains");
	//
	//  m_bounds.invalidate();
	//  auto *vertexPosition = m_params.vertexPosition->beginAs<math::float3>();
	//  int numVertices      = m_params.vertexPosition->endAs<math::float3>()-vertexPosition;
	//  auto *vertexData = m_params.vertexData->beginAs<float>();
	//  m_vertices.resize(numVertices);
	//  for (int i=0;i<numVertices;i++) {
	//    (math::float3&)m_vertices[i] = vertexPosition[i];
	//    m_bounds.insert(vertexPosition[i]);
	//    m_vertices[i].w              = vertexData[i];
	//  }
	//  auto *index = m_params.index->beginAs<uint64_t>();
	//
	//  // size_t numVerts = m_params.vertexPosition->size();
	//  size_t numIndices = m_params.index->endAs<uint64_t>() - index;
	//
	//  m_indices.resize(numIndices);
	//  for (int i=0;i<numIndices;i++)
	//    m_indices[i] = index[i];
	//
	//  auto *cellIndex = m_params.cellIndex->beginAs<uint64_t>();
	//  size_t numCells = m_params.cellIndex->endAs<uint64_t>() - cellIndex;
	//  m_elementOffsets.resize(numCells);
	//  for (int i=0;i<numCells;i++)
	//    m_elementOffsets[i] = cellIndex[i];
	//  // m_generatedVertices.clear();
	//  // m_generatedTets.clear();
	//  // m_generatedPyrs.clear();
	//  // m_generatedWedges.clear();
	//  // m_generatedHexes.clear();
	//
	//
	//  // for (size_t i = 0; i < numIndices; ++i) {
	//  //   m_bounds.insert(vertexPosition[index[i]]);
	//  // }
	//
	//  // for (size_t i = 0; i < numVerts; ++i) {
	//  //   math::float3 pos = vertexPosition[i];
	//  //   float value = vertexData[i];
	//  //   m_generatedVertices.push_back(pos.x);
	//  //   m_generatedVertices.push_back(pos.y);
	//  //   m_generatedVertices.push_back(pos.z);
	//  //   m_generatedVertices.push_back(value);
	//  // }
	//
	//  // enum { VTK_TETRA=10, VTK_HEXAHEDRON=12, VTK_WEDGE=13, VTK_PYRAMID=14 };
	//  // for (size_t i = 0; i < numCells; ++i) {
	//  //   uint64_t firstIndex = cellIndex[i];
	//  //   uint64_t lastIndex = (i < (numCells - 1)) ? cellIndex[i + 1] : numIndices;
	//
	//  //   m_elementOffsets.push_back(firstIndex);
	//  //     if (lastIndex - firstIndex == 4) {
	//  //     // for (uint64_t j = firstIndex; j < lastIndex; ++j) {
	//  //     //   m_generatedTets.push_back(index[j]);
	//  //     // }
	//  //     m_elementTypes.push_back(VTK_TETRA);
	//  //   } else if (lastIndex - firstIndex == 5) {
	//  //     // for (uint64_t j = firstIndex; j < lastIndex; ++j) {
	//  //     //   m_generatedPyrs.push_back(index[j]);
	//  //     // }
	//  //     m_elementTypes.push_back(VTK_PYRAMID);
	//  //   } else if (lastIndex - firstIndex == 6) {
	//  //     // for (uint64_t j = firstIndex; j < lastIndex; ++j) {
	//  //     //   m_generatedWedges.push_back(index[j]);
	//  //     // }
	//  //     m_elementTypes.push_back(VTK_WEDGE);
	//  //   } else if (lastIndex - firstIndex == 8) {
	//  //     // for (uint64_t j = firstIndex; j < lastIndex; ++j) {
	//  //     //   m_generatedHexes.push_back(index[j]);
	//  //     // }
	//  //     m_elementTypes.push_back(VTK_HEXAHEDRON);
	//  //   }
	//  // }
	//
	//  // if (m_params.gridData && m_params.gridDomains) {
	//  //   m_generatedGridOffsets.clear();
	//  //   m_generatedGridDims.clear();
	//  //   m_generatedGridDomains.clear();
	//  //   m_generatedGridScalars.clear();
	//
	//  //   size_t numGrids = m_params.gridData->totalSize();
	//  //   auto *gridData = (helium::Array3D **)m_params.gridData->handlesBegin();
	//  //   auto *gridDomains = m_params.gridDomains->beginAs<box3>();
	//
	//  //   for (size_t i = 0; i < numGrids; ++i) {
	//  //     const helium::Array3D *gd = *(gridData + i);
	//  //     const box3 domain = *(gridDomains + i);
	//
	//  //     m_generatedGridOffsets.push_back(m_generatedGridScalars.size());
	//
	//  //     // from anari's array3d we get the number of vertices, not cells!
	//  //     m_generatedGridDims.push_back(gd->size().x - 1);
	//  //     m_generatedGridDims.push_back(gd->size().y - 1);
	//  //     m_generatedGridDims.push_back(gd->size().z - 1);
	//
	//  //     box1 valueRange{FLT_MAX, -FLT_MAX};
	//  //     for (unsigned z = 0; z < gd->size().z; ++z)
	//  //       for (unsigned y = 0; y < gd->size().y; ++y)
	//  //         for (unsigned x = 0; x < gd->size().x; ++x) {
	//  //           size_t index =
	//  //               z * size_t(gd->size().x) * gd->size().y + y * gd->size().x + x;
	//  //           float f = gd->dataAs<float>()[index];
	//  //           m_generatedGridScalars.push_back(f);
	//  //           valueRange.insert(f);
	//  //         }
	//
	//  //     m_generatedGridDomains.push_back(domain.lower.x);
	//  //     m_generatedGridDomains.push_back(domain.lower.y);
	//  //     m_generatedGridDomains.push_back(domain.lower.z);
	//  //     m_generatedGridDomains.push_back(valueRange.lower);
	//  //     m_generatedGridDomains.push_back(domain.upper.x);
	//  //     m_generatedGridDomains.push_back(domain.upper.y);
	//  //     m_generatedGridDomains.push_back(domain.upper.z);
	//  //     m_generatedGridDomains.push_back(valueRange.upper);
	//  //   }
	//  // }
	//}
	//
	//BNScalarField UnstructuredField::createBarneyScalarField(
	//    BNModel model, int slot) const
	//{
	//  auto ctx = deviceState()->context;
	//  return bnUMeshCreate(model,
	//                       slot,
	//                       (const ::float4 *)m_vertices.data(),
	//                       m_vertices.size(),
	//                       m_indices.data(),
	//                       m_indices.size(),
	//                       m_elementOffsets.data(),
	//                       m_elementOffsets.size(),
	//                       // m_generatedVertices.data(),
	//                       // m_generatedVertices.size() / 4,
	//                       // m_generatedTets.data(),
	//                       // m_generatedTets.size() / 4,
	//                       // m_generatedPyrs.data(),
	//                       // m_generatedPyrs.size() / 5,
	//                       // m_generatedWedges.data(),
	//                       // m_generatedWedges.size() / 6,
	//                       // m_generatedHexes.data(),
	//                       // m_generatedHexes.size() / 8,
	//                       // m_generatedGridOffsets.size(),
	//                       // m_generatedGridOffsets.data(),
	//                       // m_generatedGridDims.data(),
	//                       // m_generatedGridDomains.data(),
	//                       // m_generatedGridScalars.data(),
	//                       // m_generatedGridScalars.size()
	//                       nullptr
	//                       );
	//}
	//
	//box3 UnstructuredField::bounds() const
	//{
	//  return m_bounds;
	//}
	//
	//// BlockStructuredField //
	//
	//BlockStructuredField::BlockStructuredField(CyclesGlobalState *s)
	//    : SpatialField(s)
	//{}
	//
	//void BlockStructuredField::commit()
	//{
	//  Object::commit();
	//
	//  m_params.cellWidth = getParamObject<helium::Array1D>("cellWidth");
	//  m_params.blockBounds = getParamObject<helium::Array1D>("block.bounds");
	//  m_params.blockLevel = getParamObject<helium::Array1D>("block.level");
	//  m_params.blockData = getParamObject<helium::ObjectArray>("block.data");
	//
	//  if (!m_params.blockBounds) {
	//    reportMessage(ANARI_SEVERITY_WARNING,
	//        "missing required parameter 'block.bounds' on amr spatial field");
	//    return;
	//  }
	//
	//  if (!m_params.blockLevel) {
	//    reportMessage(ANARI_SEVERITY_WARNING,
	//        "missing required parameter 'block.level' on amr spatial field");
	//    return;
	//  }
	//
	//  if (!m_params.blockData) {
	//    reportMessage(ANARI_SEVERITY_WARNING,
	//        "missing required parameter 'block.data' on amr spatial field");
	//    return;
	//  }
	//
	//  size_t numBlocks = m_params.blockData->totalSize();
	//  auto *blockBounds = m_params.blockBounds->beginAs<box3i>();
	//  auto *blockLevels = m_params.blockLevel->beginAs<int>();
	//  auto *blockData = (helium::Array3D **)m_params.blockData->handlesBegin();
	//
	//  m_generatedBlockBounds.clear();
	//  m_generatedBlockLevels.clear();
	//  m_generatedBlockOffsets.clear();
	//  m_generatedBlockScalars.clear();
	//
	//  m_bounds.invalidate();
	//
	//  for (size_t i = 0; i < numBlocks; ++i) {
	//    const box3i bounds = *(blockBounds + i);
	//    const int level = *(blockLevels + i);
	//    const helium::Array3D *bd = *(blockData + i);
	//
	//    m_generatedBlockBounds.push_back(bounds.lower.x);
	//    m_generatedBlockBounds.push_back(bounds.lower.y);
	//    m_generatedBlockBounds.push_back(bounds.lower.z);
	//    m_generatedBlockBounds.push_back(bounds.upper.x);
	//    m_generatedBlockBounds.push_back(bounds.upper.y);
	//    m_generatedBlockBounds.push_back(bounds.upper.z);
	//    m_generatedBlockLevels.push_back(level);
	//    m_generatedBlockOffsets.push_back(m_generatedBlockScalars.size());
	//
	//    for (unsigned z = 0; z < bd->size().z; ++z)
	//      for (unsigned y = 0; y < bd->size().y; ++y)
	//        for (unsigned x = 0; x < bd->size().x; ++x) {
	//          size_t index =
	//              z * size_t(bd->size().x) * bd->size().y + y * bd->size().x + x;
	//          float f = bd->dataAs<float>()[index];
	//          m_generatedBlockScalars.push_back(f);
	//        }
	//
	//    box3 worldBounds;
	//    worldBounds.lower = math::float3(bounds.lower.x * (1 << level),
	//        bounds.lower.y * (1 << level),
	//        bounds.lower.z * (1 << level));
	//    worldBounds.upper = math::float3((bounds.upper.x + 1) * (1 << level),
	//        (bounds.upper.y + 1) * (1 << level),
	//        (bounds.upper.z + 1) * (1 << level));
	//    m_bounds.insert(worldBounds);
	//  }
	//}
	//
	//BNScalarField BlockStructuredField::createBarneyScalarField(
	//    BNModel model, int slot) const
	//{
	//  return bnBlockStructuredAMRCreate(model,
	//      slot,
	//      m_generatedBlockBounds.data(),
	//      m_generatedBlockBounds.size() / 6,
	//      m_generatedBlockLevels.data(),
	//      m_generatedBlockOffsets.data(),
	//      m_generatedBlockScalars.data(),
	//      m_generatedBlockScalars.size());
	//}
	//
	//box3 BlockStructuredField::bounds() const
	//{
	//  return m_bounds;
	//}

} // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::SpatialField*);
