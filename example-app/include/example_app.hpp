#pragma once

#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"
#include "inexor/vulkan-renderer/rendering/imgui/imgui.hpp"
#include "inexor/vulkan-renderer/rendering/octree/octree_renderer.hpp"
#include "inexor/vulkan-renderer/rendering/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/tools/cla_parser.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/surface.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"
#include "inexor/vulkan-renderer/wrapper/window.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::example_app {

// Using declarations
using inexor::vulkan_renderer::input::KeyboardMouseInputData;
using inexor::vulkan_renderer::rendering::render_graph::RenderGraph;
using inexor::vulkan_renderer::tools::CommandLineArgumentParser;
using inexor::vulkan_renderer::wrapper::Device;
using inexor::vulkan_renderer::wrapper::Instance;
using inexor::vulkan_renderer::wrapper::Surface;
using inexor::vulkan_renderer::wrapper::Swapchain;
using inexor::vulkan_renderer::wrapper::Window;

/// The command line arguments will be parsed into these options
struct CommandLineOptions {
    bool stop_on_validation_error{false};
    bool vsync_enabled{true};
    std::optional<std::uint32_t> preferred_gpu{std::nullopt};
    // Add your option here...
};

/// An example app using Inexor vulkan-renderer engine
class ExampleApp {
private:
    CommandLineOptions m_options;
    std::unique_ptr<Instance> m_instance;

    // TODO: Move this to window wrapper
    bool m_wnd_resized{false};
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Surface> m_surface;
    std::unique_ptr<Device> m_device;
    std::unique_ptr<Swapchain> m_swapchain;

    std::shared_ptr<RenderGraph> m_rendergraph;
    std::unique_ptr<rendering::octree::OctreeRenderer> m_octree_renderer;
    std::unique_ptr<rendering::imgui::ImGuiRenderer> m_imgui_renderer;
    // Add your renderer here...

    float m_time_passed{0.0f};
    std::unique_ptr<tools::Camera> m_camera;
    std::unique_ptr<input::KeyboardMouseInputData> m_input_data;

    void initialize_spdlog();

    void recreate_swapchain();

    void setup_window_and_input_callbacks();

    ///
    /// @param
    /// @param
    /// @param
    /// @param
    /// @return
    static VkBool32 validation_layer_debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                                              VkDebugUtilsMessageTypeFlagsEXT,
                                                              const VkDebugUtilsMessengerCallbackDataEXT *,
                                                              void *);

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

    void cursor_position_callback(GLFWwindow *, double, double);
    void keyboard_button_callback(GLFWwindow *, int, int, int, int);
    void mouse_button_callback(GLFWwindow *, int, int, int);
    void mouse_scroll_callback(GLFWwindow *, double, double);
    void parse_command_line_arguments(int argc, char *argv[]);
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
