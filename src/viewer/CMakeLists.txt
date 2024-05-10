## Copyright 2022 Jefferson Amstutz
## SPDX-License-Identifier: Apache-2.0

return()

project(cycles_viewer LANGUAGES CXX)

find_package(match3D REQUIRED COMPONENTS stb_image)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
PRIVATE
  buffer_output_driver.cpp
  main.cpp
  Orbit.cpp
  tiny_obj_loader_impl.cpp
  ../app/cycles_xml.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE ..)

cycles_external_libraries_append(LIB)

target_link_libraries(${PROJECT_NAME}
PRIVATE
  match3D::match3D
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
