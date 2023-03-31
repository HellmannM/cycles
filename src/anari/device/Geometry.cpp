// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Geometry.h"
// cycles
#include "scene/mesh.h"

namespace cycles {

struct Triangle : public Geometry {
  Triangle(CyclesGlobalState *s);
  ~Triangle() override;

  void commit() override;

  ccl::Geometry *makeCyclesGeometry() override;

 private:
  void setVertices(ccl::Mesh *mesh);
  void setIndices(ccl::Mesh *mesh);
  void setNormals(ccl::Mesh *mesh);
  void cleanup();

  helium::IntrusivePtr<Array1D> m_index;
  helium::IntrusivePtr<Array1D> m_vertexPosition;
  helium::IntrusivePtr<Array1D> m_vertexNormal;
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

  setVertices(mesh);
  setIndices(mesh);
  setNormals(mesh);
  return mesh;
}

void Triangle::setVertices(ccl::Mesh *mesh)
{
  ccl::array<ccl::float3> P;
  auto *dst = P.resize(m_vertexPosition->size());
  std::transform(m_vertexPosition->beginAs<anari_vec::float3>(),
                 m_vertexPosition->endAs<anari_vec::float3>(),
                 dst,
                 [](const anari_vec::float3 &v) { return make_float3(v[0], v[1], v[2]); });
  mesh->set_verts(P);
}

void Triangle::setIndices(ccl::Mesh *mesh)
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

void Triangle::setNormals(ccl::Mesh *mesh)
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
    return (Geometry *)new UnknownObject(ANARI_GEOMETRY, s);
}

void Geometry::markCommitted()
{
  Object::markCommitted();
  deviceState()->objectUpdates.lastBLSCommitSceneRequest = helium::newTimeStamp();
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Geometry *);
