#pragma once

#include "inexor/vulkan-renderer/input/keyboard_mouse_data.hpp"
#include "inexor/vulkan-renderer/rendering/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/tools/cla_parser.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/surface.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"
#include "inexor/vulkan-renderer/wrapper/window.hpp"

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::example_app {

// Using declarations
using inexor::vulkan_renderer::input::KeyboardMouseInputData;
using inexor::vulkan_renderer::render_graph::RenderGraph;
using inexor::vulkan_renderer::tools::CommandLineArgumentParser;
using inexor::vulkan_renderer::wrapper::Device;
using inexor::vulkan_renderer::wrapper::Instance;
using inexor::vulkan_renderer::wrapper::Surface;
using inexor::vulkan_renderer::wrapper::Swapchain;
using inexor::vulkan_renderer::wrapper::Window;

/// The command line arguments will be parsed into these options
struct CommandLineOptions {
    bool stop_on_validation_error{false};
    bool vsync_enabled{false};
};

/// A base class for example apps which use Inexor vulkan-renderer
class ExampleAppBase {
private:
    // TODO: This should not be part of ExampleAppBase!
    const std::string m_wnd_title = "inexor-vulkan-renderer-example";
    std::uint32_t m_wnd_width{1280};
    std::uint32_t m_wnd_height{720};
    Window::Mode m_wnd_mode{Window::Mode::WINDOWED};
    bool m_wnd_resized{false};

    std::unique_ptr<Window> m_window;
    std::unique_ptr<KeyboardMouseInputData> m_input_data;

    void initialize_spdlog();

    void recreate_swapchain();

    /// Because GLFW is a C-style API, we can't use a pointer to non-static class methods as window or input callback.
    /// A good explanation can be found on Stack Overflow:
    /// https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback
    /// In order to fix this, we can pass a lambda to glfwSetKeyCallback, which calls our callbacks internally. There is
    /// another problem: Inside of the lambda, we need to call the member function. In order to do so, we need to have
    /// access to the this-pointer. Unfortunately, the this-pointer can't be captured in the lambda capture like
    /// [this](){}, because the glfw would not accept the lambda then. To fix this problem, we store the this pointer
    /// using glfwSetWindowUserPointer. Inside of these lambdas, we then cast the pointer to Application* again,
    /// allowing us to finally use the callbacks.
    void setup_window_and_input_callbacks();

protected:
    // TODO: What should be protected here?
    std::unique_ptr<Instance> m_instance;
    std::unique_ptr<Device> m_device;
    // TODO: Why do I need a surface for a device again?
    std::unique_ptr<Surface> m_surface;
    std::unique_ptr<wrapper::Swapchain> m_swapchain;

    CommandLineOptions m_options;

    PFN_vkDebugUtilsMessengerCallbackEXT
    validation_layer_debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT,
                                              VkDebugUtilsMessageTypeFlagsEXT,
                                              const VkDebugUtilsMessengerCallbackDataEXT *,
                                              void *);

    std::shared_ptr<RenderGraph> m_rendergraph;

public:
    ExampleAppBase(int argc, char **argv);
    virtual ~ExampleAppBase();

    ExampleAppBase(const ExampleAppBase &) = delete;
    // TODO: Implement me!
    ExampleAppBase(ExampleAppBase &&) noexcept;

    ExampleAppBase &operator=(const ExampleAppBase &) = delete;
    // TODO: Implement me!
    ExampleAppBase &operator=(ExampleAppBase &&) noexcept;

    virtual void evaluate_command_line_arguments(const CommandLineArgumentParser &parser) = 0;
    virtual void cursor_position_callback(GLFWwindow *, double, double) = 0;
    virtual void keyboard_button_callback(GLFWwindow *, int, int, int, int) = 0;
    virtual void mouse_button_callback(GLFWwindow *, int, int, int) = 0;
    virtual void mouse_scroll_callback(GLFWwindow *, double, double) = 0;
    virtual void initialize() = 0;
    virtual void process_mouse_input() = 0;
    virtual void process_keyboard_input() = 0;
    virtual void render_frame() = 0;
    virtual void run() = 0;
    virtual void setup_device() = 0;
    virtual void setup_render_graph() = 0;
    virtual void shutdown() = 0;
    virtual void update_imgui() = 0;
};

} // namespace inexor::vulkan_renderer::example_app
