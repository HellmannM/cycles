// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Renderer.h"

namespace cycles {

Renderer::Renderer(CyclesGlobalState *s) : Object(ANARI_RENDERER, s)
{
  s->objectCounts.renderers++;
}

Renderer::~Renderer()
{
  deviceState()->objectCounts.renderers--;
}

}  // namespace cycles
