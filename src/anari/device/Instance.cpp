// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Instance.h"
// std
#include <cstring>

namespace cycles {

Instance::Instance(CyclesGlobalState *s) : Object(ANARI_INSTANCE, s)
{
  s->objectCounts.instances++;
}

Instance::~Instance()
{
  deviceState()->objectCounts.instances--;
}

void Instance::commit()
{
  m_group = getParamObject<Group>("group");

  static const anari_vec::mat4 defaultXfm = {anari_vec::vec4{1.f, 0.f, 0.f, 0.f},
                                             anari_vec::vec4{0.f, 1.f, 0.f, 0.f},
                                             anari_vec::vec4{0.f, 0.f, 1.f, 0.f},
                                             anari_vec::vec4{0.f, 0.f, 0.f, 1.f}};

  auto xfm = getParam<anari_vec::mat4>("transform", defaultXfm);

  std::memcpy(&m_xfm, &xfm, sizeof(m_xfm));
  m_xfm.x.w = xfm[3][0];
  m_xfm.y.w = xfm[3][1];
  m_xfm.z.w = xfm[3][2];
}

Group *Instance::group() const
{
  return m_group.ptr;
}

void Instance::addInstanceObjectsToCurrentWorld()
{
  if (m_group)
    m_group->addGroupToCurrentWorld(m_xfm);
}

box3 Instance::bounds() const
{
  box3 b = empty_box3();
  if (m_group) {
    auto gb = m_group->bounds();
    extend(b, ccl::transform_point(&m_xfm, make_float3(gb.lower.x, gb.lower.y, gb.lower.z)));
    extend(b, ccl::transform_point(&m_xfm, make_float3(gb.lower.x, gb.upper.y, gb.lower.z)));
    extend(b, ccl::transform_point(&m_xfm, make_float3(gb.upper.x, gb.upper.y, gb.lower.z)));
    extend(b, ccl::transform_point(&m_xfm, make_float3(gb.upper.x, gb.lower.y, gb.lower.z)));
    extend(b, ccl::transform_point(&m_xfm, make_float3(gb.lower.x, gb.lower.y, gb.upper.z)));
    extend(b, ccl::transform_point(&m_xfm, make_float3(gb.lower.x, gb.upper.y, gb.upper.z)));
    extend(b, ccl::transform_point(&m_xfm, make_float3(gb.upper.x, gb.upper.y, gb.upper.z)));
    extend(b, ccl::transform_point(&m_xfm, make_float3(gb.upper.x, gb.lower.y, gb.upper.z)));
  }
  return b;
}

bool Instance::isValid() const
{
  return m_group;
}

}  // namespace cycles