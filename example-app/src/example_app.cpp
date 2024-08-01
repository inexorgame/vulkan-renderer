#include "../include/example_app.hpp"

#include <toml.hpp>

#include <memory>

namespace inexor::example_app {

ExampleApp::ExampleApp(int argc, char **argv) : ExampleAppBase(argc, argv) {}

ExampleApp::~ExampleApp() {}

void ExampleApp::initialize() {
    //
}

void ExampleApp::setup_render_graph() {
    spdlog::trace("Setting up rendergraph");
    m_octree_renderer = std::make_unique<OctreeRenderer>(m_rendergraph);
    m_imgui_renderer = std::make_unique<ImGuiRenderer>(m_rendergraph);
}

void ExampleApp::update_imgui() {
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = m_time_passed + 0.00001f;
    auto cursor_pos = m_input_data->get_cursor_pos();
    io.MousePos = ImVec2(static_cast<float>(cursor_pos[0]), static_cast<float>(cursor_pos[1]));
    io.MouseDown[0] = m_input_data->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
    io.MouseDown[1] = m_input_data->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
    io.DisplaySize =
        ImVec2(static_cast<float>(m_swapchain->extent().width), static_cast<float>(m_swapchain->extent().height));

    ImGui::NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(330, 0));
    ImGui::Begin("Inexor Vulkan-renderer", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", m_device->gpu_name().c_str());
    ImGui::Text("Engine version %d.%d.%d (Git sha %s)", ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2],
                BUILD_GIT);
    ImGui::Text("Vulkan API %d.%d.%d", VK_API_VERSION_MAJOR(wrapper::Instance::REQUIRED_VK_API_VERSION),
                VK_API_VERSION_MINOR(wrapper::Instance::REQUIRED_VK_API_VERSION),
                VK_API_VERSION_PATCH(wrapper::Instance::REQUIRED_VK_API_VERSION));
    const auto &cam_pos = m_camera->position();
    ImGui::Text("Camera position (%.2f, %.2f, %.2f)", cam_pos.x, cam_pos.y, cam_pos.z);
    const auto &cam_rot = m_camera->rotation();
    ImGui::Text("Camera rotation: (%.2f, %.2f, %.2f)", cam_rot.x, cam_rot.y, cam_rot.z);
    const auto &cam_front = m_camera->front();
    ImGui::Text("Camera vector front: (%.2f, %.2f, %.2f)", cam_front.x, cam_front.y, cam_front.z);
    const auto &cam_right = m_camera->right();
    ImGui::Text("Camera vector right: (%.2f, %.2f, %.2f)", cam_right.x, cam_right.y, cam_right.z);
    const auto &cam_up = m_camera->up();
    ImGui::Text("Camera vector up (%.2f, %.2f, %.2f)", cam_up.x, cam_up.y, cam_up.z);
    ImGui::Text("Yaw: %.2f pitch: %.2f roll: %.2f", m_camera->yaw(), m_camera->pitch(), m_camera->roll());
    const auto cam_fov = m_camera->fov();
    ImGui::Text("Field of view: %d", static_cast<std::uint32_t>(cam_fov));
    ImGui::PushItemWidth(150.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
}

} // namespace inexor::example_app
