#include "../include/example_app_base.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace inexor::example_app {

ExampleAppBase::ExampleAppBase(int argc, char **argv) {
    // Setup spdlog logging library
    initialize_spdlog();

    // Parse and evaluate command line arguments
    CommandLineArgumentParser cla_parser;
    cla_parser.parse_args(argc, argv);
    evaluate_command_line_arguments(cla_parser);

    spdlog::trace("Application version: {}.{}.{}", APP_VERSION[0], APP_VERSION[1], APP_VERSION[2]);
    spdlog::trace("Engine version: {}.{}.{}", ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2]);

    // NOTE: We must create the window before the VkInstance, otherwise glfwGetRequiredInstanceExtensions fails!
    m_window = std::make_unique<Window>(m_wnd_title, m_wnd_width, m_wnd_height, true, true, m_wnd_mode);
    setup_window_and_input_callbacks();

    m_instance = std::make_unique<Instance>(
        APP_NAME, ENGINE_NAME, VK_MAKE_API_VERSION(0, APP_VERSION[0], APP_VERSION[1], APP_VERSION[2]),
        VK_MAKE_API_VERSION(0, ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2]),
        validation_layer_debug_messenger_callback);

    m_surface = std::make_unique<Surface>(m_instance->instance(), m_window->window());
    m_input_data = std::make_unique<KeyboardMouseInputData>();

    m_device = std::make_unique<Device>(*m_instance, m_surface->surface(), physical_device, required_extensions,
                                        required_features, optional_extensions, optional_features);

    m_swapchain = std::make_shared<Swapchain>(*m_device, "Default Swapchain", m_surface->surface(), *m_window,
                                              m_options.vsync_enabled);
}

ExampleAppBase::~ExampleAppBase() {
    spdlog::trace("Shutting down vulkan-renderer-example");
}

void ExampleAppBase::initialize_spdlog() {
    spdlog::init_thread_pool(8192, 2);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("vulkan-renderer-example.log", true);
    auto logger = std::make_shared<spdlog::async_logger>("vulkan-renderer-example",
                                                         spdlog::sinks_init_list{console_sink, file_sink},
                                                         spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    logger->set_pattern("%Y-%m-%d %T.%f %^%l%$ %5t [%-10n] %v");
    logger->flush_on(spdlog::level::trace);
    spdlog::set_default_logger(logger);
    spdlog::trace("Inexor vulkan-renderer-example, BUILD " + std::string(__DATE__) + ", " + __TIME__);
}

void ExampleAppBase::setup_window_and_input_callbacks() {
    m_window->set_user_ptr(this);

    m_window->set_resize_callback([](GLFWwindow *window, int width, int height) {
        auto *app = static_cast<ExampleAppBase *>(glfwGetWindowUserPointer(window));
        app->m_wnd_resized = true;
    });

    m_window->set_keyboard_button_callback([](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto *app = static_cast<ExampleAppBase *>(glfwGetWindowUserPointer(window));
        app->keyboard_button_callback(window, key, scancode, action, mods);
    });

    m_window->set_cursor_position_callback([](GLFWwindow *window, double xpos, double ypos) {
        auto *app = static_cast<ExampleAppBase *>(glfwGetWindowUserPointer(window));
        app->cursor_position_callback(window, xpos, ypos);
    });

    m_window->set_mouse_button_callback([](GLFWwindow *window, int button, int action, int mods) {
        auto *app = static_cast<ExampleAppBase *>(glfwGetWindowUserPointer(window));
        app->mouse_button_callback(window, button, action, mods);
    });

    m_window->set_mouse_scroll_callback([](GLFWwindow *window, double xoffset, double yoffset) {
        auto *app = static_cast<ExampleAppBase *>(glfwGetWindowUserPointer(window));
        app->mouse_scroll_callback(window, xoffset, yoffset);
    });
}

void ExampleAppBase::recreate_swapchain() {
    // TODO: Implement!
}

VkBool32 VKAPI_CALL
ExampleAppBase::validation_layer_debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                          VkDebugUtilsMessageTypeFlagsEXT type,
                                                          const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                          void *user_data) {
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        spdlog::trace("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        spdlog::info("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::critical("{}", data->pMessage);
    }
    if (m_options.stop_on_validation_error) {
        throw std::runtime_error("[ExampleApp::validation_layer_debug_messenger_callback] Validation error, stopped.");
    }
    return false;
}

} // namespace inexor::example_app
