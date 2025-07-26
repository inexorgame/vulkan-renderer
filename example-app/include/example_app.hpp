#pragma once

#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"
#include "inexor/vulkan-renderer/render-components/imgui/imgui_renderer.hpp"
#include "inexor/vulkan-renderer/render-components/octree/colored_triangles_octree_renderer.hpp"
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/tools/cla_parser.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/surface.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"
#include "inexor/vulkan-renderer/wrapper/window.hpp"

#include <spdlog/spdlog.h>

#include <chrono>

namespace inexor::vulkan_renderer::example_app {

// Using declarations
using input::KeyboardMouseInputData;
using render_components::imgui::ImGuiRenderer;
using render_components::octree::ColoredTrianglesOctreeRenderer;
using render_graph::Buffer;
using render_graph::BufferType;
using render_graph::RenderGraph;
using render_graph::Texture;
using render_graph::TextureUsage;
using tools::Camera;
using tools::CameraMovement;
using tools::CameraType;
using tools::CommandLineArgumentParser;
using world::Cube;
using wrapper::Device;
using wrapper::Instance;
using wrapper::Surface;
using wrapper::Swapchain;
using wrapper::Window;
using wrapper::pipelines::PipelineCache;

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
    std::unique_ptr<PipelineCache> m_pipeline_cache;
    std::unique_ptr<Instance> m_instance;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Surface> m_surface;
    std::unique_ptr<Device> m_device;
    std::shared_ptr<Swapchain> m_swapchain;
    std::shared_ptr<RenderGraph> m_rendergraph;
    std::weak_ptr<Texture> m_back_buffer;
    std::weak_ptr<Texture> m_depth_buffer;

    std::vector<std::shared_ptr<Cube>> m_octrees;
    std::unique_ptr<ColoredTrianglesOctreeRenderer> m_octree_renderer;

    std::unique_ptr<ImGuiRenderer> m_imgui_renderer;

    float m_time_passed{0.0f};
    std::shared_ptr<Camera> m_camera;
    KeyboardMouseInputData m_input_data;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_duration{std::chrono::high_resolution_clock::now()};
    std::chrono::time_point<std::chrono::high_resolution_clock> m_last_time{std::chrono::high_resolution_clock::now()};

    void create_physical_device();
    void create_window();
    void initialize_spdlog();
    void setup_octree_rendering();
    void setup_render_graph();
    void recreate_swapchain();
    void render_frame();
    void update_time();

    /// Because GLFW is a C-style API, we can't use a pointer to non-static class methods as window or input callback.
    /// A good explanation can be found on Stack Overflow:
    /// https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
    /// In order to fix this, we can pass a lambda to glfwSetKeyCallback, which calls our callbacks internally. There is
    /// another problem: Inside of the lambda, we need to call the member function. In order to do so, we need to have
    /// access to the this-pointer. Unfortunately, the this-pointer can't be captured in the lambda capture like
    /// [this](){}, because the glfw would not accept the lambda then. To fix this problem, we store the this pointer
    /// using glfwSetWindowUserPointer. Inside of these lambdas, we then cast the pointer to Application* again,
    /// allowing us to finally use the callbacks.
    void setup_window_input_callbacks();

    /// A swap function for move assignment constructor and move assignment operator
    /// @param other A reference to the other instance of ExampleApp to swap with
    void swap(ExampleApp &other);

    /// The callback for validation messages (VK_EXT_debug_utils)
    /// @param severity The severity of the message (info, warning, error)
    /// @param type The type of message (validation, performance)
    /// @param data Additional data related to the message
    /// @param user_data Additional user-defined data related to the message
    /// @return This will always return ``VK_FALSE``, meaning the app will continue to run
    static VkBool32 validation_layer_debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                              VkDebugUtilsMessageTypeFlagsEXT type,
                                                              const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                              void *user_data);

public:
    /// Default constructor
    /// @param argc The number of arguments passed to the main function
    /// @param argv The arguments passed to the main function
    ExampleApp(int argc, char *argv[]);

    void cursor_position_callback(GLFWwindow *, double, double);
    void keyboard_button_callback(GLFWwindow *, int, int, int, int);
    void mouse_button_callback(GLFWwindow *, int, int, int);
    void mouse_scroll_callback(GLFWwindow *, double, double);
    void parse_command_line_arguments(int argc, char *argv[]);
    void process_mouse_input();
    void process_keyboard_input();
    void run();
    void update_imgui();
    void check_octree_collisions();
};

} // namespace inexor::vulkan_renderer::example_app
