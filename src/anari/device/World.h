// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Instance.h"

namespace cycles {

struct World : public Object {
  World(CyclesGlobalState *s);
  ~World() override;

  bool getProperty(const std::string_view &name,
      ANARIDataType type,
      void *ptr,
      uint32_t flags) override;

  void commit() override;

  void setWorldObjectsCurrent();

 private:
  void cleanup();

  helium::IntrusivePtr<ObjectArray> m_zeroSurfaceData;
  helium::IntrusivePtr<ObjectArray> m_zeroLightData;
  helium::IntrusivePtr<Group> m_zeroGroup;
  helium::IntrusivePtr<Instance> m_zeroInstance;

  helium::IntrusivePtr<ObjectArray> m_instanceData;
};

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_SPECIALIZATION(cycles::World *, ANARI_WORLD);
