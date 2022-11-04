// Copyright 2022 Jefferson Amstutz
// SPDX-License-Identifier: Apache-2.0

#include "session/output_driver.h"
// std
#include <mutex>
#include <string>
#include <string_view>
#include <vector>

CCL_NAMESPACE_BEGIN

class BufferOutputDriver : public OutputDriver
{
 public:
  BufferOutputDriver(const string_view pass);

  void write_render_tile(const Tile &tile) override;

  bool newFrameAvailable();

  const float *map(int &width, int &height);
  void unmap();

 protected:
  std::vector<float> m_buffers[2];
  int m_width{0};
  int m_height{0};
  int m_current{1};
  int m_mapped{0};
  bool m_newFrameAvailable{false};
  std::string m_pass;
  std::mutex m_mutex;
};

CCL_NAMESPACE_END
