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
    ExampleApp(int argc, char *argv[]);
    ExampleApp(const ExampleApp &) = delete;
    // TODO: Implement me!
    ExampleApp(ExampleApp &&) noexcept;
    ~ExampleApp() = default;

    ExampleApp &operator=(const ExampleApp &) = delete;
    // TODO: Implement me!
    ExampleApp &operator=(ExampleApp &&) noexcept;

    void evaluate_command_line_arguments(int argc, char *argv[]);

    void cursor_position_callback(GLFWwindow *, double, double);
    void keyboard_button_callback(GLFWwindow *, int, int, int, int);
    void mouse_button_callback(GLFWwindow *, int, int, int);
    void mouse_scroll_callback(GLFWwindow *, double, double);
    void initialize();
    void process_mouse_input();
    void process_keyboard_input();
    void render_frame();
    void run();
    void setup_device();
    void setup_render_graph();
    void shutdown();
    void update_imgui();
};

} // namespace inexor::vulkan_renderer::example_app
