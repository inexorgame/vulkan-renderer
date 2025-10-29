#include "inexor/vulkan-renderer/application.hpp"

#include "inexor/vulkan-renderer/meta/meta.hpp"
#include "inexor/vulkan-renderer/octree/collision.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/tools/camera.hpp"
#include "inexor/vulkan-renderer/tools/device_info.hpp"
#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"

#include <CLI/CLI.hpp>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include <concepts>
#include <random>
#include <string_view>
#include <thread>
#include <toml++/toml.hpp>

namespace inexor::vulkan_renderer {

void Application::load_toml_configuration_file(const std::string &file_name) {
    spdlog::trace("Loading TOML configuration file: {}", file_name);

    // @TODO Switch to std::filesystem::exists
    std::ifstream toml_file(file_name, std::ios::in);
    if (!toml_file) {
        // If you are using CLion, go to "Edit Configurations" and select "Working Directory".
        throw InexorException("Could not find configuration file: " + file_name +
                              "! You must set the working directory properly in your IDE");
    }

    toml_file.close();

    // Load the TOML file using tomlplusplus.
    auto config_file = toml::parse_file(file_name);

    const std::string_view project_title = config_file["title"].value_or("");
    spdlog::trace("Title: {}", project_title);

    using WindowMode = wrapper::window::Window::Mode;

    const std::string_view wnd_mode = config_file["application"]["window"]["mode"].value_or("windowed");

    if (wnd_mode == "windowed") {
        m_window_mode = WindowMode::WINDOWED;
    } else if (wnd_mode == "windowed_fullscreen") {
        m_window_mode = WindowMode::WINDOWED_FULLSCREEN;
    } else if (wnd_mode == "fullscreen") {
        m_window_mode = WindowMode::FULLSCREEN;
    } else {
        spdlog::warn("Invalid application window mode: {}", wnd_mode);
        m_window_mode = WindowMode::WINDOWED;
    }

    m_window_width = config_file["application"]["window"]["width"].value_or(1280);
    m_window_height = config_file["application"]["window"]["height"].value_or(720);
    m_window_title = config_file["application"]["window"]["name"].value_or("Undefined Window Title!");
    spdlog::trace("Window: {}, {} x {}", m_window_title, m_window_width, m_window_height);

    spdlog::trace("Textures:");
    const auto texture_files = config_file["textures"]["files"].as_array();
    for (const auto &value : *texture_files) {
        const auto texture_file = value.value_or("");
        spdlog::trace("   - {}", texture_file);
        m_texture_files.push_back(texture_file);
    }

    spdlog::trace("glTF 2.0 models:");
    const auto gltf_models = config_file["glTFmodels"]["files"].as_array();
    for (const auto &value : *gltf_models) {
        const std::string gltf_model_file = value.value_or("");
        spdlog::trace("   - {}", gltf_model_file);
        m_gltf_model_files.push_back(gltf_model_file);
    }

    spdlog::trace("Vertex shaders:");
    const auto vertex_shader_files = config_file["shaders"]["vertex"]["files"].as_array();
    for (const auto &value : *vertex_shader_files) {
        const std::string vertex_shader_file = value.value_or("");
        spdlog::trace("   - {}", vertex_shader_file);
        m_vertex_shader_files.push_back(vertex_shader_file);
    }

    spdlog::trace("Fragment shaders:");
    const auto fragment_shader_files = config_file["shaders"]["fragment"]["files"].as_array();
    for (const auto &value : *fragment_shader_files) {
        const std::string fragment_shader_file = value.value_or("");
        spdlog::trace("   - {}", fragment_shader_file);
        m_fragment_shader_files.push_back(fragment_shader_file);
    }
}

void Application::load_shaders() {
    assert(m_device->device());

    spdlog::trace("Loading vertex shaders:");

    if (m_vertex_shader_files.empty()) {
        spdlog::error("No vertex shaders to load!");
    }

    // Loop through the list of vertex shaders and initialise all of them.
    for (const auto &vertex_shader_file : m_vertex_shader_files) {
        spdlog::trace("   - {}", vertex_shader_file);

        // Insert the new shader into the list of shaders.
        m_shaders.emplace_back(*m_device, VK_SHADER_STAGE_VERTEX_BIT, "unnamed vertex shader", vertex_shader_file);
    }

    spdlog::trace("Loading fragment shaders:");

    if (m_fragment_shader_files.empty()) {
        spdlog::error("No fragment shaders to load!");
    }

    // Loop through the list of fragment shaders and initialise all of them.
    for (const auto &fragment_shader_file : m_fragment_shader_files) {
        spdlog::trace("   - {}", fragment_shader_file);

        // Insert the new shader into the list of shaders.
        m_shaders.emplace_back(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "unnamed fragment shader",
                               fragment_shader_file);
    }

    spdlog::trace("Loading shaders finished");
}

VkBool32 Application::validation_layer_debug_messenger_callback(const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                                                const VkDebugUtilsMessageTypeFlagsEXT type,
                                                                const VkDebugUtilsMessengerCallbackDataEXT *data,
                                                                void *user_data) {
    // Use different spdlog methods based on the severity of the message
    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        spdlog::trace("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        spdlog::info("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        spdlog::warn("{}", data->pMessage);
    } else if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        spdlog::critical("{}", data->pMessage);
    }
    return VK_FALSE;
}

void Application::load_octree_geometry(bool initialize) {
    spdlog::trace("Creating octree geometry");

    // 4: 23 012 | 5: 184352 | 6: 1474162 | 7: 11792978 cubes, DO NOT USE 7!
    m_worlds.clear();
    m_worlds.push_back(
        octree::create_random_world(2, {0.0f, 0.0f, 0.0f}, initialize ? std::optional(42) : std::nullopt));
    m_worlds.push_back(
        octree::create_random_world(2, {10.0f, 0.0f, 0.0f}, initialize ? std::optional(60) : std::nullopt));

    auto generate_random_number = []<typename T>(const T min, const T max)
        requires std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>
    {
        // Note that thread_local means that it is implicitely static!
        thread_local std::mt19937 generator(std::random_device{}());
        using U = std::decay_t<T>;
        if constexpr (std::is_integral_v<U>) {
            std::uniform_int_distribution<U> distribution(min, max);
            return distribution(generator);
        } else if constexpr (std::is_floating_point_v<U>) {
            std::uniform_real_distribution<U> distribution(min, max);
            return distribution(generator);
        } else {
            static_assert(std::is_arithmetic_v<U>, "Error: Type must be numeric (integer or float)!");
        }
    };

    m_octree_vertices.clear();
    for (const auto &world : m_worlds) {
        for (const auto &polygons : world->polygons(true)) {
            for (const auto &triangle : *polygons) {
                for (const auto &vertex : triangle) {
                    glm::vec3 color = {
                        generate_random_number(0.0f, 1.0f),
                        generate_random_number(0.0f, 1.0f),
                        generate_random_number(0.0f, 1.0f),
                    };
                    m_octree_vertices.emplace_back(vertex, color);
                }
            }
        }
    }
}

void Application::setup_window_and_input_callbacks() {
    m_window->set_user_ptr(this);

    spdlog::trace("Setting up window callback:");

    auto lambda_frame_buffer_resize_callback = [](GLFWwindow *window, int width, int height) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        spdlog::trace("Frame buffer resize callback called. window width: {}, height: {}", width, height);
        app->m_window_resized = true;
    };

    m_window->set_resize_callback(lambda_frame_buffer_resize_callback);

    spdlog::trace("   - keyboard button callback");

    auto lambda_key_callback = [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->m_input->key_callback(window, key, scancode, action, mods);
    };

    m_window->set_keyboard_button_callback(lambda_key_callback);

    spdlog::trace("   - cursor position callback");

    auto lambda_cursor_position_callback = [](GLFWwindow *window, double xpos, double ypos) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->m_input->cursor_position_callback(window, xpos, ypos);
    };

    m_window->set_cursor_position_callback(lambda_cursor_position_callback);

    spdlog::trace("   - mouse button callback");

    auto lambda_mouse_button_callback = [](GLFWwindow *window, int button, int action, int mods) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->m_input->mouse_button_callback(window, button, action, mods);
    };

    m_window->set_mouse_button_callback(lambda_mouse_button_callback);

    spdlog::trace("   - mouse wheel scroll callback");

    auto lambda_mouse_scroll_callback = [](GLFWwindow *window, double xoffset, double yoffset) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->m_input->mouse_scroll_callback(window, xoffset, yoffset);
    };

    m_window->set_mouse_scroll_callback(lambda_mouse_scroll_callback);
}

void Application::initialize_spdlog() {
    // Initialization of spdlog with only one thread should be fine because at no point do we expect many spdlog
    // messages to be written to the console and the logfile.
    spdlog::init_thread_pool(8192, 1);

    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

    // A copy of the console output will automatically be saved to a logfile
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(std::string(meta::APP_NAME) + ".log", true);
    auto logger = std::make_shared<spdlog::async_logger>("main", spdlog::sinks_init_list{console_sink, file_sink},
                                                         spdlog::thread_pool(), spdlog::async_overflow_policy::block);

    logger->flush_on(spdlog::level::trace);
    logger->set_level(spdlog::level::trace);
    logger->set_pattern("%Y-%m-%d %T.%f %^%l%$ %5t [%n] %v");

    // We only use one global logger by default, not one logger for each component of the code.
    spdlog::set_default_logger(logger);

    spdlog::trace("Inexor vulkan-renderer, BUILD " + std::string(__DATE__) + ", " + __TIME__);
}

Application::Application(int argc, char **argv) {
    initialize_spdlog();

    using namespace vulkan_renderer::meta;

    spdlog::trace("Application version: {}", APP_VERSION_STR);
    spdlog::trace("Engine version: {}", ENGINE_VERSION_STR);

    // Parse command line arguments.
    CLI::App app{"vulkan-renderer"};
    argv = app.ensure_utf8(argv);
    app.add_flag("--vsync", m_vsync_enabled);
    std::optional<std::uint32_t> preferred_gpu;
    app.add_option("--gpu", preferred_gpu);
    std::uint32_t max_fps = tools::FPSLimiter::DEFAULT_FPS;
    app.add_option("--maxfps", max_fps);
    app.parse(argc, argv);

    m_fps_limiter.set_max_fps(max_fps);

    load_toml_configuration_file("configuration/renderer.toml");

    spdlog::trace("Creating Vulkan instance");

    m_window = std::make_unique<Window>(m_window_title, m_window_width, m_window_height, true, true, m_window_mode);

    std::vector<const char *> instance_layers;
    std::vector<const char *> instance_extensions;

    // It is very important to start using Vulkan API by initializing volk with the following function call,
    // otherwise even the most basic Vulkan functions which do not depend on a VkInstance or a VkDevice will not be
    // available!
    spdlog::trace("Initializing volk metaloader");
    if (const auto result = volkInitialize(); result != VK_SUCCESS) {
        throw tools::InexorException("Error: Vulkan initialization with volk metaloader library failed!");
    }

    // If the instance extension "VK_EXT_debug_utils" is available on the system, enable it.
    if (wrapper::is_instance_extension_supported(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
        instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    // Get the instance extensions which are required by glfw library.
    std::uint32_t glfw_extension_count = 0;
    auto *glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
    if (glfw_extension_count == 0) {
        throw tools::InexorException(
            "Error: glfwGetRequiredInstanceExtensions returned 0 required instance extensions!");
    }

    spdlog::trace("Required GLFW instance extensions:");
    for (std::size_t index = 0; index < glfw_extension_count; index++) {
        // We must make sure that each instance extension that is required by glfw is available on the system.
        if (!wrapper::is_instance_extension_supported(glfw_extensions[index])) {
            // If any of the instance extensions that is required by glfw is not available, we will fail.
            throw tools::InexorException("Error: glfw instance extension '" + std::string(glfw_extensions[index]) +
                                         "' is not available on the system!");
        } else {
            spdlog::trace("   - {}", glfw_extensions[index]);
            instance_extensions.push_back(glfw_extensions[index]);
        }
    }

    if (wrapper::is_instance_layer_supported("VK_LAYER_KHRONOS_validation")) {
        instance_layers.push_back("VK_LAYER_KHRONOS_validation");
    } else {
        spdlog::error("Instance layer 'VK_LAYER_KHRONOS_validation' is not available on this system!");
    }

    m_instance = std::make_unique<Instance>(instance_layers, instance_extensions);

    m_dbg_callback =
        std::make_unique<wrapper::VulkanDebugUtilsCallback>(*m_instance, validation_layer_debug_messenger_callback);

    m_input = std::make_unique<Input>();

    m_surface = std::make_unique<WindowSurface>(m_instance->instance(), m_window->window());

    setup_window_and_input_callbacks();

    spdlog::trace("Creating window surface");

    if (preferred_gpu) {
        spdlog::trace("Preferential graphics card index {} specified", *preferred_gpu);
    }

    if (m_vsync_enabled) {
        spdlog::trace("V-sync enabled!");
    } else {
        spdlog::trace("V-sync disabled!");
    }

    const auto physical_devices = tools::get_physical_devices(m_instance->instance());
    if (preferred_gpu && *preferred_gpu >= physical_devices.size()) {
        spdlog::critical("GPU index {} is out of range!", *preferred_gpu);
        // The most suitable gpu will be chosen automatically later.
        preferred_gpu = std::nullopt;
    }

    const VkPhysicalDeviceFeatures required_features{
        // Add required physical device features here
    };

    std::vector<const char *> required_extensions{
        // Since we want to draw on a window, we need the swapchain extension
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    const VkPhysicalDevice physical_device =
        preferred_gpu ? physical_devices[*preferred_gpu]
                      : tools::pick_best_physical_device(*m_instance, m_surface->surface(), required_features,
                                                         required_extensions);

    m_device = std::make_unique<wrapper::Device>(*m_instance, m_surface->surface(), physical_device, required_features,
                                                 required_extensions);

    m_swapchain = std::make_unique<wrapper::Swapchain>(*m_device, m_surface->surface(), m_window->width(),
                                                       m_window->height(), m_vsync_enabled);

    m_camera =
        std::make_unique<tools::Camera>(glm::vec3(6.0f, 10.0f, 2.0f), 180.0f, 0.0f,
                                        static_cast<float>(m_window->width()), static_cast<float>(m_window->height()));
    m_camera->set_movement_speed(5.0f);
    m_camera->set_rotation_speed(0.5f);

    load_shaders();

    m_uniform_buffers.emplace_back(*m_device, "matrices uniform buffer", sizeof(UniformBufferObject));

    // Create an instance of the resource descriptor builder.
    // This allows us to make resource descriptors with the help of a builder pattern.
    wrapper::descriptors::DescriptorBuilder descriptor_builder(*m_device);

    // Make use of the builder to create a resource descriptor for the uniform buffer.
    m_descriptors.emplace_back(
        descriptor_builder.add_uniform_buffer<UniformBufferObject>(m_uniform_buffers[0].buffer(), 0)
            .build("Default uniform buffer"));

    load_octree_geometry(true);
    generate_octree_indices();

    m_window->show();
    recreate_swapchain();
}

void Application::update_uniform_buffers() {
    UniformBufferObject ubo{};

    ubo.model = glm::mat4(1.0f);
    ubo.view = m_camera->view_matrix();
    ubo.proj = m_camera->perspective_matrix();
    ubo.proj[1][1] *= -1;

    // TODO: Embed this into the render graph.
    m_uniform_buffers[0].update(&ubo, sizeof(ubo));
}

void Application::update_imgui_overlay() {
    auto cursor_pos = m_input->kbm_data().get_cursor_pos();

    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = m_time_passed;
    io.MousePos = ImVec2(static_cast<float>(cursor_pos[0]), static_cast<float>(cursor_pos[1]));
    io.MouseDown[0] = m_input->kbm_data().is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT);
    io.MouseDown[1] = m_input->kbm_data().is_mouse_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);
    io.DisplaySize =
        ImVec2(static_cast<float>(m_swapchain->extent().width), static_cast<float>(m_swapchain->extent().height));

    ImGui::NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(330, 0));
    using namespace vulkan_renderer::meta;
    ImGui::Begin(APP_NAME, nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", m_device->gpu_name().c_str());
    ImGui::Text("Engine version %s (git SHA %s)", ENGINE_VERSION_STR, BUILD_GIT);
    ImGui::Text("Vulkan API %d.%d.%d", VK_API_VERSION_MAJOR(VK_API_VERSION_1_2),
                VK_API_VERSION_MINOR(VK_API_VERSION_1_2), VK_API_VERSION_PATCH(VK_API_VERSION_1_2));
    const auto cam_pos = m_camera->position();
    ImGui::Text("Camera position (%.2f, %.2f, %.2f)", cam_pos.x, cam_pos.y, cam_pos.z);
    const auto cam_rot = m_camera->rotation();
    ImGui::Text("Camera rotation: (%.2f, %.2f, %.2f)", cam_rot.x, cam_rot.y, cam_rot.z);
    const auto cam_front = m_camera->front();
    ImGui::Text("Camera vector front: (%.2f, %.2f, %.2f)", cam_front.x, cam_front.y, cam_front.z);
    const auto cam_right = m_camera->right();
    ImGui::Text("Camera vector right: (%.2f, %.2f, %.2f)", cam_right.x, cam_right.y, cam_right.z);
    const auto cam_up = m_camera->up();
    ImGui::Text("Camera vector up (%.2f, %.2f, %.2f)", cam_up.x, cam_up.y, cam_up.z);
    ImGui::Text("Yaw: %.2f pitch: %.2f roll: %.2f", m_camera->yaw(), m_camera->pitch(), m_camera->roll());
    const auto cam_fov = m_camera->fov();
    ImGui::Text("Field of view: %d", static_cast<std::uint32_t>(cam_fov));
    ImGui::PushItemWidth(150.0f * m_imgui_overlay->scale());
    ImGui::PopItemWidth();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::Render();

    m_imgui_overlay->update();
}

void Application::process_input() {
    const auto cursor_pos_delta = m_input->kbm_data().calculate_cursor_position_delta();

    auto deadzone_lambda = [](const float state) { return (glm::abs(state) < 0.2f) ? 0.0f : state; };

    using namespace tools;

    if (m_camera->type() == CameraType::LOOK_AT &&
        m_input->kbm_data().is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_camera->rotate(static_cast<float>(cursor_pos_delta[0]), -static_cast<float>(cursor_pos_delta[1]));
    }
    if (m_camera->type() == CameraType::LOOK_AT) {
        m_camera->rotate(deadzone_lambda(m_input->gamepad_data().current_joystick_axes(1).x) * 5.f,
                         deadzone_lambda(m_input->gamepad_data().current_joystick_axes(1).y) * -5.f);
    }

    m_camera->set_movement_state(CameraMovement::FORWARD,
                                 m_input->gamepad_data().current_joystick_axes(0)[GLFW_GAMEPAD_AXIS_LEFT_Y] <= -0.15);
    m_camera->set_movement_state(CameraMovement::LEFT,
                                 m_input->gamepad_data().current_joystick_axes(0)[GLFW_GAMEPAD_AXIS_LEFT_X] <= -0.15);
    m_camera->set_movement_state(CameraMovement::BACKWARD,
                                 m_input->gamepad_data().current_joystick_axes(0)[GLFW_GAMEPAD_AXIS_LEFT_Y] >= 0.15);
    m_camera->set_movement_state(CameraMovement::RIGHT,
                                 m_input->gamepad_data().current_joystick_axes(0)[GLFW_GAMEPAD_AXIS_LEFT_X] >= 0.15);
    m_camera->update(m_time_passed);
    m_camera->set_movement_state(CameraMovement::FORWARD, m_input->kbm_data().is_key_pressed(GLFW_KEY_W));
    m_camera->set_movement_state(CameraMovement::LEFT, m_input->kbm_data().is_key_pressed(GLFW_KEY_A));
    m_camera->set_movement_state(CameraMovement::BACKWARD, m_input->kbm_data().is_key_pressed(GLFW_KEY_S));
    m_camera->set_movement_state(CameraMovement::RIGHT, m_input->kbm_data().is_key_pressed(GLFW_KEY_D));
}

void Application::check_octree_collisions() {
    // Check for collision between camera ray and every octree
    for (const auto &world : m_worlds) {
        const auto collision = ray_cube_collision_check(*world, m_camera->position(), m_camera->front());

        if (collision) {
            const auto intersection = collision.value().intersection();
            const auto face_normal = collision.value().face();
            const auto corner = collision.value().corner();
            const auto edge = collision.value().edge();

            spdlog::trace("pos {} {} {} | face {} {} {} | corner {} {} {} | edge {} {} {}", intersection.x,
                          intersection.y, intersection.z, face_normal.x, face_normal.y, face_normal.z, corner.x,
                          corner.y, corner.z, edge.x, edge.y, edge.z);

            // Break after one collision.
            break;
        }
    }
}

void Application::run() {
    spdlog::trace("Running Application");

    while (!m_window->should_close()) {
        m_window->poll();
        if (m_fps_limiter.is_next_frame_allowed()) {
            m_input->update_gamepad_data();
            update_uniform_buffers();
            update_imgui_overlay();
            render_frame();
            process_input();
            if (m_input->kbm_data().was_key_pressed_once(GLFW_KEY_N)) {
                load_octree_geometry(false);
                generate_octree_indices();
                m_index_buffer->upload_data(m_octree_indices);
                m_vertex_buffer->upload_data(m_octree_vertices);
            }
            m_camera->update(m_time_passed);
            m_time_passed = m_stopwatch.time_step();
            check_octree_collisions();
        }
    }
}

} // namespace inexor::vulkan_renderer
