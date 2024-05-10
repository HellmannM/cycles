## Copyright 2022 Jefferson Amstutz
## SPDX-License-Identifier: Apache-2.0

include(CMakePrintHelpers)

## Establish Project ##

project(anari_library_cycles LANGUAGES CXX)

## Dependencies ##

find_package(anari 0.10.0 REQUIRED)

## Code generation

anari_generate_queries(
  NAME cycles
  PREFIX CyclesDevice
  CPP_NAMESPACE cycles
  JSON_ROOT_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}
  JSON_DEFINITIONS_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cycles_device.json
)

## Build device library ##

add_library(${PROJECT_NAME} MODULE)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
  EXPORT_MACRO_NAME "CYCLES_DEVICE_INTERFACE"
)

target_sources(${PROJECT_NAME}
PRIVATE
  CyclesDeviceQueries.cpp
  CyclesGlobalState.cpp
  Camera.cpp
  Device.cpp
  Frame.cpp
  FrameOutputDriver.cpp
  Geometry.cpp
  Group.cpp
  Instance.cpp
  Library.cpp
  Light.cpp
  Material.cpp
  Object.cpp
  Renderer.cpp
  Surface.cpp
  World.cpp
)

target_include_directories(${PROJECT_NAME}
PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/../..
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
)

cycles_external_libraries_append(LIB)

target_link_libraries(${PROJECT_NAME}
PRIVATE
  anari::helium
  cycles_device
  cycles_kernel
  cycles_scene
  cycles_session
  cycles_bvh
  cycles_subd
  cycles_graph
  cycles_util
  extern_sky
  ${OPENIMAGEIO_LIBRARIES}
  ${LIB}
)
