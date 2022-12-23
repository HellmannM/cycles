// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

// cycles
#include "util/transform.h"
#include "util/types.h"
// anari
#include <anari/anari_cpp.hpp>
#include <anari/anari_cpp/ext/std.h>
// std
#include <limits>

namespace cycles {

using namespace ccl;

namespace anari_vec {

using float2 = std::array<float, 2>;
using float3 = std::array<float, 3>;
using float4 = std::array<float, 4>;
using uint3 = std::array<uint32_t, 3>;

}  // namespace anari_math

// Types //////////////////////////////////////////////////////////////////////

template<typename T> struct range_t {
  using element_t = T;

  range_t() = default;
  range_t(const T &t) : lower(t), upper(t)
  {
  }
  range_t(const T &_lower, const T &_upper) : lower(_lower), upper(_upper)
  {
  }

  range_t<T> &extend(const T &t)
  {
    lower = min(lower, t);
    upper = max(upper, t);
    return *this;
  }

  range_t<T> &extend(const range_t<T> &t)
  {
    lower = min(lower, t.lower);
    upper = max(upper, t.upper);
    return *this;
  }

  T lower{T(std::numeric_limits<float>::max())};
  T upper{T(-std::numeric_limits<float>::max())};
};

using box1 = range_t<float>;
using box2 = range_t<float2>;
using box3 = range_t<float3>;

// Functions //////////////////////////////////////////////////////////////////

inline float radians(float degrees)
{
  return degrees * float(M_PI) / 180.f;
}

}  // namespace cycles

namespace anari {

ANARI_TYPEFOR_SPECIALIZATION(cycles::float2, ANARI_FLOAT32_VEC2);
ANARI_TYPEFOR_SPECIALIZATION(cycles::float3, ANARI_FLOAT32_VEC3);
ANARI_TYPEFOR_SPECIALIZATION(cycles::float4, ANARI_FLOAT32_VEC4);
ANARI_TYPEFOR_SPECIALIZATION(cycles::int2, ANARI_INT32_VEC2);
ANARI_TYPEFOR_SPECIALIZATION(cycles::int3, ANARI_INT32_VEC3);
ANARI_TYPEFOR_SPECIALIZATION(cycles::int4, ANARI_INT32_VEC4);
ANARI_TYPEFOR_SPECIALIZATION(cycles::uint2, ANARI_UINT32_VEC2);
ANARI_TYPEFOR_SPECIALIZATION(cycles::uint3, ANARI_UINT32_VEC3);
ANARI_TYPEFOR_SPECIALIZATION(cycles::uint4, ANARI_UINT32_VEC4);

#ifdef CYCLES_ANARI_DEFINITIONS
ANARI_TYPEFOR_DEFINITION(cycles::float2);
ANARI_TYPEFOR_DEFINITION(cycles::float3);
ANARI_TYPEFOR_DEFINITION(cycles::float4);
ANARI_TYPEFOR_DEFINITION(cycles::int2);
ANARI_TYPEFOR_DEFINITION(cycles::int3);
ANARI_TYPEFOR_DEFINITION(cycles::int4);
ANARI_TYPEFOR_DEFINITION(cycles::uint2);
ANARI_TYPEFOR_DEFINITION(cycles::uint3);
ANARI_TYPEFOR_DEFINITION(cycles::uint4);
#endif

}  // namespace anari
