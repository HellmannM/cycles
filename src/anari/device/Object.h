// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "CyclesGlobalState.h"
#include "cycles_math.h"
// helium
#include "helium/BaseObject.h"
// std
#include <string_view>

namespace cycles {

struct Object : public helium::BaseObject {
  Object(ANARIDataType type, CyclesGlobalState *s);
  virtual ~Object() = default;

  // respond to anariGetProperty()
  virtual bool getProperty(const std::string_view &name,
                           ANARIDataType type,
                           void *ptr,
                           uint32_t flags);

  // respond to parameters set on the object with anariSetParameter()
  virtual void commit();

  // 'true' if the object subtype is both known and has all parameter
  // requirements met
  virtual bool isValid() const;
  virtual void warnIfUnknownObject() const;

  virtual box3 bounds() const;

  virtual void markCommitted() override;

  CyclesGlobalState *deviceState() const;
};

// This type is used to represent object subtypes that are not known by Cycles,
// so it is always invalid.
struct UnknownObject : public Object {
  UnknownObject(ANARIDataType type, std::string_view subtype, CyclesGlobalState *s);
  ~UnknownObject() override;
  bool isValid() const override;
  void warnIfUnknownObject() const override;

 private:
  std::string m_subtype;
};

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_SPECIALIZATION(cycles::Object *, ANARI_OBJECT);