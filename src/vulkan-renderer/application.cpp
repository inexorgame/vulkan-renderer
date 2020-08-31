#include "inexor/vulkan-renderer/application.hpp"

#include "inexor/vulkan-renderer/debug_callback.hpp"
#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/octree_gpu_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/tools/cla_parser.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <spdlog/spdlog.h>
#include <toml11/toml.hpp>

#include <thread>

namespace inexor::vulkan_renderer {

/// @brief Static callback for window resize events.
/// @note Because GLFW is a C-style API, we can't pass a poiner to a class method, so we have to do it this way!
/// @param window The GLFW window.
/// @param height The width of the window.
/// @param height The height of the window.
/// @TODO Avoid static methods! Poll the events manually in the render loop!
static void frame_buffer_resize_callback(GLFWwindow *window, int width, int height) {
    spdlog::debug("Frame buffer resize callback called. window width: {}, height: {}", width, height);

    // This is actually the way it is handled by the official Vulkan samples.
    auto *app = reinterpret_cast<VulkanRenderer *>(glfwGetWindowUserPointer(window));
    app->m_window_resized = true;
}

void Application::load_toml_configuration_file(const std::string &file_name) {
    spdlog::debug("Loading TOML configuration file: '{}'", file_name);

    std::ifstream toml_file(file_name, std::ios::in);
    if (!toml_file) {
        throw std::runtime_error(std::string("Could not open configuration file: " + file_name + "!"));
    }

    toml_file.close();

    // Load the TOML file using toml11.
    auto renderer_configuration = toml::parse(file_name);

    // Search for the title of the configuration file and print it to debug output.
    auto configuration_title = toml::find<std::string>(renderer_configuration, "title");
    spdlog::debug("Title: '{}'", configuration_title);

    m_window_width = toml::find<int>(renderer_configuration, "application", "window", "width");
    m_window_height = toml::find<int>(renderer_configuration, "application", "window", "height");
    m_window_title = toml::find<std::string>(renderer_configuration, "application", "window", "name");
    spdlog::debug("Window: '{}', {} x {}", m_window_title, m_window_width, m_window_height);

    m_application_name = toml::find<std::string>(renderer_configuration, "application", "name");
    m_engine_name = toml::find<std::string>(renderer_configuration, "application", "engine", "name");
    spdlog::debug("Application name: '{}'", m_application_name);
    spdlog::debug("Engine name: '{}'", m_engine_name);

    int application_version_major = toml::find<int>(renderer_configuration, "application", "version", "major");
    int application_version_minor = toml::find<int>(renderer_configuration, "application", "version", "minor");
    int application_version_patch = toml::find<int>(renderer_configuration, "application", "version", "patch");
    spdlog::debug("Application version {}.{}.{}", application_version_major, application_version_minor,
                  application_version_patch);

    // Generate an std::uint32_t value from the major, minor and patch version info.
    m_application_version =
        VK_MAKE_VERSION(application_version_major, application_version_minor, application_version_patch);

    int engine_version_major = toml::find<int>(renderer_configuration, "application", "engine", "version", "major");
    int engine_version_minor = toml::find<int>(renderer_configuration, "application", "engine", "version", "minor");
    int engine_version_patch = toml::find<int>(renderer_configuration, "application", "engine", "version", "patch");
    spdlog::debug("Engine version {}.{}.{}", engine_version_major, engine_version_minor, engine_version_patch);

    // Generate an std::uint32_t value from the major, minor and patch version info.
    m_engine_version = VK_MAKE_VERSION(engine_version_major, engine_version_minor, engine_version_patch);

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

VkResult Application::load_textures() {
    assert(m_vkdevice->device());
    assert(m_vkdevice->physical_device());
    assert(m_vkdevice->allocator());

    // TODO: Refactor! use key from TOML file as name!
    std::size_t texture_number = 1;

    // Insert the new texture into the list of textures.
    std::string texture_name = "unnamed texture";

    for (const auto &texture_file : m_texture_files) {
        m_textures.emplace_back(*m_vkdevice, m_vkdevice->physical_device(), m_vkdevice->allocator(), texture_file,
                                texture_name, m_vkdevice->graphics_queue(), m_vkdevice->graphics_queue_family_index());
    }

    return VK_SUCCESS;
}

VkResult Application::load_shaders() {
    assert(m_vkdevice->device());

    spdlog::debug("Loading vertex shaders.");

    if (m_vertex_shader_files.empty()) {
        spdlog::error("No vertex shaders to load!");
    }

    auto total_number_of_shaders = m_vertex_shader_files.size() + m_fragment_shader_files.size();

    // Loop through the list of vertex shaders and initialise all of them.
    for (const auto &vertex_shader_file : m_vertex_shader_files) {
        spdlog::debug("Loading vertex shader file {}.", vertex_shader_file);

        // Insert the new shader into the list of shaders.
        m_shaders.emplace_back(*m_vkdevice, VK_SHADER_STAGE_VERTEX_BIT, "unnamed vertex shader", vertex_shader_file);
    }

    spdlog::debug("Loading fragment shaders.");

    if (m_fragment_shader_files.empty()) {
        spdlog::error("No fragment shaders to load!");
    }

    // Loop through the list of fragment shaders and initialise all of them.
    for (const auto &fragment_shader_file : m_fragment_shader_files) {
        spdlog::debug("Loading fragment shader file {}.", fragment_shader_file);

        // Insert the new shader into the list of shaders.
        m_shaders.emplace_back(*m_vkdevice, VK_SHADER_STAGE_FRAGMENT_BIT, "unnamed fragment shader",
                               fragment_shader_file);
    }

    spdlog::debug("Loading shaders finished.");

    return VK_SUCCESS;
}

VkResult Application::load_octree_geometry() {
    spdlog::debug("Creating octree geometry.");

    std::shared_ptr<world::Cube> cube =
        std::make_shared<world::Cube>(world::Cube::Type::OCTANT, 2.0f, glm::vec3{0, -1, -1});
    for (const auto &child : cube->childs()) {
        child->set_type(world::Cube::Type::NORMAL);
        child->indent(8, true, 3);
        child->indent(11, true, 5);
        child->indent(1, false, 2);
    }

    for (const auto &polygons : cube->polygons(true)) {
        glm::vec3 color = {
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
            static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
        };
        for (const auto &triangle : *polygons) {
            for (const auto &vertex : triangle) {
                m_octree_vertices.emplace_back(vertex, color);
            }
        }
    }

    return VK_SUCCESS;
}

VkResult Application::check_application_specific_features() {
    assert(m_vkdevice->physical_device());

    VkPhysicalDeviceFeatures graphics_card_features;

    vkGetPhysicalDeviceFeatures(m_vkdevice->physical_device(), &graphics_card_features);

    // Check if anisotropic filtering is available!
    if (!graphics_card_features.samplerAnisotropy) {
        spdlog::warn("The selected graphics card does not support anisotropic filtering!");
    } else {
        spdlog::debug("The selected graphics card does support anisotropic filtering.");
    }

    // TODO: Add more checks if necessary.

    return VK_SUCCESS;
}

Application::Application(int argc, char **argv) {
    spdlog::debug("Initialising vulkan-renderer.");
    spdlog::debug("Initialising thread-pool with {} threads.", std::thread::hardware_concurrency());

    tools::CommandLineArgumentParser cla_parser;
    cla_parser.parse_args(argc, argv);

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

    bool enable_khronos_validation_instance_layer = true;

    // If the user specified command line argument "--no-validation", the Khronos validation instance layer will be
    // disabled. For debug builds, this is not advisable! Always use validation layers during development!
    auto disable_validation = cla_parser.arg<bool>("--no-validation");
    if (disable_validation.value_or(false)) {
        spdlog::warn("--no-validation specified, disabling validation layers.");
        enable_khronos_validation_instance_layer = false;
    }

    spdlog::debug("Creating Vulkan instance.");

    m_glfw_context = std::make_unique<wrapper::GLFWContext>();

    m_vkinstance = std::make_unique<wrapper::Instance>(
        m_application_name, m_engine_name, m_application_version, m_engine_version, VK_API_VERSION_1_1,
        enable_khronos_validation_instance_layer, enable_renderdoc_instance_layer);

    m_window = std::make_unique<wrapper::Window>(m_window_title, m_window_width, m_window_height, true, true);

    m_surface = std::make_unique<wrapper::WindowSurface>(m_vkinstance->instance(), m_window->get());

    spdlog::debug("Storing GLFW window user pointer.");

    m_window->set_user_ptr(this);

    spdlog::debug("Setting up framebuffer resize callback.");

    m_window->set_resize_callback(frame_buffer_resize_callback);

#ifndef NDEBUG
    // Check if validation is enabled check for availabiliy of VK_EXT_debug_utils.
    if (enable_khronos_validation_instance_layer) {
        spdlog::debug("Khronos validation layer is enabled.");

        if (m_availability_checks_manager->has_instance_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
            auto debug_report_ci = wrapper::make_info<VkDebugReportCallbackCreateInfoEXT>();
            debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
            debug_report_ci.pfnCallback = (PFN_vkDebugReportCallbackEXT)&vulkan_debug_message_callback;

            // We have to explicitly load this function.
            PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
                reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
                    vkGetInstanceProcAddr(m_vkinstance->instance(), "vkCreateDebugReportCallbackEXT"));

            if (vkCreateDebugReportCallbackEXT) {
                // Create the debug report callback.
                VkResult result = vkCreateDebugReportCallbackEXT(m_vkinstance->instance(), &debug_report_ci, nullptr,
                                                                 &m_debug_report_callback);
                if (VK_SUCCESS == result) {
                    spdlog::debug("Creating Vulkan debug callback.");
                    m_debug_report_callback_initialised = true;
                } else {
                    vulkan_error_check(result);
                }
            } else {
                spdlog::error("vkCreateDebugReportCallbackEXT is a null-pointer! Function not available.");
            }
        } else {
            spdlog::warn("Khronos validation layer is not available!");
        }
    } else {
        spdlog::warn("Khronos validation layer is DISABLED.");
    }
#endif

    spdlog::debug("Creating window surface.");

    // The user can specify with "--gpu <number>" which graphics card to prefer.
    auto prefered_graphics_card = cla_parser.arg<std::uint32_t>("--gpu");
    if (prefered_graphics_card) {
        spdlog::debug("Preferential graphics card index {} specified.", *prefered_graphics_card);
    }

    bool display_graphics_card_info = true;

    // If the user specified command line argument "--nostats", no information will be
    // displayed about all the graphics cards which are available on the system.
    auto hide_gpu_stats = cla_parser.arg<bool>("--no-stats");
    if (hide_gpu_stats.value_or(false)) {
        spdlog::debug("--no-stats specified, no extended information about graphics cards will be shown.");
        display_graphics_card_info = false;
    }

    // If the user specified command line argument "--vsync", the presentation engine waits
    // for the next vertical blanking period to update the current image.
    auto enable_vertical_synchronisation = cla_parser.arg<bool>("--vsync");
    if (enable_vertical_synchronisation.value_or(false)) {
        spdlog::debug("V-sync enabled!");
        m_vsync_enabled = true;
    } else {
        spdlog::debug("V-sync disabled!");
        m_vsync_enabled = false;
    }

    if (display_graphics_card_info) {
        spdlog::debug("Displaying extended information about graphics cards.");

        // Print general information about Vulkan.
        m_gpu_info_manager->print_driver_vulkan_version();
        m_gpu_info_manager->print_instance_layers();
        m_gpu_info_manager->print_instance_extensions();

        // Print all information that we can find about all graphics card available.
        // gpu_info_manager->print_all_physical_devices(vkinstance->instance(), surface);
    }

    bool use_distinct_data_transfer_queue = true;

    // Ignore distinct data transfer queue
    auto forbid_distinct_data_transfer_queue = cla_parser.arg<bool>("--no-separate-data-queue");
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
    auto no_vulkan_debug_markers = cla_parser.arg<bool>("--no-vk-debug-markers");
    if (no_vulkan_debug_markers.value_or(false)) {
        spdlog::warn("--no-vk-debug-markers specified, disabling useful debug markers!");
        enable_debug_marker_device_extension = false;
    }

    m_vkdevice =
        std::make_unique<wrapper::Device>(m_vkinstance->instance(), m_surface->get(),
                                          enable_debug_marker_device_extension, use_distinct_data_transfer_queue);

    VkResult result = check_application_specific_features();
    vulkan_error_check(result);

    m_swapchain = std::make_unique<wrapper::Swapchain>(*m_vkdevice, m_vkdevice->physical_device(), m_surface->get(),
                                                       m_window->width(), m_window->height(), m_vsync_enabled,
                                                       "Standard swapchain");

    result = load_textures();
    vulkan_error_check(result);

    result = load_shaders();
    vulkan_error_check(result);

    m_command_pool =
        std::make_unique<wrapper::CommandPool>(m_vkdevice->device(), m_vkdevice->graphics_queue_family_index());

    m_uniform_buffers.emplace_back(m_vkdevice->device(), m_vkdevice->allocator(), "matrices uniform buffer",
                                   sizeof(UniformBufferObject));

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings(1);

    layout_bindings[0].binding = 0;
    layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_bindings[0].descriptorCount = 1;
    layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_bindings[0].pImmutableSamplers = nullptr;

    std::vector<VkWriteDescriptorSet> descriptor_writes(1);

    // Link the matrices uniform buffer to the descriptor set so the shader can access it.

    // We can do better than this, but therefore RAII refactoring needs to be done..
    m_uniform_buffer_info.buffer = m_uniform_buffers[0].buffer();
    m_uniform_buffer_info.offset = 0;
    m_uniform_buffer_info.range = sizeof(UniformBufferObject);

    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = nullptr;
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pBufferInfo = &m_uniform_buffer_info;

    m_descriptors.emplace_back(wrapper::ResourceDescriptor{*m_vkdevice,
                                                           m_swapchain->image_count(),
                                                           {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
                                                           layout_bindings,
                                                           descriptor_writes,
                                                           "Default descriptor"});

    result = load_octree_geometry();
    generate_octree_indices();
    vulkan_error_check(result);

    spdlog::debug("Vulkan initialisation finished.");

    spdlog::debug("Showing window.");
    m_window->show();
    recreate_swapchain();
}

VkResult Application::update_uniform_buffers() {
    float time = m_time_step.time_step_since_initialisation();

    UniformBufferObject ubo{};

    // Rotate the model as a function of time.
    ubo.model = glm::rotate(glm::mat4(1.0f), /*time */ glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    ubo.view = m_game_camera.m_matrices.view;
    ubo.proj = m_game_camera.m_matrices.perspective;
    ubo.proj[1][1] *= -1;

    // TODO: Don't use vector of uniform buffers.
    m_uniform_buffers[0].update(&ubo, sizeof(ubo));

    return VK_SUCCESS;
}

VkResult Application::update_mouse_input() {
    double current_cursor_x{0.0};
    double current_cursor_y{0.0};

    m_window->cursor_pos(current_cursor_x, current_cursor_y);

    double cursor_delta_x = current_cursor_x - m_cursor_x;
    double cursor_delta_y = current_cursor_y - m_cursor_y;

    int state = m_window->is_button_pressed(GLFW_MOUSE_BUTTON_LEFT);

    if (state == GLFW_PRESS) {
        m_game_camera.rotate(glm::vec3(cursor_delta_y * m_game_camera.m_rotation_speed,
                                       -cursor_delta_x * m_game_camera.m_rotation_speed, 0.0f));
    }

    m_window->is_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);

    m_cursor_x = current_cursor_x;
    m_cursor_y = current_cursor_y;

    return VK_SUCCESS;
}

void Application::update_imgui_overlay() {
    double current_cursor_x{0.0};
    double current_cursor_y{0.0};

    glfwGetCursorPos(m_window->get(), &current_cursor_x, &current_cursor_y);

    ImGuiIO &io = ImGui::GetIO();

    // TODO: Does that work? We can't just pass time_passed since it's 0 in the beginning and imgui doesn't accept that.
    io.DeltaTime = std::clamp(m_time_passed, 0.001f, 100.0f);
    // TODO() move to update() method: mouse buttons left+right and cursor position
    // TODO: Use a keyboard/mouse input callback!
    io.MousePos = ImVec2(static_cast<float>(current_cursor_x), static_cast<float>(current_cursor_y));
    io.MouseDown[0] = (glfwGetMouseButton(m_window->get(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    io.MouseDown[1] = (glfwGetMouseButton(m_window->get(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    io.DisplaySize =
        ImVec2(static_cast<float>(m_swapchain->extent().width), static_cast<float>(m_swapchain->extent().height));

    ImGui::NewFrame();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(200, 0));
    ImGui::Begin("Inexor Vulkan-renderer", nullptr,
                 ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("%s", m_vkdevice->gpu_name().c_str());
    ImGui::Text("Engine version %d.%d.%d", VK_VERSION_MAJOR(m_engine_version), VK_VERSION_MINOR(m_engine_version),
                VK_VERSION_PATCH(m_engine_version));
    ImGui::PushItemWidth(150.0f * m_imgui_overlay->get_scale());
    ImGui::PopItemWidth();
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::Render();

    m_imgui_overlay->update();
}

void Application::run() {
    spdlog::debug("Running Application.");

    while (!m_window->should_close()) {
        m_window->poll();
        update_uniform_buffers();
        update_imgui_overlay();
        render_frame();

        // TODO: Run this in a separated thread?
        // TODO: Merge into one update_game_data() method?
        update_mouse_input();
        m_game_camera.update(m_time_passed);

        m_time_passed = m_stopwatch.time_step();
    }
}

} // namespace inexor::vulkan_renderer
