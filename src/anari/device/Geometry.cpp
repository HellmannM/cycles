// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Geometry.h"
// cycles
#include "scene/mesh.h"

namespace cycles {

// Helper types/functions /////////////////////////////////////////////////////

template<int T> struct convert_toFloat4 {
  using base_type = typename anari::ANARITypeProperties<T>::base_type;
  const int nc = anari::ANARITypeProperties<T>::components;
  anari_vec::float4 operator()(const void *src, size_t offset)
  {
    anari_vec::float4 retval = {0.f, 0.f, 0.f, 1.f};
    if constexpr (!anari::isObject(T) && T != ANARI_UNKNOWN)
      anari::ANARITypeProperties<T>::toFloat4(&retval[0], (const base_type *)src + nc * offset);
    return retval;
  }
};

// Geometry definitions ///////////////////////////////////////////////////////

struct Triangle : public Geometry {
  Triangle(CyclesGlobalState *s);
  ~Triangle() override;

  void commit() override;

  ccl::Geometry *makeCyclesGeometry() override;

  box3 bounds() const override;

 private:
  void setVertexPosition(ccl::Mesh *mesh);
  void setPrimitiveIndex(ccl::Mesh *mesh);
  void setVertexNormal(ccl::Mesh *mesh);
  void setVertexColor(ccl::Mesh *mesh);
  void setVertexAttribute(ccl::Mesh *mesh,
                          const helium::IntrusivePtr<Array1D> &array,
                          const char *name);
  void cleanup();

  helium::IntrusivePtr<Array1D> m_index;
  helium::IntrusivePtr<Array1D> m_vertexPosition;
  helium::IntrusivePtr<Array1D> m_vertexNormal;
  helium::IntrusivePtr<Array1D> m_vertexColor;
  helium::IntrusivePtr<Array1D> m_vertexAttribute0;
  helium::IntrusivePtr<Array1D> m_vertexAttribute1;
  helium::IntrusivePtr<Array1D> m_vertexAttribute2;
  helium::IntrusivePtr<Array1D> m_vertexAttribute3;
};

Triangle::Triangle(CyclesGlobalState *s) : Geometry(s)
{
}

Triangle::~Triangle()
{
  cleanup();
}

void Triangle::commit()
{
  Geometry::commit();

  cleanup();

  m_index = getParamObject<Array1D>("primitive.index");
  m_vertexPosition = getParamObject<Array1D>("vertex.position");
  m_vertexNormal = getParamObject<Array1D>("vertex.normal");
  m_vertexColor = getParamObject<Array1D>("vertex.color");
  m_vertexAttribute0 = getParamObject<Array1D>("vertex.attribute0");
  m_vertexAttribute1 = getParamObject<Array1D>("vertex.attribute1");
  m_vertexAttribute2 = getParamObject<Array1D>("vertex.attribute2");
  m_vertexAttribute3 = getParamObject<Array1D>("vertex.attribute3");

  if (!m_vertexPosition) {
    reportMessage(ANARI_SEVERITY_WARNING,
                  "missing required parameter 'vertex.position' on triangle geometry");
    return;
  }

  m_vertexPosition->addCommitObserver(this);
  if (m_index)
    m_index->addCommitObserver(this);
}

ccl::Geometry *Triangle::makeCyclesGeometry()
{
  auto *mesh = new ccl::Mesh();

  if (!m_vertexPosition)
    reportMessage(ANARI_SEVERITY_WARNING, "detected incomplete geometry");

  setVertexPosition(mesh);
  setPrimitiveIndex(mesh);
  setVertexNormal(mesh);
  setVertexColor(mesh);
  setVertexAttribute(mesh, m_vertexAttribute0, "vertex.attribute0");
  setVertexAttribute(mesh, m_vertexAttribute1, "vertex.attribute1");
  setVertexAttribute(mesh, m_vertexAttribute2, "vertex.attribute2");
  setVertexAttribute(mesh, m_vertexAttribute3, "vertex.attribute3");
  return mesh;
}

box3 Triangle::bounds() const
{
  box3 b = empty_box3();
  if (!m_vertexPosition)
    return b;
  std::for_each(m_vertexPosition->beginAs<anari_vec::float3>(),
                m_vertexPosition->endAs<anari_vec::float3>(),
                [&](const anari_vec::float3 &v) { extend(b, make_float3(v[0], v[1], v[2])); });
  return b;
}

void Triangle::setVertexPosition(ccl::Mesh *mesh)
{
  ccl::array<ccl::float3> P;
  auto *dst = P.resize(m_vertexPosition->size());
  std::transform(m_vertexPosition->beginAs<anari_vec::float3>(),
                 m_vertexPosition->endAs<anari_vec::float3>(),
                 dst,
                 [](const anari_vec::float3 &v) { return make_float3(v[0], v[1], v[2]); });
  mesh->set_verts(P);
}

void Triangle::setPrimitiveIndex(ccl::Mesh *mesh)
{
  const uint32_t numTriangles = m_index ? m_index->size() : m_vertexPosition->size() / 3;
  mesh->reserve_mesh(numTriangles * 3, numTriangles);
  for (uint32_t i = 0; i < numTriangles; i++) {
    if (m_index) {
      auto *idxs = m_index->beginAs<anari_vec::uint3>();
      mesh->add_triangle(idxs[i][0], idxs[i][1], idxs[i][2], 0 /* local shaderID */, true);
    }
    else {
      mesh->add_triangle(3 * i + 0, 3 * i + 1, 3 * i + 2, 0 /* local shaderID */, true);
    }
  }
}

void Triangle::setVertexNormal(ccl::Mesh *mesh)
{
  if (!m_vertexNormal)
    return;

  ustring name = ustring("vertex.normal");
  Attribute *attr = mesh->attributes.add(ATTR_STD_VERTEX_NORMAL, name);
  float3 *dst = attr->data_float3();
  std::transform(m_vertexNormal->beginAs<anari_vec::float3>(),
                 m_vertexNormal->endAs<anari_vec::float3>(),
                 dst,
                 [](const anari_vec::float3 &v) { return make_float3(v[0], v[1], v[2]); });
}

void Triangle::setVertexColor(ccl::Mesh *mesh)
{
  auto &array = m_vertexColor;
  if (!array)
    return;

  const void *src = array->data();
  anari::DataType type = array->elementType();

#if 0  // bespoke cycles face-varying version
  Attribute *attr = mesh->attributes.add(ATTR_STD_VERTEX_COLOR, ustring("vertex.color"));
  uchar4 *dst = attr->data_uchar4();
  const uint32_t numTriangles = m_index ? m_index->size() : m_vertexPosition->size() / 3;
  for (uint32_t i = 0; i < numTriangles; i++) {
    auto i0 = 3 * i + 0;
    auto i1 = 3 * i + 1;
    auto i2 = 3 * i + 2;
    auto di0 = i0;
    auto di1 = i1;
    auto di2 = i2;
    if (m_index) {
      auto *idxs = m_index->beginAs<anari_vec::uint3>();
      i0 = idxs[i][0];
      i1 = idxs[i][1];
      i2 = idxs[i][2];
    }

    auto toUchar4 = [](const anari_vec::float4 &v) {
      uchar4 r;
      r.x = uint8_t(v[0] * 255);
      r.y = uint8_t(v[1] * 255);
      r.z = uint8_t(v[2] * 255);
      r.w = uint8_t(v[3] * 255);
      return r;
    };

    auto c0 = anariTypeInvokeColors<anari_vec::float4, convert_toFloat4>(type, src, i0);
    auto c1 = anariTypeInvokeColors<anari_vec::float4, convert_toFloat4>(type, src, i1);
    auto c2 = anariTypeInvokeColors<anari_vec::float4, convert_toFloat4>(type, src, i2);

    dst[di0] = toUchar4(c0);
    dst[di1] = toUchar4(c1);
    dst[di2] = toUchar4(c2);
  }
#else  // true per-vertex
  Attribute *attr = mesh->attributes.add(
      ustring("vertex.color"), TypeDesc::TypeColor, ATTR_ELEMENT_VERTEX);
  attr->std = ATTR_STD_VERTEX_COLOR;
  float3 *dst = attr->data_float3();
  for (uint32_t i = 0; i < array->size(); i++) {
    auto c = anari::anariTypeInvoke<anari_vec::float4, convert_toFloat4>(type, src, i);
    dst[i] = make_float3(c[0], c[1], c[2]);
  }
#endif
}

void Triangle::setVertexAttribute(ccl::Mesh *mesh,
                                  const helium::IntrusivePtr<Array1D> &array,
                                  const char *name)
{
  if (!array)
    return;

  anari::DataType type = array->elementType();
  void *src = array->data();

  Attribute *attr = mesh->attributes.add(ATTR_STD_UV, ustring(name));
  float2 *dst = attr->data_float2();
  size_t i = 0;
  std::for_each(dst, dst + m_vertexPosition->size(), [&](float2 &v) {
    auto r = anari::anariTypeInvoke<anari_vec::float4, convert_toFloat4>(type, src, i);
    v.x = r[0];
    v.y = r[1];
    i++;
  });
}

void Triangle::cleanup()
{
  if (m_index)
    m_index->removeCommitObserver(this);
  if (m_vertexPosition)
    m_vertexPosition->removeCommitObserver(this);
}

///////////////////////////////////////////////////////////////////////////////
// Geometry definitions ///////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

Geometry::Geometry(CyclesGlobalState *s) : Object(ANARI_GEOMETRY, s)
{
  s->objectCounts.geometries++;
}

Geometry::~Geometry()
{
  deviceState()->objectCounts.geometries--;
}

Geometry *Geometry::createInstance(std::string_view type, CyclesGlobalState *s)
{
  if (type == "triangle")
    return new Triangle(s);
  else
    return (Geometry *)new UnknownObject(ANARI_GEOMETRY, type, s);
}

void Geometry::markCommitted()
{
  Object::markCommitted();
  deviceState()->objectUpdates.lastBLSCommitSceneRequest = helium::newTimeStamp();
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Geometry *);
