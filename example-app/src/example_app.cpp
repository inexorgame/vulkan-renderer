#include "../include/example_app.hpp"

#include "../include/example_app_meta.hpp"

#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <memory>

namespace inexor::vulkan_renderer::example_app {

ExampleApp::ExampleApp(int argc, char *argv[]) {
    parse_command_line_arguments(argc, argv);

    initialize_spdlog();
    spdlog::trace("{}, VERSION: {}.{}.{}", ENGINE_NAME, ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2]);
    spdlog::trace("{}, VERSION: {}.{}.{}, BUILD: {} {}, GIT-SHA: {}", APP_NAME, APP_VERSION[0], APP_VERSION[1],
                  APP_VERSION[2], __DATE__, __TIME__, BUILD_GIT);

    m_window = std::make_unique<Window>(APP_NAME, 1920, 1080, true, true, Window::Mode::WINDOWED);
    setup_window_input_callbacks();

    m_instance = std::make_unique<Instance>(
        VK_MAKE_API_VERSION(0, USED_VULKAN_API_VERSION[0], USED_VULKAN_API_VERSION[1], USED_VULKAN_API_VERSION[2]),
        APP_NAME, ENGINE_NAME, VK_MAKE_API_VERSION(0, APP_VERSION[0], APP_VERSION[1], APP_VERSION[2]),
        VK_MAKE_API_VERSION(0, ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2]),
        validation_layer_debug_messenger_callback);

    m_surface = std::make_unique<Surface>(*m_instance, *m_window);

    create_physical_device();
    setup_render_graph();
}

void ExampleApp::process_mouse_input() {
    // TODO: This here is really "update_camera" code...
    const auto cursor_pos_delta = m_input_data.calculate_cursor_position_delta();

    if (m_camera.type() == CameraType::LOOK_AT && m_input_data.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_camera.rotate(static_cast<float>(cursor_pos_delta[0]), -static_cast<float>(cursor_pos_delta[1]));
    }
    m_camera.set_movement_state(CameraMovement::FORWARD, m_input_data.is_key_pressed(GLFW_KEY_W));
    m_camera.set_movement_state(CameraMovement::LEFT, m_input_data.is_key_pressed(GLFW_KEY_A));
    m_camera.set_movement_state(CameraMovement::BACKWARD, m_input_data.is_key_pressed(GLFW_KEY_S));
    m_camera.set_movement_state(CameraMovement::RIGHT, m_input_data.is_key_pressed(GLFW_KEY_D));
}

void ExampleApp::process_keyboard_input() {}

void ExampleApp::create_physical_device() {
    std::vector<const char *> required_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    const VkPhysicalDeviceFeatures required_features = {};

    const auto selected_gpu =
        Device::pick_best_physical_device(*m_instance, *m_surface, required_features, required_extensions);

    m_device = std::make_unique<Device>(*m_instance, *m_surface, selected_gpu, required_extensions, required_features);
}

void ExampleApp::setup_window_input_callbacks() {
    m_window->set_user_ptr(this);

    m_window->set_resize_callback([](GLFWwindow *window, int width, int height) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->m_wnd_resized = true;
    });
    m_window->set_keyboard_button_callback([](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->keyboard_button_callback(window, key, scancode, action, mods);
    });
    m_window->set_cursor_position_callback([](GLFWwindow *window, double xpos, double ypos) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->cursor_position_callback(window, xpos, ypos);
    });
    m_window->set_mouse_button_callback([](GLFWwindow *window, int button, int action, int mods) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->mouse_button_callback(window, button, action, mods);
    });
    m_window->set_mouse_scroll_callback([](GLFWwindow *window, double xoffset, double yoffset) {
        auto *app = static_cast<ExampleApp *>(glfwGetWindowUserPointer(window));
        app->mouse_scroll_callback(window, xoffset, yoffset);
    });
}

void ExampleApp::initialize_spdlog() {
    spdlog::init_thread_pool(8192, 2);
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string(APP_NAME) + ".log", true);
    auto logger = std::make_shared<spdlog::async_logger>("main", spdlog::sinks_init_list{console_sink, file_sink},
                                                         spdlog::thread_pool(), spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    logger->set_pattern("%Y-%m-%d %T.%f %^%l%$ %5t [%n] %v");
    logger->flush_on(spdlog::level::trace);
    spdlog::set_default_logger(logger);
}

void ExampleApp::parse_command_line_arguments(int argc, char *argv[]) {
    CommandLineArgumentParser cla_parser;
    cla_parser.parse_args(argc, argv);
    m_options.vsync_enabled = cla_parser.arg<bool>("--vsync").value_or(false);
    m_options.stop_on_validation_error = cla_parser.arg<bool>("--stop-on-validation-error").value_or(false);
    m_options.preferred_gpu = cla_parser.arg<std::uint32_t>("--gpu");
}

void ExampleApp::run() {
    while (!m_window->should_close()) {
        m_window->poll();
        process_mouse_input();
        process_keyboard_input();
        // render_frame();
        m_camera.update(m_time_passed);
        check_octree_collisions();
    }
}

void ExampleApp::check_octree_collisions() {
    //
}

void ExampleApp::setup_render_graph() {
    spdlog::trace("Setting up rendergraph");
    m_rendergraph = std::make_shared<RenderGraph>(*m_device);
    m_octree_renderer = std::make_unique<OctreeRenderer>(m_rendergraph);
    // m_imgui_renderer = std::make_unique<ImGuiRenderer>(m_rendergraph);
}

void ExampleApp::update_imgui() {
    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = m_time_passed + 0.00001f;
    auto cursor_pos = m_input_data.get_cursor_pos();
    io.MousePos = ImVec2(static_cast<float>(cursor_pos[0]), static_cast<float>(cursor_pos[1]));
    io.MouseDown[0] = m_input_data.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
    io.MouseDown[1] = m_input_data.is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
    io.DisplaySize =
        ImVec2(static_cast<float>(m_swapchain->extent().width), static_cast<float>(m_swapchain->extent().height));

    ImGui::NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(330, 0));
    ImGui::Begin("Inexor vulkan-renderer", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", m_device->gpu_name().c_str());
    ImGui::Text("Engine version %d.%d.%d (Git sha %s)", ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2],
                BUILD_GIT);
    ImGui::Text("Vulkan API %d.%d.%d", VK_API_VERSION_MAJOR(USED_VULKAN_API_VERSION[0]),
                VK_API_VERSION_MINOR(USED_VULKAN_API_VERSION[1]), VK_API_VERSION_PATCH(USED_VULKAN_API_VERSION[2]));
    const auto &cam_pos = m_camera.position();
    ImGui::Text("Camera position (%.2f, %.2f, %.2f)", cam_pos.x, cam_pos.y, cam_pos.z);
    const auto &cam_rot = m_camera.rotation();
    ImGui::Text("Camera rotation: (%.2f, %.2f, %.2f)", cam_rot.x, cam_rot.y, cam_rot.z);
    const auto &cam_front = m_camera.front();
    ImGui::Text("Camera vector front: (%.2f, %.2f, %.2f)", cam_front.x, cam_front.y, cam_front.z);
    const auto &cam_right = m_camera.right();
    ImGui::Text("Camera vector right: (%.2f, %.2f, %.2f)", cam_right.x, cam_right.y, cam_right.z);
    const auto &cam_up = m_camera.up();
    ImGui::Text("Camera vector up (%.2f, %.2f, %.2f)", cam_up.x, cam_up.y, cam_up.z);
    ImGui::Text("Yaw: %.2f pitch: %.2f roll: %.2f", m_camera.yaw(), m_camera.pitch(), m_camera.roll());
    const auto cam_fov = m_camera.fov();
    ImGui::Text("Field of view: %d", static_cast<std::uint32_t>(cam_fov));
    ImGui::PushItemWidth(150.0f);
    ImGui::PopItemWidth();
    ImGui::PopStyleVar();
    ImGui::End();
    ImGui::EndFrame();
    ImGui::Render();
}

VkBool32 ExampleApp::validation_layer_debug_messenger_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                               const VkDebugUtilsMessageTypeFlagsEXT type,
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
        // TODO: Respect --stop-on-validation-error command line argument!
    }
    return VK_FALSE;
}

void ExampleApp::cursor_position_callback(GLFWwindow *, double, double) {}
void ExampleApp::keyboard_button_callback(GLFWwindow *, int, int, int, int) {}
void ExampleApp::mouse_button_callback(GLFWwindow *, int, int, int) {}
void ExampleApp::mouse_scroll_callback(GLFWwindow *, double, double) {}

} // namespace inexor::vulkan_renderer::example_app

using inexor::vulkan_renderer::example_app::ExampleApp;

int main(int argc, char *argv[]) {
    try {
        std::unique_ptr<ExampleApp> my_renderer = std::make_unique<ExampleApp>(argc, argv);
        my_renderer->run();
    } catch (std::exception &exception) {
        spdlog::critical(exception.what());
    }
    return 0;
}
