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

#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

#include <chrono>

namespace inexor::vulkan_renderer::example_app {

// Using declarations
using input::KeyboardMouseInputData;
using rendering::imgui::ImGuiRenderer;
using rendering::octree::OctreeRenderer;
using rendering::render_graph::Buffer;
using rendering::render_graph::BufferType;
using rendering::render_graph::RenderGraph;
using rendering::render_graph::Texture;
using rendering::render_graph::TextureUsage;
using tools::Camera;
using tools::CameraMovement;
using tools::CameraType;
using tools::CommandLineArgumentParser;
using wrapper::Device;
using wrapper::Instance;
using wrapper::Surface;
using wrapper::Swapchain;
using wrapper::Window;

/// The command line arguments will be parsed into these options
struct CommandLineOptions {
    bool stop_on_validation_error{false};
    bool vsync_enabled{true};
    std::optional<std::uint32_t> preferred_gpu{std::nullopt};
    // Add your option here...
};

/// The model, view, and projection matrix used in a uniform buffer for rendering
struct ModelViewProjMatrix {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

/// An example app using Inexor vulkan-renderer engine
class ExampleApp {
private:
    CommandLineOptions m_options;
    std::unique_ptr<Instance> m_instance;
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Surface> m_surface;
    std::unique_ptr<Device> m_device;
    std::unique_ptr<Swapchain> m_swapchain;

    std::shared_ptr<RenderGraph> m_rendergraph;
    ModelViewProjMatrix matrix{};
    VkDescriptorSetLayout m_desc_set_layout{VK_NULL_HANDLE};
    VkDescriptorSet m_desc_set{VK_NULL_HANDLE};
    std::weak_ptr<Buffer> m_mvp_matrix_buffer;
    std::weak_ptr<Texture> m_back_buffer;
    std::weak_ptr<Texture> m_depth_buffer;

    std::unique_ptr<OctreeRenderer> m_octree_renderer;
    std::unique_ptr<ImGuiRenderer> m_imgui_renderer;

    float m_time_passed{0.0f};
    Camera m_camera;
    KeyboardMouseInputData m_input_data;

    std::chrono::time_point<std::chrono::high_resolution_clock> m_duration{std::chrono::high_resolution_clock::now()};
    std::chrono::time_point<std::chrono::high_resolution_clock> m_last_time{std::chrono::high_resolution_clock::now()};

    void create_physical_device();
    void create_window();
    void initialize_spdlog();
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
    void run();
    void update_imgui();
    void check_octree_collisions();
};

} // namespace inexor::vulkan_renderer::example_app
