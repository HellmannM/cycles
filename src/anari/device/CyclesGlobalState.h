// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "FrameOutputDriver.h"
// helium
#include "helium/BaseGlobalDeviceState.h"
// cycles
#include "session/session.h"
// std
#include <atomic>

namespace ccl {
struct BackgroundNode;
}  // namespace ccl

namespace cycles {

struct CyclesGlobalState : public helium::BaseGlobalDeviceState {
  struct ObjectUpdates {
#if 0
    helium::TimeStamp lastBLSReconstructSceneRequest{0};
    helium::TimeStamp lastBLSCommitSceneRequest{0};
    helium::TimeStamp lastTLSReconstructSceneRequest{0};
#endif
    helium::TimeStamp lastSceneChange{0};
    helium::TimeStamp lastAccumulationReset{0};
  } objectUpdates;

  struct ObjectCounts {
    std::atomic<size_t> frames{0};
    std::atomic<size_t> cameras{0};
    std::atomic<size_t> renderers{0};
    std::atomic<size_t> worlds{0};
    std::atomic<size_t> instances{0};
    std::atomic<size_t> groups{0};
    std::atomic<size_t> lights{0};
    std::atomic<size_t> surfaces{0};
    std::atomic<size_t> geometries{0};
    std::atomic<size_t> materials{0};
    std::atomic<size_t> arrays{0};
    std::atomic<size_t> unknown{0};
  } objectCounts;

  ccl::SessionParams session_params;
  std::unique_ptr<ccl::Session> session;
  size_t sessionSamples{0};

  ccl::Scene *scene{nullptr};
  ccl::SceneParams scene_params;
  ccl::BufferParams buffer_params;

  FrameOutputDriver *output_driver{nullptr};

  ccl::BackgroundNode *background{nullptr};
  ccl::BackgroundNode *ambient{nullptr};

  // Helper methods //

  CyclesGlobalState(ANARIDevice d);
  void waitOnCurrentFrame() const;
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
