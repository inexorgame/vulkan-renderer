#include "inexor/vulkan-renderer/application.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/meta.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/tools/cla_parser.hpp"
#include "inexor/vulkan-renderer/world/collision.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <toml11/toml.hpp>

#include <random>
#include <thread>

namespace inexor::vulkan_renderer {

void Application::key_callback(GLFWwindow * /*window*/, int key, int, int action, int /*mods*/) {
    switch (action) {
    case GLFW_PRESS:
        m_input_data->press_key(key);
        break;
    case GLFW_RELEASE:
        m_input_data->release_key(key);
        break;
    default:
        break;
    }
}

void Application::cursor_position_callback(GLFWwindow * /*window*/, double x_pos, double y_pos) {
    m_input_data->set_cursor_pos(x_pos, y_pos);
}

void Application::mouse_button_callback(GLFWwindow * /*window*/, int button, int action, int /*mods*/) {
    switch (action) {
    case GLFW_PRESS:
        m_input_data->press_mouse_button(button);
        break;
    case GLFW_RELEASE:
        m_input_data->release_mouse_button(button);
        break;
    default:
        break;
    }
}

void Application::mouse_scroll_callback(GLFWwindow * /*window*/, double /*x_offset*/, double y_offset) {
    m_camera->change_zoom(static_cast<float>(y_offset));
}

void Application::load_toml_configuration_file(const std::string &file_name) {
    spdlog::debug("Loading TOML configuration file: '{}'", file_name);

    std::ifstream toml_file(file_name, std::ios::in);
    if (!toml_file) {
        throw std::runtime_error("Could not open configuration file: " + file_name + "!");
    }

    toml_file.close();

    // Load the TOML file using toml11.
    auto renderer_configuration = toml::parse(file_name);

    // Search for the title of the configuration file and print it to debug output.
    const auto &configuration_title = toml::find<std::string>(renderer_configuration, "title");
    spdlog::debug("Title: '{}'", configuration_title);

    using WindowMode = ::inexor::vulkan_renderer::wrapper::Window::Mode;
    const auto &wmodestr = toml::find<std::string>(renderer_configuration, "application", "window", "mode");
    if (wmodestr == "windowed") {
        m_window_mode = WindowMode::WINDOWED;
    } else if (wmodestr == "windowed_fullscreen") {
        m_window_mode = WindowMode::WINDOWED_FULLSCREEN;
    } else if (wmodestr == "fullscreen") {
        m_window_mode = WindowMode::FULLSCREEN;
    } else {
        spdlog::warn("Invalid application window mode: {}", wmodestr);
        m_window_mode = WindowMode::WINDOWED;
    }

    m_window_width = toml::find<int>(renderer_configuration, "application", "window", "width");
    m_window_height = toml::find<int>(renderer_configuration, "application", "window", "height");
    m_window_title = toml::find<std::string>(renderer_configuration, "application", "window", "name");
    spdlog::debug("Window: '{}', {} x {}", m_window_title, m_window_width, m_window_height);

    m_texture_files = toml::find<std::vector<std::string>>(renderer_configuration, "textures", "files");

    spdlog::debug("Textures:");

    for (const auto &texture_file : m_texture_files) {
        spdlog::debug("{}", texture_file);
    }

    m_gltf_model_files = toml::find<std::vector<std::string>>(renderer_configuration, "glTFmodels", "files");

    spdlog::debug("glTF 2.0 models:");

    for (const auto &gltf_model_file : m_gltf_model_files) {
        spdlog::debug("{}", gltf_model_file);
    }

    m_vertex_shader_files = toml::find<std::vector<std::string>>(renderer_configuration, "shaders", "vertex", "files");

    spdlog::debug("Vertex shaders:");

    for (const auto &vertex_shader_file : m_vertex_shader_files) {
        spdlog::debug("{}", vertex_shader_file);
    }

    m_fragment_shader_files =
        toml::find<std::vector<std::string>>(renderer_configuration, "shaders", "fragment", "files");

    spdlog::debug("Fragment shaders:");

    for (const auto &fragment_shader_file : m_fragment_shader_files) {
        spdlog::debug("{}", fragment_shader_file);
    }

    // TODO: Load more info from TOML file.
}

void Application::load_textures() {
    assert(m_device->device());
    assert(m_device->physical_device());
    assert(m_device->allocator());

    // Insert the new texture into the list of textures.
    std::string texture_name = "unnamed texture";

    for (const auto &texture_file : m_texture_files) {
        wrapper::CpuTexture cpu_texture(texture_file, texture_name);
        m_textures.emplace_back(*m_device, cpu_texture);
    }
}

void Application::load_shaders() {
    assert(m_device->device());

    spdlog::debug("Loading vertex shaders.");

    if (m_vertex_shader_files.empty()) {
        spdlog::error("No vertex shaders to load!");
    }

    // Loop through the list of vertex shaders and initialise all of them.
    for (const auto &vertex_shader_file : m_vertex_shader_files) {
        spdlog::debug("Loading vertex shader file {}.", vertex_shader_file);

        // Insert the new shader into the list of shaders.
        m_shaders.emplace_back(*m_device, VK_SHADER_STAGE_VERTEX_BIT, "unnamed vertex shader", vertex_shader_file);
    }

    spdlog::debug("Loading fragment shaders.");

    if (m_fragment_shader_files.empty()) {
        spdlog::error("No fragment shaders to load!");
    }

    // Loop through the list of fragment shaders and initialise all of them.
    for (const auto &fragment_shader_file : m_fragment_shader_files) {
        spdlog::debug("Loading fragment shader file {}.", fragment_shader_file);

        // Insert the new shader into the list of shaders.
        m_shaders.emplace_back(*m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "unnamed fragment shader",
                               fragment_shader_file);
    }

    spdlog::debug("Loading shaders finished.");
}

void Application::load_octree_geometry() {
    spdlog::debug("Creating octree geometry.");

    // 4: 23 012 | 5: 184352 | 6: 1474162 | 7: 11792978 cubes, DO NOT USE 7!
    m_worlds.clear();
    m_worlds.push_back(world::create_random_world(2, {0.0f, 0.0f, 0.0f}));
    m_worlds.push_back(world::create_random_world(2, {10.0f, 0.0f, 0.0f}));

    m_octree_vertices.clear();
    for (const auto &world : m_worlds) {
        for (const auto &polygons : world->polygons(true)) {
            for (const auto &triangle : *polygons) {
                for (const auto &vertex : triangle) {
                    glm::vec3 color = {
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                        static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    };
                    m_octree_vertices.emplace_back(vertex, color);
                }
            }
        }
    }
}

void Application::check_application_specific_features() {
    assert(m_device->physical_device());

    VkPhysicalDeviceFeatures graphics_card_features;

    vkGetPhysicalDeviceFeatures(m_device->physical_device(), &graphics_card_features);

    // Check if anisotropic filtering is available!
    if (graphics_card_features.samplerAnisotropy != VK_TRUE) {
        spdlog::warn("The selected graphics card does not support anisotropic filtering!");
    } else {
        spdlog::debug("The selected graphics card does support anisotropic filtering.");
    }

    // TODO: Add more checks if necessary.
}

void Application::setup_window_and_input_callbacks() {
    spdlog::debug("Storing GLFW window user pointer.");

    m_window->set_user_ptr(this);

    spdlog::debug("Setting up framebuffer resize callback.");

    auto lambda_frame_buffer_resize_callback = [](GLFWwindow *window, int width, int height) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        spdlog::debug("Frame buffer resize callback called. window width: {}, height: {}", width, height);
        app->m_window_resized = true;
    };

    m_window->set_resize_callback(lambda_frame_buffer_resize_callback);

    spdlog::debug("Setting up keyboard button callback.");

    auto lambda_key_callback = [](GLFWwindow *window, int key, int scancode, int action, int mods) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->key_callback(window, key, scancode, action, mods);
    };

    m_window->set_keyboard_button_callback(lambda_key_callback);

    spdlog::debug("Setting up cursor position callback.");

    auto lambda_cursor_position_callback = [](GLFWwindow *window, double xpos, double ypos) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->cursor_position_callback(window, xpos, ypos);
    };

    m_window->set_cursor_position_callback(lambda_cursor_position_callback);

    spdlog::debug("Setting up mouse button callback.");

    auto lambda_mouse_button_callback = [](GLFWwindow *window, int button, int action, int mods) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->mouse_button_callback(window, button, action, mods);
    };

    m_window->set_mouse_button_callback(lambda_mouse_button_callback);

    spdlog::debug("Setting up mouse wheel scroll callback.");

    auto lambda_mouse_scroll_callback = [](GLFWwindow *window, double xoffset, double yoffset) {
        auto *app = static_cast<Application *>(glfwGetWindowUserPointer(window));
        app->mouse_scroll_callback(window, xoffset, yoffset);
    };

    m_window->set_mouse_scroll_callback(lambda_mouse_scroll_callback);
}

void Application::setup_vulkan_debug_callback() {
    // Check if validation is enabled check for availability of VK_EXT_debug_utils.
    if (m_enable_validation_layers) {
        spdlog::debug("Khronos validation layer is enabled.");

        if (wrapper::Instance::is_extension_supported(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
            auto debug_report_ci = wrapper::make_info<VkDebugReportCallbackCreateInfoEXT>();
            debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | // NOLINT
                                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |                     // NOLINT
                                    VK_DEBUG_REPORT_ERROR_BIT_EXT;                                    // NOLINT

            // We use this user data pointer to signal the callback if "" is specified.
            // All other solutions to this problem either involve duplicated versions of the lambda
            // or global variables.
            debug_report_ci.pUserData = reinterpret_cast<void *>(&m_stop_on_validation_message); // NOLINT

            debug_report_ci.pfnCallback = reinterpret_cast<PFN_vkDebugReportCallbackEXT>( // NOLINT
                +[](VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT, std::uint64_t, std::size_t, std::int32_t,
                    const char *, const char *message, void *user_data) {
                    if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0) {
                        spdlog::info(message);
                    } else if ((flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0) {
                        spdlog::debug(message);
                    } else if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) {
                        spdlog::error(message);
                    } else {
                        // This also deals with VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT.
                        spdlog::warn(message);
                    }

                    // Check if --stop-on-validation-message is enabled.
                    if (user_data != nullptr) {
                        // This feature stops command lines from overflowing with messages in case many validation
                        // layer messages are reported in a short amount of time.
                        spdlog::critical("Command line argument --stop-on-validation-message is enabled.");
                        spdlog::critical("Application will cause a break point now!");

                        // Wait for spdlog to shut down before aborting.
                        spdlog::shutdown();
                        std::abort();
                    }
                    return VK_FALSE;
                });

            // We have to explicitly load this function.
            auto vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>( // NOLINT
                vkGetInstanceProcAddr(m_instance->instance(), "vkCreateDebugReportCallbackEXT"));

            if (vkCreateDebugReportCallbackEXT != nullptr) {
                if (const auto result = vkCreateDebugReportCallbackEXT(m_instance->instance(), &debug_report_ci,
                                                                       nullptr, &m_debug_report_callback);
                    result != VK_SUCCESS) {
                    throw VulkanException("Error: vkCreateDebugReportCallbackEXT failed!", result);
                }
                spdlog::debug("Creating Vulkan debug callback.");
                m_debug_report_callback_initialised = true;
            } else {
                spdlog::error("vkCreateDebugReportCallbackEXT is a null-pointer! Function not available.");
            }
        } else {
            spdlog::warn("Khronos validation layer is not available!");
        }
    } else {
        spdlog::warn("Khronos validation layer is DISABLED.");
    }
}

Application::Application(int argc, char **argv) {
    spdlog::debug("Initialising vulkan-renderer.");
    spdlog::debug("Initialising thread-pool with {} threads.", std::thread::hardware_concurrency());

    tools::CommandLineArgumentParser cla_parser;
    cla_parser.parse_args(argc, argv);

    spdlog::debug("application version: {}.{}.{}", APP_VERSION[0], APP_VERSION[1], APP_VERSION[2]);
    spdlog::debug("engine version: {}.{}.{}", ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2]);

    // Load the configuration from the TOML file.
    load_toml_configuration_file("configuration/renderer.toml");

    bool enable_renderdoc_instance_layer = false;

    auto enable_renderdoc = cla_parser.arg<bool>("--renderdoc");
    if (enable_renderdoc) {
#ifdef NDEBUG
        spdlog::warn("You can't use -renderdoc command line argument in release mode. You have to download the code "
                     "and compile it yourself in debug mode.");
#else
        if (*enable_renderdoc) {
            spdlog::debug("--renderdoc specified, enabling renderdoc instance layer.");
            enable_renderdoc_instance_layer = true;
        }
#endif
    }

    // If the user specified command line argument "--no-validation", the Khronos validation instance layer will be
    // disabled. For debug builds, this is not advisable! Always use validation layers during development!
    const auto disable_validation = cla_parser.arg<bool>("--no-validation");
    if (disable_validation.value_or(false)) {
        spdlog::warn("--no-validation specified, disabling validation layers.");
        m_enable_validation_layers = false;
    }

    spdlog::debug("Creating Vulkan instance.");

    m_glfw_context = std::make_unique<wrapper::GLFWContext>();

    m_instance = std::make_unique<wrapper::Instance>(
        APP_NAME, ENGINE_NAME, VK_MAKE_VERSION(APP_VERSION[0], APP_VERSION[1], APP_VERSION[2]),
        VK_MAKE_VERSION(ENGINE_VERSION[0], ENGINE_VERSION[1], ENGINE_VERSION[2]), VK_API_VERSION_1_1,
        m_enable_validation_layers, enable_renderdoc_instance_layer);

    m_window =
        std::make_unique<wrapper::Window>(m_window_title, m_window_width, m_window_height, true, true, m_window_mode);

    m_input_data = std::make_unique<input::KeyboardMouseInputData>();

    m_surface = std::make_unique<wrapper::WindowSurface>(m_instance->instance(), m_window->get());

    setup_window_and_input_callbacks();

#ifndef NDEBUG
    if (cla_parser.arg<bool>("--stop-on-validation-message").value_or(false)) {
        spdlog::warn("--stop-on-validation-message specified. Application will call a breakpoint after reporting a "
                     "validation layer message.");
        m_stop_on_validation_message = true;
    }

    setup_vulkan_debug_callback();
#endif

    spdlog::debug("Creating window surface.");

    // The user can specify with "--gpu <number>" which graphics card to prefer.
    auto preferred_graphics_card = cla_parser.arg<std::uint32_t>("--gpu");
    if (preferred_graphics_card) {
        spdlog::debug("Preferential graphics card index {} specified.", *preferred_graphics_card);
    }

    bool display_graphics_card_info = true;

    // If the user specified command line argument "--nostats", no information will be
    // displayed about all the graphics cards which are available on the system.
    const auto hide_gpu_stats = cla_parser.arg<bool>("--no-stats");
    if (hide_gpu_stats.value_or(false)) {
        spdlog::debug("--no-stats specified, no extended information about graphics cards will be shown.");
        display_graphics_card_info = false;
    }

    // If the user specified command line argument "--vsync", the presentation engine waits
    // for the next vertical blanking period to update the current image.
    const auto enable_vertical_synchronisation = cla_parser.arg<bool>("--vsync");
    if (enable_vertical_synchronisation.value_or(false)) {
        spdlog::debug("V-sync enabled!");
        m_vsync_enabled = true;
    } else {
        spdlog::debug("V-sync disabled!");
        m_vsync_enabled = false;
    }

    if (display_graphics_card_info) {
        vk_tools::print_all_physical_devices(m_instance->instance(), m_surface->get());
    }

    bool use_distinct_data_transfer_queue = true;

    // Ignore distinct data transfer queue
    const auto forbid_distinct_data_transfer_queue = cla_parser.arg<bool>("--no-separate-data-queue");
    if (forbid_distinct_data_transfer_queue.value_or(false)) {
        spdlog::warn("Command line argument --no-separate-data-queue specified.");
        spdlog::warn("This will force the application to avoid using a distinct queue for data transfer to GPU.");
        spdlog::warn("Performance loss might be a result of this!");
        use_distinct_data_transfer_queue = false;
    }

    bool enable_debug_marker_device_extension = true;

    if (!enable_renderdoc_instance_layer) {
        // Debug markers are only available if RenderDoc is enabled.
        enable_debug_marker_device_extension = false;
    }

    // Check if Vulkan debug markers should be disabled.
    // Those are only available if RenderDoc instance layer is enabled!
    const auto no_vulkan_debug_markers = cla_parser.arg<bool>("--no-vk-debug-markers");
    if (no_vulkan_debug_markers.value_or(false)) {
        spdlog::warn("--no-vk-debug-markers specified, disabling useful debug markers!");
        enable_debug_marker_device_extension = false;
    }

    m_device = std::make_unique<wrapper::Device>(m_instance->instance(), m_surface->get(),
                                                 enable_debug_marker_device_extension, use_distinct_data_transfer_queue,
                                                 preferred_graphics_card);

    check_application_specific_features();

    m_swapchain = std::make_unique<wrapper::Swapchain>(*m_device, m_surface->get(), m_window->width(),
                                                       m_window->height(), m_vsync_enabled, "Standard swapchain");

    load_textures();
    load_shaders();

    m_command_pool = std::make_unique<wrapper::CommandPool>(*m_device, m_device->graphics_queue_family_index());

    m_uniform_buffers.emplace_back(*m_device, "matrices uniform buffer", sizeof(UniformBufferObject));

    // Create an instance of the resource descriptor builder.
    // This allows us to make resource descriptors with the help of a builder pattern.
    wrapper::DescriptorBuilder descriptor_builder(*m_device, m_swapchain->image_count());

    // Make use of the builder to create a resource descriptor for the uniform buffer.
    m_descriptors.emplace_back(
        descriptor_builder.add_uniform_buffer<UniformBufferObject>(m_uniform_buffers[0].buffer(), 0)
            .build("Default uniform buffer"));

    load_octree_geometry();
    generate_octree_indices();

    spdlog::debug("Vulkan initialisation finished.");
    spdlog::debug("Showing window.");

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
    auto cursor_pos = m_input_data->get_cursor_pos();

    ImGuiIO &io = ImGui::GetIO();
    io.DeltaTime = m_time_passed;
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

void Application::process_mouse_input() {
    const auto cursor_pos_delta = m_input_data->calculate_cursor_position_delta();

    if (m_camera->type() == CameraType::LOOK_AT && m_input_data->is_mouse_button_pressed(GLFW_MOUSE_BUTTON_LEFT)) {
        m_camera->rotate(static_cast<float>(cursor_pos_delta[0]), -static_cast<float>(cursor_pos_delta[1]));
    }

    m_camera->set_movement_state(CameraMovement::FORWARD, m_input_data->is_key_pressed(GLFW_KEY_W));
    m_camera->set_movement_state(CameraMovement::LEFT, m_input_data->is_key_pressed(GLFW_KEY_A));
    m_camera->set_movement_state(CameraMovement::BACKWARD, m_input_data->is_key_pressed(GLFW_KEY_S));
    m_camera->set_movement_state(CameraMovement::RIGHT, m_input_data->is_key_pressed(GLFW_KEY_D));
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
    spdlog::debug("Running Application.");

    while (!m_window->should_close()) {
        m_window->poll();
        update_uniform_buffers();
        update_imgui_overlay();
        render_frame();
        process_mouse_input();
        if (m_input_data->was_key_pressed_once(GLFW_KEY_N)) {
            load_octree_geometry();
            generate_octree_indices();
            m_index_buffer->upload_data(m_octree_indices);
            m_vertex_buffer->upload_data(m_octree_vertices);
        }
        m_camera->update(m_time_passed);
        m_time_passed = m_stopwatch.time_step();
        check_octree_collisions();
    }
}

} // namespace inexor::vulkan_renderer
