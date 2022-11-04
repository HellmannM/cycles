// Copyright 2022 Jefferson Amstutz
// SPDX-License-Identifier: Apache-2.0

#include "match3D/match3D.h"
// NOTE: Also can include stb_image because it was passed as a CMake package
//       component. If those were omitted in CMake, then only match3D/match3D.h
//       can be included.
#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"
// cycles
#include "device/device.h"
#include "scene/background.h"
#include "scene/camera.h"
#include "scene/integrator.h"
#include "scene/mesh.h"
#include "scene/object.h"
#include "scene/scene.h"
#include "scene/shader.h"
#include "scene/shader_graph.h"
#include "scene/shader_nodes.h"
#include "session/buffers.h"
#include "session/session.h"
// tiny_obj_loader
#include "tiny_obj_loader.h"

#include "Orbit.h"
#include "buffer_output_driver.h"

struct OBJData
{
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
};

class ExampleApp : public match3D::SimpleApplication
{
 public:
  ExampleApp(std::string filename, std::string deviceType = "CPU");
  ~ExampleApp() override = default;

  void setup() override;
  void buildUI() override;
  void drawBackground() override;
  void teardown() override;

 private:
  void handleInput();
  void updateCamera();
  void reshape(int width, int height);
  void restartRender();
  void loadOBJ();
  ccl::Mesh *addMesh();
  void addMaterialShader(
      const std::string &basePath, const tinyobj::material_t &mat);
  void makeBGShader();

  // Data //

  ccl::float2 m_previousMouse;
  bool m_mouseRotating{false};
  bool m_manipulating{false};
  ccl::Orbit m_arcball;

  GLuint m_framebufferTexture{0};
  GLuint m_framebufferObject{0};
  int m_width{1200};
  int m_height{800};
  int m_renderWidth{0};
  int m_renderHeight{0};

  int m_samples{1};
  int m_maxSamples{64};

  std::string m_filename;
  std::string m_deviceType;

  std::unique_ptr<ccl::Session> m_session;
  ccl::Scene *m_scene{nullptr};
  ccl::SceneParams m_scene_params;
  ccl::SessionParams m_session_params;
  ccl::BufferParams m_buffer_params;
  ccl::BufferOutputDriver *m_output_driver{nullptr};

  // Nodes
  struct
  {
    ccl::BackgroundNode *bg{nullptr};
  } m_nodes;
};

// Helper functions ///////////////////////////////////////////////////////////

static std::string pathOf(const std::string &filename)
{
#ifdef _WIN32
  const char path_sep = '\\';
#else
  const char path_sep = '/';
#endif

  size_t pos = filename.find_last_of(path_sep);
  if (pos == std::string::npos)
    return "";
  return filename.substr(0, pos + 1);
}

static ccl::Transform camera_materix(
    const ccl::float3 &eye, const ccl::float3 &dir, const ccl::float3 &up)
{
  ccl::Transform retval;
  const auto s = ccl::normalize(ccl::cross(dir, up));
  const auto u = ccl::cross(s, dir);
  retval.x[0] = s.x;
  retval.x[1] = u.x;
  retval.x[2] = dir.x;
  retval.y[0] = s.y;
  retval.y[1] = u.y;
  retval.y[2] = dir.y;
  retval.z[0] = s.z;
  retval.z[1] = u.z;
  retval.z[2] = dir.z;
  retval.x[3] = eye.x;
  retval.y[3] = eye.y;
  retval.z[3] = eye.z;
  return retval;
}

// ExampleApp definitions /////////////////////////////////////////////////////

ExampleApp::ExampleApp(std::string filename, std::string deviceType)
    : m_filename(filename), m_deviceType(deviceType)
{
  m_previousMouse = ccl::make_float2(-1, -1);
  m_arcball = ccl::Orbit(ccl::make_float3(0, 0, 0), 8.f);
}

void ExampleApp::setup()
{
  // ImGui //

  ImGuiIO &io = ImGui::GetIO();
  io.FontGlobalScale = 1.5f;
  io.IniFilename = nullptr;

  // OpenGL //

  // Create a texture
  glGenTextures(1, &m_framebufferTexture);
  glBindTexture(GL_TEXTURE_2D, m_framebufferTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D,
      0,
      GL_RGBA8,
      m_width,
      m_height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      0);

  // Map framebufferTexture (above) to a new OpenGL read/write framebuffer
  glGenFramebuffers(1, &m_framebufferObject);
  glBindFramebuffer(GL_FRAMEBUFFER, m_framebufferObject);
  glFramebufferTexture2D(GL_FRAMEBUFFER,
      GL_COLOR_ATTACHMENT0,
      GL_TEXTURE_2D,
      m_framebufferTexture,
      0);
  glReadBuffer(GL_COLOR_ATTACHMENT0);

  // Cycles //

  m_session_params.device.type =
      m_deviceType == "CPU" ? ccl::DEVICE_CPU : ccl::DEVICE_OPTIX;
  m_session_params.background = false;
  m_session_params.headless = false;
  m_session_params.use_auto_tile = false;
  m_session_params.tile_size = 64; // 2048
  m_session_params.use_resolution_divider = false;
  m_session_params.samples = 1;

  m_session = std::make_unique<ccl::Session>(m_session_params, m_scene_params);
  m_scene = m_session->scene;

  auto output_driver = std::make_unique<ccl::BufferOutputDriver>("combined");
  m_output_driver = output_driver.get();

  m_session->set_output_driver(std::move(output_driver));

  loadOBJ();

  ccl::Pass *pass = m_scene->create_node<ccl::Pass>();
  pass->set_name(OIIO::ustring("combined"));
  pass->set_type(ccl::PASS_COMBINED);

  reshape(m_width, m_height);
  updateCamera();
}

void ExampleApp::buildUI()
{
  if (getWindowSize(m_width, m_height))
    reshape(m_width, m_height);

  handleInput();

  {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoDecoration
        | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings
        | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav
        | ImGuiWindowFlags_NoMove;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);

    ImGui::Begin("Debug Info", nullptr, windowFlags);

    ImGui::Text("display rate: %.1f FPS", 1.f / getLastFrameLatency());
    ImGui::Text("samples: %i", m_samples);
    ImGui::InputInt("maxSamples", &m_maxSamples);
    ImGui::NewLine();

    if (ImGui::Button("restart"))
      restartRender();

    {
      auto color = m_nodes.bg->get_color();
      if (ImGui::ColorEdit3("bgColor", &color.x)) {
        m_nodes.bg->set_color(color);
        m_scene->default_background->tag_update(m_scene);
        restartRender();
      }
    }

    {
      float strength = m_nodes.bg->get_strength();
      if (ImGui::SliderFloat("bgStrength", &strength, 0.f, 10.f)) {
        m_nodes.bg->set_strength(strength);
        m_scene->default_background->tag_update(m_scene);
        restartRender();
      }
    }

    ImGui::End();
  }
}

void ExampleApp::drawBackground()
{
  int &w = m_renderWidth;
  int &h = m_renderHeight;

  if (m_output_driver->newFrameAvailable()) {
    const auto *pixels = m_output_driver->map(w, h);
    if (pixels) {
      glBindTexture(GL_TEXTURE_2D, m_framebufferTexture);
      glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, pixels);
    }

    m_output_driver->unmap();

    if (m_samples < m_maxSamples || m_maxSamples == 0) {
      m_session->set_samples(++m_samples);
      m_session->start();
    }
  }

  if (w == 0 || h == 0)
    return;

  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferObject);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  glClear(GL_COLOR_BUFFER_BIT);
  glBlitFramebuffer(0, 0, w, h, 0, 0, w, h, GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void ExampleApp::teardown()
{
  m_session->cancel(true);
  m_session->wait();
}

void ExampleApp::handleInput()
{
  ImGuiIO &io = ImGui::GetIO();

  if (io.WantCaptureMouse)
    return;

  const bool leftDown = ImGui::IsMouseDown(ImGuiMouseButton_Left);
  const bool rightDown = ImGui::IsMouseDown(ImGuiMouseButton_Right);
  const bool middleDown = ImGui::IsMouseDown(ImGuiMouseButton_Middle);

  const bool anyDown = leftDown || rightDown || middleDown;

  if (!anyDown) {
    m_manipulating = false;
    m_previousMouse = ccl::make_float2(-1, -1);
  } else if (!m_manipulating)
    m_manipulating = true;

  if (m_mouseRotating && !leftDown)
    m_mouseRotating = false;

  if (m_manipulating) {
    ccl::float2 position;
    std::memcpy(&position, &io.MousePos, sizeof(position));

    const ccl::float2 mouse = ccl::make_float2(position.x, position.y);

    if (anyDown && m_previousMouse != ccl::make_float2(-1, -1)) {
      const ccl::float2 prev = m_previousMouse;

      const ccl::float2 mouseFrom =
          prev * 2.f / ccl::make_float2(m_width, m_height);
      const ccl::float2 mouseTo =
          mouse * 2.f / ccl::make_float2(m_width, m_height);

      const ccl::float2 mouseDelta = mouseFrom - mouseTo;

      if (mouseDelta != ccl::make_float2(0.f, 0.f)) {
        if (leftDown) {
          if (!m_mouseRotating) {
            m_arcball.startNewRotation();
            m_mouseRotating = true;
          }

          m_arcball.rotate(mouseDelta);
        } else if (rightDown)
          m_arcball.zoom(mouseDelta.y);
        else if (middleDown)
          m_arcball.pan(mouseDelta);

        updateCamera();
      }
    }

    m_previousMouse = mouse;
  }
}

void ExampleApp::updateCamera()
{
  auto eye = m_arcball.eye();
  auto dir = m_arcball.dir();
  auto up = m_arcball.up();
  m_session->scene->camera->set_matrix(camera_materix(eye, dir, up));
  m_session->scene->camera->need_flags_update = true;
  m_session->scene->camera->need_device_update = true;
  restartRender();
}

void ExampleApp::reshape(int width, int height)
{
  glViewport(0, 0, width, height);

  glBindTexture(GL_TEXTURE_2D, m_framebufferTexture);
  glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, 0);

  m_buffer_params.width = width;
  m_buffer_params.height = height;
  m_buffer_params.full_width = width;
  m_buffer_params.full_height = height;

  m_scene->camera->set_full_width(width);
  m_scene->camera->set_full_height(height);
  m_scene->camera->compute_auto_viewplane();
  m_scene->camera->need_flags_update = true;
  m_scene->camera->need_device_update = true;

  restartRender();
}

void ExampleApp::restartRender()
{
  m_session->reset(m_session_params, m_buffer_params);
  m_samples = 1;
  m_session->set_samples(m_samples);
  m_session->start();
}

void ExampleApp::loadOBJ()
{
  OBJData objdata;
  std::string warn;
  std::string err;
  std::string basePath = pathOf(m_filename);

  auto retval = tinyobj::LoadObj(&objdata.attrib,
      &objdata.shapes,
      &objdata.materials,
      &warn,
      &err,
      m_filename.c_str(),
      basePath.c_str(),
      true);

  if (!retval)
    throw std::runtime_error("failed to open/parse obj file!");

  size_t numDefaultShaders = m_scene->shaders.size();

  for (auto &mat : objdata.materials)
    addMaterialShader(basePath, mat);

  makeBGShader();

  auto *vertices = objdata.attrib.vertices.data();
  auto *texcoords = objdata.attrib.texcoords.data();

  for (auto &shape : objdata.shapes) {
    auto *mesh = addMesh();

    auto shaderID = shape.mesh.material_ids[0] + numDefaultShaders;

    ccl::array<ccl::Node *> used_shaders;
    used_shaders.push_back_slow(m_scene->shaders[shaderID] /*shader*/);
    mesh->set_used_shaders(used_shaders);

    ccl::vector<ccl::float3> P;
    ccl::vector<int> verts;

    const size_t numIndices = shape.mesh.indices.size();
    const size_t numTriangles = numIndices / 3;
    P.reserve(numIndices);
    mesh->reserve_mesh(numIndices, numTriangles);

    for (size_t i = 0; i < numIndices; i += 3) {
      const auto i0 = shape.mesh.indices[i + 0].vertex_index;
      const auto i1 = shape.mesh.indices[i + 1].vertex_index;
      const auto i2 = shape.mesh.indices[i + 2].vertex_index;

      const auto *v0 = vertices + (i0 * 3);
      const auto *v1 = vertices + (i1 * 3);
      const auto *v2 = vertices + (i2 * 3);
      P.push_back(ccl::make_float3(v0[0], v0[1], v0[2]));
      P.push_back(ccl::make_float3(v1[0], v1[1], v1[2]));
      P.push_back(ccl::make_float3(v2[0], v2[1], v2[2]));

      mesh->add_triangle(i + 0, i + 1, i + 2, 0 /* local shaderID */, true);
    }

    ccl::array<ccl::float3> P_array;
    P_array = P;
    mesh->set_verts(P_array);

    ccl::float2 *uv = nullptr;
    if (texcoords) {
      auto *attr =
          mesh->attributes.add(ccl::ATTR_STD_UV, OIIO::ustring("UVMap"));
      uv = attr->data_float2();
    }

    for (size_t i = 0; i < numIndices; i += 3) {
      if (uv) {
        const auto ti0 = shape.mesh.indices[i + 0].texcoord_index;
        const auto ti1 = shape.mesh.indices[i + 1].texcoord_index;
        const auto ti2 = shape.mesh.indices[i + 2].texcoord_index;
        const auto *t0 = texcoords + (ti0 * 2);
        const auto *t1 = texcoords + (ti1 * 2);
        const auto *t2 = texcoords + (ti2 * 2);
        uv[i + 0] =
            ti0 >= 0 ? ccl::make_float2(t0[0], t0[1]) : ccl::make_float2(0, 0);
        uv[i + 1] =
            ti1 >= 0 ? ccl::make_float2(t1[0], t1[1]) : ccl::make_float2(0, 0);
        uv[i + 2] =
            ti2 >= 0 ? ccl::make_float2(t2[0], t2[1]) : ccl::make_float2(0, 0);
      }
    }
  }
}

ccl::Mesh *ExampleApp::addMesh()
{
  auto *mesh = new ccl::Mesh();
  m_scene->geometry.push_back(mesh);

  auto *object = new ccl::Object();
  object->set_geometry(mesh);
  object->set_tfm(ccl::transform_identity());
  m_scene->objects.push_back(object);

  return mesh;
}

void ExampleApp::addMaterialShader(
    const std::string &basePath, const tinyobj::material_t &mat)
{
  auto *shader = new ccl::Shader();
  m_scene->shaders.push_back(shader);
  auto *graph = new ccl::ShaderGraph();

  auto *bsdf = new ccl::PrincipledBsdfNode();

  bsdf->set_owner(graph);
  bsdf->input("Base Color")->set(*((ccl::float3 *)mat.diffuse));
  bsdf->input("Roughness")->set(1.f);
  bsdf->input("Clearcoat Roughness")->set(1.f);
  bsdf->input("Specular")->set(0.f);
  bsdf->input("Alpha")->set(mat.dissolve);
  graph->add(bsdf);

  if (!mat.diffuse_texname.empty()) {
    std::string filename = basePath + mat.diffuse_texname;
    std::transform(
        filename.begin(), filename.end(), filename.begin(), [](char c) {
          return c == '\\' ? '/' : c;
        });

    auto *tex = new ccl::ImageTextureNode();
    tex->set_filename(OIIO::ustring(filename));
    tex->set_owner(graph);
    graph->add(tex);

    ccl::ShaderOutput *output = tex->output("Color");
    ccl::ShaderInput *input = bsdf->input("Base Color");

    if (output && input)
      graph->connect(output, input);
  }

  {
    ccl::ShaderOutput *output = bsdf->output("BSDF");
    ccl::ShaderInput *input = graph->output()->input("Surface");

    if (output && input)
      graph->connect(output, input);
  }

  shader->set_graph(graph);
  shader->tag_update(m_scene);
}

void ExampleApp::makeBGShader()
{
  auto *shader = m_scene->default_background;
  auto *graph = new ccl::ShaderGraph();

  auto *bg = new ccl::BackgroundNode();
  bg->set_owner(graph);
  bg->input("Color")->set(ccl::make_float3(1.f, 1.f, 1.f));
  bg->input("Strength")->set(1.f);
  graph->add(bg);

  m_nodes.bg = bg;

  ccl::ShaderOutput *output = bg->output("Background");
  ccl::ShaderInput *input = graph->output()->input("Surface");

  if (output && input)
    graph->connect(output, input);

  shader->set_graph(graph);
  shader->tag_update(m_scene);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

int main(int argc, const char *argv[])
{
  if (argc < 2) {
    printf("usage: ./cycles_viewer obj_file");
    return 1;
  }

  auto devices = ccl::Device::available_devices();
  printf("Devices:\n");

  for (ccl::DeviceInfo &info : devices) {
    printf("    %-10s%s%s\n",
        ccl::Device::string_from_type(info.type).c_str(),
        info.description.c_str(),
        (info.display_device) ? " (display)" : "");
  }

  ExampleApp app(argv[1], "OPTIX");
  app.run(1200, 800, "Example match3D application");
}
