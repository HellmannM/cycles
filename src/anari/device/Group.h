// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Light.h"
#include "Surface.h"
#include "array/ObjectArray.h"

namespace cycles {

struct Group : public Object {
  Group(CyclesGlobalState *s);
  ~Group() override;

  void commit() override;

  void addGroupToCurrentWorld(const ccl::Transform &xfm) const;

  box3 bounds() const override;

 private:
  void cleanup();

  helium::IntrusivePtr<ObjectArray> m_surfaceData;
  helium::IntrusivePtr<ObjectArray> m_lightData;
};

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_SPECIALIZATION(cycles::Group *, ANARI_WORLD);

