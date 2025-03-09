#pragma once

#include "../include/example_app_base.hpp"

#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"
#include "inexor/vulkan-renderer/rendering/imgui/imgui.hpp"
#include "inexor/vulkan-renderer/rendering/octree/octree_renderer.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::example_app {

class ExampleApp : public ExampleAppBase {
private:
    float m_time_passed{0.0f};
    std::unique_ptr<tools::Camera> m_camera;
    std::unique_ptr<input::KeyboardMouseInputData> m_input_data;

    std::unique_ptr<rendering::octree::OctreeRenderer> m_octree_renderer;
    std::unique_ptr<rendering::imgui::ImGuiRenderer> m_imgui_renderer;
    // Add your renderer here...

public:
    /// Default constructor
    /// @param argc The number of arguments passed to main()
    /// @param argv The arguments passed to main()
    ExampleApp(int argc, char **argv);
    ExampleApp(const ExampleApp &) = delete;
    // TODO: Implement me!
    ExampleApp(ExampleApp &&) noexcept;
    ~ExampleApp() = default;

    ExampleApp &operator=(const ExampleApp &) = delete;
    // TODO: Implement me!
    ExampleApp &operator=(ExampleApp &&) noexcept;

    void cursor_position_callback(GLFWwindow *window, double x_pos, double y_pos) override;

    void keyboard_button_callback(GLFWwindow *window, int key, int scancode, int action, int mods) override;

    void initialize() override;

    void load_toml_configuration_file(const std::string &file_name);

    void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) override;

    void mouse_scroll_callback(GLFWwindow *window, double x_offset, double y_offset) override;

    void run() override;

    void setup_render_graph() override;

    void update_imgui() override;
};

} // namespace inexor::vulkan_renderer::example_app
