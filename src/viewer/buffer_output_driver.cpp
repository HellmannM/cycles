// Copyright 2022 Jefferson Amstutz
// SPDX-License-Identifier: Apache-2.0

#include "buffer_output_driver.h"

CCL_NAMESPACE_BEGIN

BufferOutputDriver::BufferOutputDriver(const string_view pass) : m_pass(pass) {}

const float *BufferOutputDriver::map(int &w, int &h)
{
  if (!newFrameAvailable())
    return nullptr;
  m_mutex.lock();
  const auto &front = m_buffers[m_current];
  w = m_width;
  h = m_height;
  return front.empty() || m_mapped == m_current ? nullptr : front.data();
}

void BufferOutputDriver::unmap()
{
  m_mapped = m_current;
  m_newFrameAvailable = false;
  m_mutex.unlock();
}

void BufferOutputDriver::write_render_tile(const Tile &tile)
{
  /* Only write the full buffer, no intermediate tiles. */
  if (!(tile.size == tile.full_size)) {
    printf("rejecting partial tile\n");
    return;
  }

  const int width = tile.size.x;
  const int height = tile.size.y;

#if 0
  printf("grabbing %ix%i image buffer\n", width, height);
  fflush(stdout);
#endif

  auto &back = m_buffers[!m_current];

  back.resize(width * height * 4);
  if (!tile.get_pass_pixels(m_pass, 4, back.data())) {
    printf("Failed to read render pass pixels\n");
    return;
  }

  m_mutex.lock();
  m_width = width;
  m_height = height;
  m_current = !m_current;
  m_newFrameAvailable = true;
  m_mutex.unlock();
}

bool BufferOutputDriver::newFrameAvailable()
{
  return m_newFrameAvailable;
}

CCL_NAMESPACE_END
