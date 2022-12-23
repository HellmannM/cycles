// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Object.h"
// std
#include <atomic>
#include <cstdarg>

namespace cycles {

// Object definitions /////////////////////////////////////////////////////////

Object::Object(ANARIDataType type, CyclesGlobalState *s) : helium::BaseObject(type, s)
{
}

void Object::commit()
{
  // no-op
}

bool Object::getProperty(const std::string_view &name,
                         ANARIDataType type,
                         void *ptr,
                         uint32_t flags)
{
  if (name == "valid" && type == ANARI_BOOL) {
    helium::writeToVoidP(ptr, isValid());
    return true;
  }

  return false;
}

bool Object::isValid() const
{
  return true;
}

CyclesGlobalState *Object::deviceState() const
{
  return (CyclesGlobalState *)helium::BaseObject::m_state;
}

// UnknownObject definitions //////////////////////////////////////////////////

UnknownObject::UnknownObject(ANARIDataType type, CyclesGlobalState *s) : Object(type, s)
{
  s->objectCounts.unknown++;
}

UnknownObject::~UnknownObject()
{
  deviceState()->objectCounts.unknown--;
}

bool UnknownObject::isValid() const
{
  return false;
}

}  // namespace cycles

CYCLES_ANARI_TYPEFOR_DEFINITION(cycles::Object *);
