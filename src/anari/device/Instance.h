// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Group.h"

namespace cycles {

struct Instance : public Object {
  Instance(CyclesGlobalState *s);
  ~Instance() override;

  void commit() override;

  Group *group() const;

  void addInstanceObjectsToCurrentWorld();

  box3 bounds() const override;

  bool isValid() const override;

 private:
  void cleanup();

  helium::IntrusivePtr<Group> m_group;
  ccl::Transform m_xfm;
};


}  // namespace cycles
