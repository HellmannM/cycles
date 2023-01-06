// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Frame.h"

namespace cycles {

Frame::Frame(CyclesGlobalState *s) : helium::BaseFrame(s)
{
  s->objectCounts.frames++;
}

Frame::~Frame()
{
  wait();
  deviceState()->objectCounts.frames--;
}

bool Frame::isValid() const
{
  return m_valid;
}

CyclesGlobalState *Frame::deviceState() const
{
  return (CyclesGlobalState *)helium::BaseObject::m_state;
}

void Frame::commit()
{
  m_renderer = getParamObject<Renderer>("renderer");
  if (!m_renderer) {
    reportMessage(ANARI_SEVERITY_WARNING, "missing required parameter 'renderer' on frame");
  }

  m_camera = getParamObject<Camera>("camera");
  if (!m_camera) {
    reportMessage(ANARI_SEVERITY_WARNING, "missing required parameter 'camera' on frame");
  }

  m_world = getParamObject<World>("world");
  if (!m_world) {
    reportMessage(ANARI_SEVERITY_WARNING, "missing required parameter 'world' on frame");
  }

  m_valid = m_renderer && m_renderer->isValid() && m_camera && m_camera->isValid() && m_world &&
            m_world->isValid();

  m_colorType = getParam<anari::DataType>("channel.color", ANARI_UNKNOWN);
  m_depthType = getParam<anari::DataType>("channel.depth", ANARI_UNKNOWN);

  m_frameData.size = getParam<uint2>("size", make_uint2(10, 10));

  const auto numPixels = m_frameData.size.x * m_frameData.size.y;

  m_perPixelBytes = 4 * (m_colorType == ANARI_FLOAT32_VEC4 ? 4 : 1);
  m_pixelBuffer.resize(numPixels * m_perPixelBytes);
  std::fill(m_pixelBuffer.begin(), m_pixelBuffer.end(), ~0);

  m_depthBuffer.resize(m_depthType == ANARI_FLOAT32 ? numPixels : 0);
}

bool Frame::getProperty(const std::string_view &name,
                        ANARIDataType type,
                        void *ptr,
                        uint32_t flags)
{
  if (type == ANARI_FLOAT32 && name == "duration") {
    if (flags & ANARI_WAIT)
      wait();
    helium::writeToVoidP(ptr, m_duration);
    return true;
  }
  else if (type == ANARI_INT32 && name == "numSamples") {
    if (flags & ANARI_WAIT)
      wait();
    helium::writeToVoidP(ptr, int(deviceState()->sessionSamples));
    return true;
  }
  else if (type == ANARI_BOOL && name == "nextFrameReset") {
    if (flags & ANARI_WAIT)
      wait();
    if (ready())
      deviceState()->commitBuffer.flush();
    bool doReset = resetAccumulationNextFrame();
    helium::writeToVoidP(ptr, doReset);
    return true;
  }

  return 0;
}

void Frame::renderFrame()
{
  wait();

  auto &state = *deviceState();

  state.output_driver->renderBegin(this);

  state.commitBuffer.flush();

  bool resetAccumulation = resetAccumulationNextFrame();

  if (!isValid()) {
    reportMessage(ANARI_SEVERITY_ERROR, "skipping render of incomplete frame object");
    std::fill(m_pixelBuffer.begin(), m_pixelBuffer.end(), ~0);
    state.output_driver->renderEnd();
    return;
  }

  if (resetAccumulation) {
    reportMessage(ANARI_SEVERITY_INFO, "resetting accumulation");

    state.objectUpdates.lastAccumulationReset = helium::newTimeStamp();

    state.lastFrameRendered = this;

    m_camera->setCameraCurrent(m_frameData.size.x, m_frameData.size.y);
    m_world->setWorldObjectsCurrent();

    state.buffer_params.width = m_frameData.size.x;
    state.buffer_params.height = m_frameData.size.y;
    state.buffer_params.full_width = m_frameData.size.x;
    state.buffer_params.full_height = m_frameData.size.y;

    state.session->reset(state.session_params, state.buffer_params);
    state.sessionSamples = 0;
  }

  state.session->set_samples(++state.sessionSamples);
  state.session->start();
}

void *Frame::map(std::string_view channel,
                 uint32_t *width,
                 uint32_t *height,
                 ANARIDataType *pixelType)
{
  wait();

  *width = m_frameData.size.x;
  *height = m_frameData.size.y;

  if (channel == "color" || channel == "channel.color") {
    *pixelType = m_colorType;
    return m_pixelBuffer.data();
  }
  else if (channel == "depth" || channel == "channel.depth") {
    *pixelType = ANARI_FLOAT32;
    return m_depthBuffer.data();
  }
  else {
    *width = 0;
    *height = 0;
    *pixelType = ANARI_UNKNOWN;
    return nullptr;
  }
}

void Frame::unmap(std::string_view channel)
{
  // no-op
}

int Frame::frameReady(ANARIWaitMask m)
{
  if (m == ANARI_NO_WAIT)
    return ready();
  else {
    wait();
    return 1;
  }
}

void Frame::discard()
{
  // no-op
}

bool Frame::ready() const
{
  return deviceState()->output_driver->ready();
}

void Frame::wait() const
{
  deviceState()->output_driver->wait();
}

bool Frame::resetAccumulationNextFrame() const
{
  auto &state = *deviceState();
  return state.objectUpdates.lastAccumulationReset < state.commitBuffer.lastFlush() ||
         state.lastFrameRendered != this;
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Frame *);
