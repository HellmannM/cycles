// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "FrameOutputDriver.h"
// helium
#include "helium/BaseGlobalDeviceState.h"
// cycles
#include "session/session.h"

namespace ccl {
struct BackgroundNode;
}  // namespace ccl

namespace cycles {

struct CyclesGlobalState : public helium::BaseGlobalDeviceState {
  struct ObjectUpdates {
    helium::TimeStamp lastBLSReconstructSceneRequest{0};
    helium::TimeStamp lastBLSCommitSceneRequest{0};
    helium::TimeStamp lastTLSReconstructSceneRequest{0};
    helium::TimeStamp lastAccumulationReset{0};
  } objectUpdates;

  struct ObjectCounts {
    size_t frames{0};
    size_t cameras{0};
    size_t renderers{0};
    size_t worlds{0};
    size_t instances{0};
    size_t groups{0};
    size_t lights{0};
    size_t surfaces{0};
    size_t geometries{0};
    size_t materials{0};
    size_t arrays{0};
    size_t unknown{0};
  } objectCounts;

  ccl::SessionParams session_params;
  std::unique_ptr<ccl::Session> session;
  size_t sessionSamples{0};

  ccl::Scene *scene{nullptr};
  ccl::SceneParams scene_params;
  ccl::BufferParams buffer_params;

  FrameOutputDriver *output_driver{nullptr};
  Frame *lastFrameRendered{nullptr};

  ccl::BackgroundNode *background{nullptr};
  ccl::BackgroundNode *ambient{nullptr};

  // Helper methods //

  CyclesGlobalState(ANARIDevice d) : helium::BaseGlobalDeviceState(d)
  {
  }
};

#define CYCLES_ANARI_TYPEFOR_SPECIALIZATION(type, anari_type) \
  namespace anari { \
  ANARI_TYPEFOR_SPECIALIZATION(type, anari_type); \
  }

#define CYCLES_ANARI_TYPEFOR_DEFINITION(type) \
  namespace anari { \
  ANARI_TYPEFOR_DEFINITION(type); \
  }

}  // namespace cycles
