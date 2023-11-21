// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "CyclesGlobalState.h"
#include "Frame.h"

namespace cycles {

CyclesGlobalState::CyclesGlobalState(ANARIDevice d) : helium::BaseGlobalDeviceState(d) {}

void CyclesGlobalState::waitOnCurrentFrame() const
{
  output_driver->wait();
}

}  // namespace cycles
