## Copyright 2022 Jefferson Amstutz
## SPDX-License-Identifier: Apache-2.0

include(CMakePrintHelpers)

## Establish Project ##

project(anari_library_cycles LANGUAGES CXX)

## Dependencies ##

find_package(anari 0.3.0 REQUIRED)

## Code generation

option(CYCLES_ENABLE_CODE_GEN "Enable Python code generation for query updates" OFF)
mark_as_advanced(CYCLES_ENABLE_CODE_GEN)
if (CYCLES_ENABLE_CODE_GEN)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  add_custom_target(generate_cycles_device
    COMMAND ${Python3_EXECUTABLE} ${ANARI_CODE_GEN_ROOT}/generate_queries.py
      --json ${ANARI_CODE_GEN_ROOT}
      --json ${CMAKE_CURRENT_LIST_DIR}
      --prefix CyclesDevice
      --device ${CMAKE_CURRENT_SOURCE_DIR}/cycles_device.json
      --namespace cycles
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS cycles_device.json
  )

  add_custom_target(generate_cycles_debug_objects
    COMMAND ${Python3_EXECUTABLE} ${ANARI_CODE_GEN_ROOT}/generate_debug_objects.py
      --json ${ANARI_CODE_GEN_ROOT}
      --json ${CMAKE_CURRENT_LIST_DIR}
      --device ${CMAKE_CURRENT_SOURCE_DIR}/cycles_device.json
      --prefix CyclesDevice
      --namespace cycles
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS cycles_device.json
  )

  add_custom_target(generate_all)
  add_dependencies(generate_all
    generate_cycles_device
    generate_cycles_debug_objects
  )
endif()

## Build device library ##

add_library(${PROJECT_NAME} MODULE)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME}
  EXPORT_MACRO_NAME "CYCLES_DEVICE_INTERFACE"
)

target_sources(${PROJECT_NAME}
PRIVATE
  CyclesDeviceDebugFactory.cpp
  CyclesDeviceQueries.cpp
  Camera.cpp
  Device.cpp
  Frame.cpp
  FrameOutputDriver.cpp
  Geometry.cpp
  Light.cpp
  Material.cpp
  Object.cpp
  Renderer.cpp
  Surface.cpp
  World.cpp
  array/Array.cpp
  array/Array1D.cpp
  array/Array2D.cpp
  array/Array3D.cpp
  array/ObjectArray.cpp
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
  anari::anari_library_debug
  cycles_device
  cycles_kernel
  cycles_scene
  cycles_session
  cycles_bvh
  cycles_subd
  cycles_graph
  cycles_util
  extern_sky
  ${LIB}
)
