﻿// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Device.h"
// anari
#include "anari/anari_cpp.hpp"
// cycles
#include "scene/background.h"
#include "scene/integrator.h"

#include "Frame.h"
#include "array/Array1D.h"
#include "array/Array2D.h"
#include "array/Array3D.h"
#include "array/ObjectArray.h"

#include "FrameOutputDriver.h"

namespace cycles {

///////////////////////////////////////////////////////////////////////////////
// Generated function declarations ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

const char **query_object_types(ANARIDataType type);

const void *query_object_info(ANARIDataType type,
                              const char *subtype,
                              const char *infoName,
                              ANARIDataType infoType);

const void *query_param_info(ANARIDataType type,
                             const char *subtype,
                             const char *paramName,
                             ANARIDataType paramType,
                             const char *infoName,
                             ANARIDataType infoType);

const char **query_extensions();

///////////////////////////////////////////////////////////////////////////////
// Helper functions ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template<typename HANDLE_T, typename OBJECT_T> inline HANDLE_T getHandleForAPI(OBJECT_T *object)
{
  return (HANDLE_T)object;
}

template<typename OBJECT_T, typename HANDLE_T, typename... Args>
inline HANDLE_T createObjectForAPI(CyclesGlobalState *s, Args &&...args)
{
  return getHandleForAPI<HANDLE_T>(new OBJECT_T(s, std::forward<Args>(args)...));
}

///////////////////////////////////////////////////////////////////////////////
// CyclesDevice definitions ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Data Arrays ////////////////////////////////////////////////////////////////

void *CyclesDevice::mapArray(ANARIArray a)
{
  deviceState()->waitOnCurrentFrame();
  return helium::BaseDevice::mapArray(a);
}

// API Objects ////////////////////////////////////////////////////////////////

ANARIArray1D CyclesDevice::newArray1D(const void *appMemory,
                                      ANARIMemoryDeleter deleter,
                                      const void *userData,
                                      ANARIDataType type,
                                      uint64_t numItems)
{
  initDevice();

  Array1DMemoryDescriptor md;
  md.appMemory = appMemory;
  md.deleter = deleter;
  md.deleterPtr = userData;
  md.elementType = type;
  md.numItems = numItems;

  if (anari::isObject(type))
    return createObjectForAPI<ObjectArray, ANARIArray1D>(deviceState(), md);
  else
    return createObjectForAPI<Array1D, ANARIArray1D>(deviceState(), md);
}

ANARIArray2D CyclesDevice::newArray2D(const void *appMemory,
                                      ANARIMemoryDeleter deleter,
                                      const void *userData,
                                      ANARIDataType type,
                                      uint64_t numItems1,
                                      uint64_t numItems2)
{
  initDevice();

  Array2DMemoryDescriptor md;
  md.appMemory = appMemory;
  md.deleter = deleter;
  md.deleterPtr = userData;
  md.elementType = type;
  md.numItems1 = numItems1;
  md.numItems2 = numItems2;

  return createObjectForAPI<Array2D, ANARIArray2D>(deviceState(), md);
}

ANARIArray3D CyclesDevice::newArray3D(const void *appMemory,
                                      ANARIMemoryDeleter deleter,
                                      const void *userData,
                                      ANARIDataType type,
                                      uint64_t numItems1,
                                      uint64_t numItems2,
                                      uint64_t numItems3)
{
  initDevice();

  Array3DMemoryDescriptor md;
  md.appMemory = appMemory;
  md.deleter = deleter;
  md.deleterPtr = userData;
  md.elementType = type;
  md.numItems1 = numItems1;
  md.numItems2 = numItems2;
  md.numItems3 = numItems3;

  return createObjectForAPI<Array3D, ANARIArray3D>(deviceState(), md);
}

ANARICamera CyclesDevice::newCamera(const char *subtype)
{
  initDevice();
  return getHandleForAPI<ANARICamera>(Camera::createInstance(subtype, deviceState()));
}

ANARIFrame CyclesDevice::newFrame()
{
  initDevice();
  return createObjectForAPI<Frame, ANARIFrame>(deviceState());
}

ANARIGeometry CyclesDevice::newGeometry(const char *subtype)
{
  initDevice();
  return getHandleForAPI<ANARIGeometry>(Geometry::createInstance(subtype, deviceState()));
}

ANARIGroup CyclesDevice::newGroup()
{
  initDevice();
  return createObjectForAPI<Group, ANARIGroup>(deviceState());
}

ANARIInstance CyclesDevice::newInstance(const char * /*subtype*/)
{
  initDevice();
  return createObjectForAPI<Instance, ANARIInstance>(deviceState());
}

ANARILight CyclesDevice::newLight(const char *subtype)
{
  initDevice();
  return getHandleForAPI<ANARILight>(Light::createInstance(subtype, deviceState()));
}

ANARIMaterial CyclesDevice::newMaterial(const char *subtype)
{
  initDevice();
  return getHandleForAPI<ANARIMaterial>(Material::createInstance(subtype, deviceState()));
}

ANARIRenderer CyclesDevice::newRenderer(const char *subtype)
{
  initDevice();
  return createObjectForAPI<Renderer, ANARIRenderer>(deviceState());
}

ANARISampler CyclesDevice::newSampler(const char *subtype)
{
  initDevice();
#if 1
  return (ANARISampler) new UnknownObject(ANARI_SAMPLER, subtype, deviceState());
#else
  return getHandleForAPI<ANARISampler>(Sampler::createInstance(subtype, deviceState()));
#endif
}

ANARISpatialField CyclesDevice::newSpatialField(const char *subtype)
{
  initDevice();
  //return (ANARISpatialField) new UnknownObject(ANARI_SPATIAL_FIELD, subtype, deviceState());
  return getHandleForAPI<ANARISpatialField>(SpatialField::createInstance(subtype, deviceState()));
}

ANARISurface CyclesDevice::newSurface()
{
  initDevice();
  return createObjectForAPI<Surface, ANARISurface>(deviceState());
}

ANARIVolume CyclesDevice::newVolume(const char *subtype)
{
  initDevice();
  //return (ANARIVolume) new UnknownObject(ANARI_VOLUME, subtype, deviceState());
  return getHandleForAPI<ANARIVolume>(Volume::createInstance(subtype, deviceState()));
}

ANARIWorld CyclesDevice::newWorld()
{
  initDevice();
  return createObjectForAPI<World, ANARIWorld>(deviceState());
}

// Query functions ////////////////////////////////////////////////////////////

const char **CyclesDevice::getObjectSubtypes(ANARIDataType objectType)
{
  return cycles::query_object_types(objectType);
}

const void *CyclesDevice::getObjectInfo(ANARIDataType objectType,
                                        const char *objectSubtype,
                                        const char *infoName,
                                        ANARIDataType infoType)
{
  return cycles::query_object_info(objectType, objectSubtype, infoName, infoType);
}

const void *CyclesDevice::getParameterInfo(ANARIDataType objectType,
                                           const char *objectSubtype,
                                           const char *parameterName,
                                           ANARIDataType parameterType,
                                           const char *infoName,
                                           ANARIDataType infoType)
{
  return cycles::query_param_info(
      objectType, objectSubtype, parameterName, parameterType, infoName, infoType);
}

// Object + Parameter Lifetime Management /////////////////////////////////////

int CyclesDevice::getProperty(ANARIObject object,
                              const char *name,
                              ANARIDataType type,
                              void *mem,
                              uint64_t size,
                              uint32_t mask)
{
  if (mask == ANARI_WAIT) {
    auto lock = scopeLockObject();
#if 0  // TODO: this causes a crash in anariViewer...(???)
    deviceState()->waitOnCurrentFrame();
#endif
  }

  return helium::BaseDevice::getProperty(object, name, type, mem, size, mask);
}

// Other CyclesDevice definitions /////////////////////////////////////////////

CyclesDevice::CyclesDevice(ANARIStatusCallback cb, const void *ptr) : helium::BaseDevice(cb, ptr)
{
  m_state = std::make_unique<CyclesGlobalState>(this_device());
  deviceCommitParameters();
}

CyclesDevice::CyclesDevice(ANARILibrary l) : helium::BaseDevice(l)
{
  m_state = std::make_unique<CyclesGlobalState>(this_device());
  deviceCommitParameters();
}

CyclesDevice::~CyclesDevice()
{
  auto &state = *deviceState();

  state.session->cancel(true);
  state.session->wait();

  state.commitBufferClear();

  // We don't want the ccl::Scene deleting these objects, they are already gone
  state.scene->lights.clear();
  state.scene->shaders.clear();

  reportMessage(ANARI_SEVERITY_DEBUG, "destroying cycles device (%p)", this);

  // NOTE: These object leak warnings are not required to be done by
  //       implementations as the debug layer in the SDK is far more
  //       comprehensive and designed for detecting bugs like this. However
  //       these simple checks are very straightforward to implement and do not
  //       really add substantial code complexity, so they are provided out of
  //       convenience.

  auto reportLeaks = [&](auto &count, const char *handleType) {
    size_t c = count.load();
    if (c != 0) {
      reportMessage(ANARI_SEVERITY_WARNING, "detected %zu leaked %s objects", c, handleType);
    }
  };

  reportLeaks(state.objectCounts.frames, "ANARIFrame");
  reportLeaks(state.objectCounts.cameras, "ANARICamera");
  reportLeaks(state.objectCounts.renderers, "ANARIRenderer");
  reportLeaks(state.objectCounts.worlds, "ANARIWorld");
  reportLeaks(state.objectCounts.instances, "ANARIInstance");
  reportLeaks(state.objectCounts.groups, "ANARIGroup");
  reportLeaks(state.objectCounts.lights, "ANARILight");
  reportLeaks(state.objectCounts.surfaces, "ANARISurface");
  reportLeaks(state.objectCounts.geometries, "ANARIGeometry");
  reportLeaks(state.objectCounts.materials, "ANARIMaterial");

  if (state.objectCounts.unknown.load() != 0) {
    reportMessage(ANARI_SEVERITY_WARNING,
                  "detected %zu leaked ANARIObject objects created by unknown subtypes",
                  state.objectCounts.unknown.load());
  }
}

int CyclesDevice::deviceGetProperty(const char *name, ANARIDataType type, void *mem, uint64_t size)
{
  std::string_view prop = name;
  if (prop == "feature" && type == ANARI_STRING_LIST) {
    helium::writeToVoidP(mem, query_extensions());
    return 1;
  }
  else if (prop == "cycles" && type == ANARI_BOOL) {
    helium::writeToVoidP(mem, true);
    return 1;
  }
  return 0;
}

void CyclesDevice::initDevice()
{
  if (m_initialized)
    return;

  reportMessage(ANARI_SEVERITY_DEBUG, "initializing cycles device (%p)", this);

#if 0 // can cause crash in VisRTX viewer app...(???)
  auto devices = ccl::Device::available_devices();
  for (ccl::DeviceInfo &info : devices) {
    reportMessage(ANARI_SEVERITY_DEBUG,
                  "Found Cycles Device: %-7s| %s",
                  ccl::Device::string_from_type(info.type).c_str(),
                  info.description.c_str());
  }
#endif

  auto &state = *deviceState();

  auto *useGPU = getenv("CYCLES_ANARI_USE_GPU");

  //state.session_params.device.type = useGPU ? ccl::DEVICE_OPTIX : ccl::DEVICE_CPU;
  state.session_params.device.type = ccl::DEVICE_CPU;
  if (useGPU) {
    printf("useGPU: %s\n", useGPU);
    
    if (!strcmp(useGPU, "CPU"))
      state.session_params.device.type = ccl::DEVICE_CPU;
    else if (!strcmp(useGPU, "OPTIX"))
      state.session_params.device.type = ccl::DEVICE_OPTIX;
    else if (!strcmp(useGPU, "CUDA"))
      state.session_params.device.type = ccl::DEVICE_CUDA;      
    else if (!strcmp(useGPU, "HIP"))
      state.session_params.device.type = ccl::DEVICE_HIP;            
  }

  state.session_params.background = false;
  state.session_params.headless = false;
  state.session_params.use_auto_tile = false;
  state.session_params.tile_size = 64;  // 2048
  state.session_params.use_resolution_divider = false;
  state.session_params.samples = 1;

  //state.session_params.threads = 1;

  state.session = std::make_unique<ccl::Session>(state.session_params, state.scene_params);
  state.scene = state.session->scene;

  // We cannot use adaptive sampling based on ANARI's async execution model,
  // me _must_ know that the next sample will get executed to trigger completion
  // code signaling the frame is complete.
  state.scene->integrator->set_use_adaptive_sampling(false);

  state.scene->integrator->set_volume_step_rate(0.1f);
  state.scene->integrator->set_volume_max_steps(64);
  //state.scene->integrator->set_max_volume_bounce(64);
  
  // combined
  ccl::Pass *pass_combined = state.scene->create_node<ccl::Pass>();
  pass_combined->set_name(OIIO::ustring("combined"));
  pass_combined->set_type(ccl::PASS_COMBINED);

  // depth
  ccl::Pass *pass_depth = state.scene->create_node<ccl::Pass>();
  pass_depth->set_name(OIIO::ustring("depth"));
  pass_depth->set_type(ccl::PASS_DEPTH);

  auto output_driver = std::make_unique<FrameOutputDriver>();
  state.output_driver = output_driver.get();

  state.session->set_output_driver(std::move(output_driver));

  // setup background shader (divides out ambient and bg color)
#if 0
  {
    auto *shader = state.scene->default_background;
    auto *graph = new ccl::ShaderGraph();

    auto *mix = graph->create_node<ccl::MixClosureNode>();
    graph->add(mix);

    auto *lightPath = graph->create_node<ccl::LightPathNode>();
    graph->add(lightPath);

    auto *bg = graph->create_node<ccl::BackgroundNode>();
    bg->name = "background_shader";
    graph->add(bg);

    auto *ambient = graph->create_node<ccl::BackgroundNode>();
    ambient->name = "ambient_shader";
    graph->add(ambient);

    state.background = bg;
    state.ambient = ambient;

    graph->connect(ambient->output("Background"), mix->input("Closure1"));
    graph->connect(bg->output("Background"), mix->input("Closure2"));
    graph->connect(lightPath->output("Is Camera Ray"), mix->input("Fac"));
    graph->connect(mix->output("Closure"), graph->output()->input("Surface"));

    shader->set_graph(graph);

    state.scene->background->set_shader(state.scene->default_background);
    state.scene->background->set_use_shader(true);
  }
#else
  {
      auto* shader = state.scene->default_background;
      auto* graph = new ccl::ShaderGraph();

#if 0

      // Create the texture coordinate node
      auto* tex_coord = graph->create_node<ccl::TextureCoordinateNode>();
      tex_coord->set_owner(graph);
      graph->add(tex_coord);

      // Create the mapping node to handle rotation
      auto* mapping = graph->create_node<ccl::MappingNode>();
      mapping->set_owner(graph);
      //mapping->set_rotation(ccl::make_float3(M_PI_2, 0.0f, 0.0f));  // Rotate 90 degrees around the Y axis
      graph->add(mapping);

      // Create the sky texture node
      auto* sky = graph->create_node<ccl::SkyTextureNode>();
      sky->set_owner(graph);
      sky->set_sky_type(ccl::NODE_SKY_PREETHAM);
      graph->add(sky);

      // Create the background node
      auto* bg = graph->create_node<ccl::BackgroundNode>();
      bg->set_strength(0.5f);
      bg->set_owner(graph);
      graph->add(bg);

      //graph->connect(sky->output("Color"), bg->input("Color"));
      //graph->connect(bg->output("Background"), graph->output()->input("Surface"));
      
      //Connect the nodes
      graph->connect(tex_coord->output("Generated"), mapping->input("Vector"));
      graph->connect(mapping->output("Vector"), sky->input("Vector"));
      graph->connect(sky->output("Color"), bg->input("Color"));
      graph->connect(bg->output("Background"), graph->output()->input("Surface"));

      shader->set_graph(graph);
      state.scene->background->set_shader(state.scene->default_background);
      state.scene->background->set_use_shader(true);
      shader->tag_update(state.scene);

#endif

      // Create the background node
      auto* bg = graph->create_node<ccl::BackgroundNode>();
      bg->set_strength(0.3f);
      bg->set_color(ccl::make_float3(0.3f, 0.3f, 0.3f));
      bg->set_owner(graph);
      graph->add(bg);

      graph->connect(bg->output("Background"), graph->output()->input("Surface"));

      shader->set_graph(graph);
      state.scene->background->set_shader(state.scene->default_background);
      state.scene->background->set_use_shader(true);
      shader->tag_update(state.scene);

  }
#endif

#if 0
  // setup global light shader
  {
    auto *shader = state.scene->default_light;
    auto *graph = new ccl::ShaderGraph();

    auto *emission = graph->create_node<ccl::EmissionNode>();
    emission->set_color(make_float3(1.f, 1.f, 1.f));
    emission->set_strength(4.0f);  // to match VisRTX
    graph->add(emission);

    graph->connect(emission->output("Emission"), graph->output()->input("Surface"));

    shader->name = "default_anari_light";
    shader->set_graph(graph);
    shader->reference();
  }
#endif

#if 0
  //log
  int verbosity = 3;
  util_logging_start();
  util_logging_verbosity_set(verbosity);
#endif

  m_initialized = true;
}

CyclesGlobalState *CyclesDevice::deviceState() const
{
  return (CyclesGlobalState *)helium::BaseDevice::m_state.get();
}

}  // namespace cycles