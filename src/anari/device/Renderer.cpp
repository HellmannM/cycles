// Copyright 2022 The Khronos Group
// SPDX-License-Identifier: Apache-2.0

#include "Renderer.h"
// cycles
#include "scene/shader_nodes.h"

namespace cycles {

Renderer::Renderer(CyclesGlobalState *s) : Object(ANARI_RENDERER, s)
{
  s->objectCounts.renderers++;
}

Renderer::~Renderer()
{
  deviceState()->objectCounts.renderers--;
}

void Renderer::commit()
{
  m_backgroundColor = getParam<anari_vec::float4>("backgroundColor", {1.f, 1.f, 1.f, 1.f});
}

void Renderer::makeRendererCurrent() const
{
  auto &state = *deviceState();
  auto bgc = m_backgroundColor;

  // Cycles can't seem to deal with a full black bg color on first frame
  for (auto &v : bgc)
    v = v == 0.f ? 1e-3f : v;

  state.background->set_color(ccl::make_float3(bgc[0], bgc[1], bgc[2]));
  state.background->set_strength(1.f);

  state.ambient->set_color(ccl::make_float3(1.f, 1.f, 1.f));
  state.ambient->set_strength(2.f);

  state.scene->default_background->tag_update(state.scene);
}

}  // namespace cycles
