// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "anari_library_cycles_export.h"

#include "Device.h"
// anari
#include "anari/backend/LibraryImpl.h"
#include "anari/ext/debug/DebugObject.h"
#include "anari/type_utility.h"
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

anari::debug_device::ObjectFactory *getDebugFactory();

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

ANARIArray1D CyclesDevice::newArray1D(const void *appMemory,
                                      ANARIMemoryDeleter deleter,
                                      const void *userData,
                                      ANARIDataType type,
                                      uint64_t numItems,
                                      uint64_t byteStride)
{
  initDevice();

  Array1DMemoryDescriptor md;
  md.appMemory = appMemory;
  md.deleter = deleter;
  md.deleterPtr = userData;
  md.elementType = type;
  md.numItems = numItems;
  md.byteStride = byteStride;

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
                                      uint64_t numItems2,
                                      uint64_t byteStride1,
                                      uint64_t byteStride2)
{
  initDevice();

  Array2DMemoryDescriptor md;
  md.appMemory = appMemory;
  md.deleter = deleter;
  md.deleterPtr = userData;
  md.elementType = type;
  md.numItems1 = numItems1;
  md.numItems2 = numItems2;
  md.byteStride1 = byteStride1;
  md.byteStride2 = byteStride2;

  return createObjectForAPI<Array2D, ANARIArray2D>(deviceState(), md);
}

ANARIArray3D CyclesDevice::newArray3D(const void *appMemory,
                                      ANARIMemoryDeleter deleter,
                                      const void *userData,
                                      ANARIDataType type,
                                      uint64_t numItems1,
                                      uint64_t numItems2,
                                      uint64_t numItems3,
                                      uint64_t byteStride1,
                                      uint64_t byteStride2,
                                      uint64_t byteStride3)
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
  md.byteStride1 = byteStride1;
  md.byteStride2 = byteStride2;
  md.byteStride3 = byteStride3;

  return createObjectForAPI<Array3D, ANARIArray3D>(deviceState(), md);
}

// Renderable Objects /////////////////////////////////////////////////////////

ANARILight CyclesDevice::newLight(const char *subtype)
{
  initDevice();
  return getHandleForAPI<ANARILight>(Light::createInstance(subtype, deviceState()));
}

ANARICamera CyclesDevice::newCamera(const char *subtype)
{
  initDevice();
  return getHandleForAPI<ANARICamera>(Camera::createInstance(subtype, deviceState()));
}

ANARIGeometry CyclesDevice::newGeometry(const char *subtype)
{
  initDevice();
  return getHandleForAPI<ANARIGeometry>(Geometry::createInstance(subtype, deviceState()));
}

ANARISpatialField CyclesDevice::newSpatialField(const char *subtype)
{
  initDevice();
  return (ANARISpatialField) new UnknownObject(ANARI_SPATIAL_FIELD, subtype, deviceState());
}

ANARISurface CyclesDevice::newSurface()
{
  initDevice();
  return createObjectForAPI<Surface, ANARISurface>(deviceState());
}

ANARIVolume CyclesDevice::newVolume(const char *subtype)
{
  initDevice();
  return (ANARIVolume) new UnknownObject(ANARI_VOLUME, subtype, deviceState());
}

// Surface Meta-Data //////////////////////////////////////////////////////////

ANARIMaterial CyclesDevice::newMaterial(const char * /*subtype*/)
{
  initDevice();
  return createObjectForAPI<Material, ANARIMaterial>(deviceState());
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

// Instancing /////////////////////////////////////////////////////////////////

ANARIGroup CyclesDevice::newGroup()
{
  initDevice();
  return createObjectForAPI<Group, ANARIGroup>(deviceState());
}

ANARIInstance CyclesDevice::newInstance()
{
  initDevice();
  return createObjectForAPI<Instance, ANARIInstance>(deviceState());
}

// Top-level Worlds ///////////////////////////////////////////////////////////

ANARIWorld CyclesDevice::newWorld()
{
  initDevice();
  return createObjectForAPI<World, ANARIWorld>(deviceState());
}

// Object + Parameter Lifetime Management /////////////////////////////////////

int CyclesDevice::getProperty(ANARIObject object,
                              const char *name,
                              ANARIDataType type,
                              void *mem,
                              uint64_t size,
                              uint32_t mask)
{
  if (handleIsDevice(object)) {
    std::string_view prop = name;
    if (prop == "debugObjects" && type == ANARI_FUNCTION_POINTER) {
      helium::writeToVoidP(mem, getDebugFactory);
      return 1;
    }
    else if (prop == "feature" && type == ANARI_STRING_LIST) {
      helium::writeToVoidP(mem, query_extensions());
      return 1;
    }
    else if (prop == "cycles" && type == ANARI_BOOL) {
      helium::writeToVoidP(mem, true);
      return 1;
    }
  }
  else {
    if (mask == ANARI_WAIT)
      flushCommitBuffer();
    return helium::referenceFromHandle(object).getProperty(name, type, mem, mask);
  }

  return 0;
}

// Frame Manipulation /////////////////////////////////////////////////////////

ANARIFrame CyclesDevice::newFrame()
{
  initDevice();
  return createObjectForAPI<Frame, ANARIFrame>(deviceState());
}

// Frame Rendering ////////////////////////////////////////////////////////////

ANARIRenderer CyclesDevice::newRenderer(const char *subtype)
{
  initDevice();
  return createObjectForAPI<Renderer, ANARIRenderer>(deviceState());
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

  state.commitBuffer.clear();

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

  auto reportLeaks = [&](size_t &count, const char *handleType) {
    if (count != 0) {
      reportMessage(ANARI_SEVERITY_WARNING, "detected %zu leaked %s objects", count, handleType);
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

  if (state.objectCounts.unknown != 0) {
    reportMessage(ANARI_SEVERITY_WARNING,
                  "detected %zu leaked ANARIObject objects created by unknown subtypes",
                  state.objectCounts.unknown);
  }
}

void CyclesDevice::initDevice()
{
  if (m_initialized)
    return;

  reportMessage(ANARI_SEVERITY_DEBUG, "initializing cycles device (%p)", this);

  auto devices = ccl::Device::available_devices();
  for (ccl::DeviceInfo &info : devices) {
    reportMessage(ANARI_SEVERITY_DEBUG,
                  "Found Cycles Device: %-7s| %s",
                  ccl::Device::string_from_type(info.type).c_str(),
                  info.description.c_str());
  }

  auto &state = *deviceState();

  auto *useGPU = getenv("CYCLES_ANARI_USE_GPU");

  state.session_params.device.type = useGPU ? ccl::DEVICE_OPTIX : ccl::DEVICE_CPU;
  state.session_params.background = false;
  state.session_params.headless = false;
  state.session_params.use_auto_tile = false;
  state.session_params.tile_size = 64;  // 2048
  state.session_params.use_resolution_divider = false;
  state.session_params.samples = 1;

  state.session = std::make_unique<ccl::Session>(state.session_params, state.scene_params);
  state.scene = state.session->scene;

  // We cannot use adaptive sampling based on ANARI's async execution model,
  // me _must_ know that the next sample will get executed to trigger completion
  // code signaling the frame is complete.
  state.scene->integrator->set_use_adaptive_sampling(false);

  ccl::Pass *pass = state.scene->create_node<ccl::Pass>();
  pass->set_name(OIIO::ustring("combined"));
  pass->set_type(ccl::PASS_COMBINED);

  auto output_driver = std::make_unique<FrameOutputDriver>();
  state.output_driver = output_driver.get();

  state.session->set_output_driver(std::move(output_driver));

  // setup background shader (divides out ambient and bg color)
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

  // setup global light shader
  {
    auto *shader = state.scene->default_light;
    auto *graph = new ccl::ShaderGraph();

    auto *emission = graph->create_node<ccl::EmissionNode>();
    emission->set_color(make_float3(1.f, 1.f, 1.f));
    emission->set_strength(1.0f);
    graph->add(emission);

    graph->connect(emission->output("Emission"), graph->output()->input("Surface"));

    shader->name = "default_anari_light";
    shader->set_graph(graph);
    shader->reference();
  }

  m_initialized = true;
}

CyclesGlobalState *CyclesDevice::deviceState() const
{
  return (CyclesGlobalState *)helium::BaseDevice::m_state.get();
}

}  // namespace cycles

extern "C" CYCLES_DEVICE_INTERFACE ANARI_DEFINE_LIBRARY_NEW_DEVICE(cycles, library, _subtype)
{
  auto subtype = std::string_view(_subtype);
  if (subtype == "default" || subtype == "cycles")
    return (ANARIDevice) new cycles::CyclesDevice(library);
  return nullptr;
}

extern "C" CYCLES_DEVICE_INTERFACE ANARI_DEFINE_LIBRARY_GET_DEVICE_SUBTYPES(cycles, libdata)
{
  static const char *devices[] = {"cycles", nullptr};
  return devices;
}

extern "C" CYCLES_DEVICE_INTERFACE ANARI_DEFINE_LIBRARY_GET_OBJECT_SUBTYPES(cycles,
                                                                            library,
                                                                            deviceSubtype,
                                                                            objectType)
{
  return cycles::query_object_types(objectType);
}

extern "C" CYCLES_DEVICE_INTERFACE ANARI_DEFINE_LIBRARY_GET_OBJECT_PROPERTY(
    cycles, library, deviceSubtype, objectSubtype, objectType, propertyName, propertyType)
{
  return cycles::query_object_info(objectType, objectSubtype, propertyName, propertyType);
}

extern "C" CYCLES_DEVICE_INTERFACE ANARI_DEFINE_LIBRARY_GET_PARAMETER_PROPERTY(cycles,
                                                                               library,
                                                                               deviceSubtype,
                                                                               objectSubtype,
                                                                               objectType,
                                                                               parameterName,
                                                                               parameterType,
                                                                               propertyName,
                                                                               propertyType)
{
  return cycles::query_param_info(
      objectType, objectSubtype, parameterName, parameterType, propertyName, propertyType);
}

extern "C" CYCLES_DEVICE_INTERFACE ANARIDevice
anariNewCyclesDevice(ANARIStatusCallback defaultCallback, const void *userPtr)
{
  return (ANARIDevice) new cycles::CyclesDevice(defaultCallback, userPtr);
}
