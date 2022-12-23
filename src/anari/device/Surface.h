// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Geometry.h"
#include "Material.h"
// cycles
#include "scene/geometry.h"
#include "scene/object.h"

namespace cycles {

struct Surface : public Object {
  Surface(CyclesGlobalState *s);
  ~Surface() override;

  void commit() override;

  const Geometry *geometry() const;
  const Material *material() const;

  ccl::Geometry *cyclesGeometry() const;
  ccl::Object *cyclesObject() const;

  void markCommitted() override;
  bool isValid() const override;

 private:
  helium::IntrusivePtr<Geometry> m_geometry;
  helium::IntrusivePtr<Material> m_material;

  std::unique_ptr<ccl::Geometry> m_cyclesGeometry;
  std::unique_ptr<ccl::Object> m_cyclesObject;
};

}  // namespace cycles
