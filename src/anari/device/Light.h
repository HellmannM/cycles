// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Object.h"
// cycles
#include "scene/light.h"
// std
#include <memory>

namespace cycles {

struct Light : public Object {

  Light(CyclesGlobalState *s);
  ~Light() override;

  static Light *createInstance(std::string_view type, CyclesGlobalState *state);

  virtual void commit() override;

  ccl::Light *cyclesLight() const;

 protected:
  std::unique_ptr<ccl::Light> m_cyclesLight;

  anari_vec::float3 m_color;
};

}  // namespace cycles
