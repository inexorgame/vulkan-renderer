#include "inexor/vulkan-renderer/application.hpp"

#include "inexor/vulkan-renderer/debug_callback.hpp"
#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/octree_vertex.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/tools/cla_parser.hpp"
#include "inexor/vulkan-renderer/world/cube.hpp"
#include "inexor/vulkan-renderer/wrapper/info.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <spdlog/spdlog.h>
#include <toml11/toml.hpp>

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
    // app->frame_buffer_resized = true;
}

VkResult Application::load_toml_configuration_file(const std::string &file_name) {
    spdlog::debug("Loading TOML configuration file: '{}'", file_name);

    std::ifstream toml_file(file_name, std::ios::in);
    if (!toml_file) {
        spdlog::error("Could not open configuration file: '{}'!", file_name);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    toml_file.close();

    // Load the TOML file using toml11.
    auto renderer_configuration = toml::parse(file_name);

    // Search for the title of the configuration file and print it to debug output.
    auto configuration_title = toml::find<std::string>(renderer_configuration, "title");
    spdlog::debug("Title: '{}'", configuration_title);

    window_width = toml::find<int>(renderer_configuration, "application", "window", "width");
    window_height = toml::find<int>(renderer_configuration, "application", "window", "height");
    window_title = toml::find<std::string>(renderer_configuration, "application", "window", "name");
    spdlog::debug("Window: '{}', {} x {}", window_title, window_width, window_height);

    application_name = toml::find<std::string>(renderer_configuration, "application", "name");
    engine_name = toml::find<std::string>(renderer_configuration, "application", "engine", "name");
    spdlog::debug("Application name: '{}'", application_name);
    spdlog::debug("Engine name: '{}'", engine_name);

    int application_version_major = toml::find<int>(renderer_configuration, "application", "version", "major");
    int application_version_minor = toml::find<int>(renderer_configuration, "application", "version", "minor");
    int application_version_patch = toml::find<int>(renderer_configuration, "application", "version", "patch");
    spdlog::debug("Application version {}.{}.{}", application_version_major, application_version_minor,
                  application_version_patch);

    // Generate an std::uint32_t value from the major, minor and patch version info.
    application_version =
        VK_MAKE_VERSION(application_version_major, application_version_minor, application_version_patch);

    int engine_version_major = toml::find<int>(renderer_configuration, "application", "engine", "version", "major");
    int engine_version_minor = toml::find<int>(renderer_configuration, "application", "engine", "version", "minor");
    int engine_version_patch = toml::find<int>(renderer_configuration, "application", "engine", "version", "patch");
    spdlog::debug("Engine version {}.{}.{}", engine_version_major, engine_version_minor, engine_version_patch);

    // Generate an std::uint32_t value from the major, minor and patch version info.
    engine_version = VK_MAKE_VERSION(engine_version_major, engine_version_minor, engine_version_patch);

    texture_files = toml::find<std::vector<std::string>>(renderer_configuration, "textures", "files");

    spdlog::debug("Textures:");

    for (const auto &texture_file : texture_files) {
        spdlog::debug("{}", texture_file);
    }

    gltf_model_files = toml::find<std::vector<std::string>>(renderer_configuration, "glTFmodels", "files");

    spdlog::debug("glTF 2.0 models:");

    for (const auto &gltf_model_file : gltf_model_files) {
        spdlog::debug("{}", gltf_model_file);
    }

    vertex_shader_files = toml::find<std::vector<std::string>>(renderer_configuration, "shaders", "vertex", "files");

    spdlog::debug("Vertex shaders:");

    for (const auto &vertex_shader_file : vertex_shader_files) {
        spdlog::debug("{}", vertex_shader_file);
    }

    fragment_shader_files =
        toml::find<std::vector<std::string>>(renderer_configuration, "shaders", "fragment", "files");

    spdlog::debug("Fragment shaders:");

    for (const auto &fragment_shader_file : fragment_shader_files) {
        spdlog::debug("{}", fragment_shader_file);
    }

    // TODO: Load more info from TOML file.

    return VK_SUCCESS;
}

VkResult Application::load_textures() {
    assert(vkdevice->get_device());
    assert(vkdevice->get_physical_device());
    assert(vkdevice->allocator());

    // TODO: Refactor! use key from TOML file as name!
    std::size_t texture_number = 1;

    // Insert the new texture into the list of textures.
    std::string texture_name = "unnamed texture";

    for (const auto &texture_file : texture_files) {
        textures.emplace_back(vkdevice->get_device(), vkdevice->get_physical_device(), vkdevice->allocator(),
                              texture_file, texture_name, vkdevice->get_graphics_queue(),
                              vkdevice->get_graphics_queue_family_index());
    }

    return VK_SUCCESS;
}

VkResult Application::load_shaders() {
    assert(vkdevice->get_device());

    spdlog::debug("Loading vertex shaders.");

    if (vertex_shader_files.empty()) {
        spdlog::error("No vertex shaders to load!");
    }

    auto total_number_of_shaders = vertex_shader_files.size() + fragment_shader_files.size();

    // Loop through the list of vertex shaders and initialise all of them.
    for (const auto &vertex_shader_file : vertex_shader_files) {
        spdlog::debug("Loading vertex shader file {}.", vertex_shader_file);

        // Insert the new shader into the list of shaders.
        shaders.emplace_back(vkdevice->get_device(), VK_SHADER_STAGE_VERTEX_BIT, "unnamed vertex shader",
                             vertex_shader_file);
    }

    spdlog::debug("Loading fragment shaders.");

    if (fragment_shader_files.empty()) {
        spdlog::error("No fragment shaders to load!");
    }

    // Loop through the list of fragment shaders and initialise all of them.
    for (const auto &fragment_shader_file : fragment_shader_files) {
        spdlog::debug("Loading fragment shader file {}.", fragment_shader_file);

        // Insert the new shader into the list of shaders.
        shaders.emplace_back(vkdevice->get_device(), VK_SHADER_STAGE_FRAGMENT_BIT, "unnamed fragment shader",
                             fragment_shader_file);
    }

    spdlog::debug("Loading shaders finished.");

    return VK_SUCCESS;
}

VkResult Application::load_octree_geometry() {
    spdlog::debug("Creating octree geometry.");

    std::shared_ptr<world::Cube> cube =
        std::make_shared<world::Cube>(world::Cube::Type::OCTANT, 2, glm::vec3{0, -1, -1});
    for (auto child : cube->childs()) {
        child->set_type(world::Cube::Type::NORMAL);
        child->indent(8, true, 3);
        child->indent(11, true, 5);
        child->indent(1, false, 2);
    }

    std::vector<OctreeVertex> octree_vertices;
    for (const auto &polygons : cube->polygons(true)) {
        for (const auto &triangle : *polygons) {
            for (const auto &vertex : triangle) {
                glm::vec3 color = {
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                    static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                };
                octree_vertices.emplace_back(vertex, color);
            }
        }
    }

    const std::string octree_mesh_name = "unnamed octree";

    // Create a mesh buffer for octree vertex geometry.
    mesh_buffers.emplace_back(vkdevice->get_device(), vkdevice->get_transfer_queue(),
                              vkdevice->get_transfer_queue_family_index(), vkdevice->allocator(), octree_mesh_name,
                              sizeof(OctreeVertex), octree_vertices.size(), octree_vertices.data());

    return VK_SUCCESS;
}

VkResult Application::load_models() {
    spdlog::debug("Loading models.");

    // TODO: Load models from TOML list.
    // TODO: Load glTF 2 models here.

    spdlog::debug("Loading models finished.");

    return VK_SUCCESS;
}

VkResult Application::check_application_specific_features() {
    assert(vkdevice->get_physical_device());

    VkPhysicalDeviceFeatures graphics_card_features;

    vkGetPhysicalDeviceFeatures(vkdevice->get_physical_device(), &graphics_card_features);

    // Check if anisotropic filtering is available!
    if (!graphics_card_features.samplerAnisotropy) {
        spdlog::warn("The selected graphics card does not support anisotropic filtering!");
    } else {
        spdlog::debug("The selected graphics card does support anisotropic filtering.");
    }

    // TODO: Add more checks if necessary.

    return VK_SUCCESS;
}

VkResult Application::init(int argc, char **argv) {
    spdlog::debug("Initialising vulkan-renderer.");
    spdlog::debug("Initialising thread-pool with {} threads.", std::thread::hardware_concurrency());

    tools::CommandLineArgumentParser cla_parser;
    cla_parser.parse_args(argc, argv);

    // Initialise Inexor thread-pool.
    thread_pool = std::make_shared<ThreadPool>();

    // Load the configuration from the TOML file.
    VkResult result = load_toml_configuration_file("configuration/renderer.toml");
    vulkan_error_check(result);

    bool enable_renderdoc_instance_layer = false;

    auto enable_renderdoc = cla_parser.get_arg<bool>("--renderdoc");
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
    auto disable_validation = cla_parser.get_arg<bool>("--no-validation");
    if (disable_validation.value_or(false)) {
        spdlog::warn("--no-validation specified, disabling validation layers.");
        enable_khronos_validation_instance_layer = false;
    }

    spdlog::debug("Creating Vulkan instance.");

    glfw_context = std::make_unique<wrapper::GLFWContext>();

    vkinstance = std::make_unique<wrapper::Instance>(application_name, engine_name, application_version, engine_version,
                                                     VK_API_VERSION_1_1);

    window = std::make_unique<wrapper::Window>(window_title, window_width, window_height, true, true);

    surface = std::make_unique<wrapper::WindowSurface>(vkinstance->get_instance(), window->get());

    spdlog::debug("Storing GLFW window user pointer.");

    window->set_user_ptr(this);

    spdlog::debug("Setting up framebuffer resize callback.");

    window->set_resize_callback(frame_buffer_resize_callback);

#ifndef NDEBUG
    // Check if validation is enabled check for availabiliy of VK_EXT_debug_utils.
    if (enable_khronos_validation_instance_layer) {
        spdlog::debug("Khronos validation layer is enabled.");

        if (availability_checks_manager->has_instance_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
            auto debug_report_ci = wrapper::make_info<VkDebugReportCallbackCreateInfoEXT>();
            debug_report_ci.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
                                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
            debug_report_ci.pfnCallback = (PFN_vkDebugReportCallbackEXT)&vulkan_debug_message_callback;

            // We have to explicitly load this function.
            PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
                reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(
                    vkGetInstanceProcAddr(vkinstance->get_instance(), "vkCreateDebugReportCallbackEXT"));

            if (vkCreateDebugReportCallbackEXT) {
                // Create the debug report callback.
                VkResult result = vkCreateDebugReportCallbackEXT(vkinstance->get_instance(), &debug_report_ci, nullptr,
                                                                 &debug_report_callback);
                if (VK_SUCCESS == result) {
                    spdlog::debug("Creating Vulkan debug callback.");
                    debug_report_callback_initialised = true;
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
    auto prefered_graphics_card = cla_parser.get_arg<std::uint32_t>("--gpu");
    if (prefered_graphics_card) {
        spdlog::debug("Preferential graphics card index {} specified.", *prefered_graphics_card);
    }

    bool display_graphics_card_info = true;

    // If the user specified command line argument "--nostats", no information will be
    // displayed about all the graphics cards which are available on the system.
    auto hide_gpu_stats = cla_parser.get_arg<bool>("--no-stats");
    if (hide_gpu_stats.value_or(false)) {
        spdlog::debug("--no-stats specified, no extended information about graphics cards will be shown.");
        display_graphics_card_info = false;
    }

    // If the user specified command line argument "--vsync", the presentation engine waits
    // for the next vertical blanking period to update the current image.
    auto enable_vertical_synchronisation = cla_parser.get_arg<bool>("--vsync");
    if (enable_vertical_synchronisation.value_or(false)) {
        spdlog::debug("V-sync enabled!");
        vsync_enabled = true;
    } else {
        spdlog::debug("V-sync disabled!");
        vsync_enabled = false;
    }

    if (display_graphics_card_info) {
        spdlog::debug("Displaying extended information about graphics cards.");

        // Print general information about Vulkan.
        gpu_info_manager->print_driver_vulkan_version();
        gpu_info_manager->print_instance_layers();
        gpu_info_manager->print_instance_extensions();

        // Print all information that we can find about all graphics card available.
        // gpu_info_manager->print_all_physical_devices(vkinstance->get_instance(), surface);
    }

    bool use_distinct_data_transfer_queue = true;

    // Ignore distinct data transfer queue
    auto forbid_distinct_data_transfer_queue = cla_parser.get_arg<bool>("--no-separate-data-queue");
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
    auto no_vulkan_debug_markers = cla_parser.get_arg<bool>("--no-vk-debug-markers");
    if (no_vulkan_debug_markers.value_or(false)) {
        spdlog::warn("--no-vk-debug-markers specified, disabling useful debug markers!");
        enable_debug_marker_device_extension = false;
    }

    vkdevice =
        std::make_unique<wrapper::Device>(vkinstance->get_instance(), surface->get(),
                                          enable_debug_marker_device_extension, use_distinct_data_transfer_queue);

    result = check_application_specific_features();
    vulkan_error_check(result);

    swapchain = std::make_unique<wrapper::Swapchain>(vkdevice->get_device(), vkdevice->get_physical_device(),
                                                     surface->get(), window->get_width(), window->get_height(),
                                                     vsync_enabled, "Standard swapchain.");

    spdlog::debug("Starting to load textures using threadpool.");

    result = load_textures();
    vulkan_error_check(result);

    result = load_shaders();
    vulkan_error_check(result);

    result = create_descriptor_pool();
    vulkan_error_check(result);

    result = create_descriptor_set_layouts();
    vulkan_error_check(result);

    command_pool =
        std::make_unique<wrapper::CommandPool>(vkdevice->get_device(), vkdevice->get_graphics_queue_family_index());

    uniform_buffers.emplace_back(vkdevice->get_device(), vkdevice->allocator(), "matrices uniform buffer",
                                 sizeof(UniformBufferObject));

    result = create_descriptor_writes();
    vulkan_error_check(result);

    result = load_models();
    vulkan_error_check(result);

    result = load_octree_geometry();
    vulkan_error_check(result);

    result = create_synchronisation_objects();
    vulkan_error_check(result);

    m_frame_graph =
        std::make_unique<FrameGraph>(vkdevice->get_device(), command_pool->get(), vkdevice->allocator(), *swapchain);
    setup_frame_graph();

    spdlog::debug("Vulkan initialisation finished.");

    spdlog::debug("Showing window.");

    window->show();

    // We must store the window user pointer to be able to call the window resize callback.
    window->set_user_ptr(this);

    // Setup game camera
    game_camera.type = Camera::CameraType::LOOKAT;

    game_camera.set_perspective(45.0f, (float)window_width / (float)window_height, 0.1f, 256.0f);
    game_camera.rotation_speed = 0.25f;
    game_camera.movement_speed = 0.1f;
    game_camera.set_position({0.0f, 0.0f, 5.0f});
    game_camera.set_rotation({0.0f, 0.0f, 0.0f});

    return VK_SUCCESS;
}

VkResult Application::update_uniform_buffers() {
    float time = time_step.get_time_step_since_initialisation();

    UniformBufferObject ubo = {};

    // Rotate the model as a function of time.
    ubo.model = glm::rotate(glm::mat4(1.0f), /*time */ glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    ubo.view = game_camera.matrices.view;
    ubo.proj = game_camera.matrices.perspective;
    ubo.proj[1][1] *= -1;

    // TODO: Don't use vector of uniform buffers.
    uniform_buffers[0].update(&ubo, sizeof(ubo));

    return VK_SUCCESS;
}

VkResult Application::update_mouse_input() {

    double current_cursor_x;
    double current_cursor_y;

    window->get_cursor_pos(current_cursor_x, current_cursor_y);

    double cursor_delta_x = current_cursor_x - cursor_x;
    double cursor_delta_y = current_cursor_y - cursor_y;

    int state =

        window->is_button_pressed(GLFW_MOUSE_BUTTON_LEFT);

    if (state == GLFW_PRESS) {
        game_camera.rotate(
            glm::vec3(cursor_delta_y * game_camera.rotation_speed, -cursor_delta_x * game_camera.rotation_speed, 0.0f));
    }

    window->is_button_pressed(GLFW_MOUSE_BUTTON_RIGHT);

    cursor_x = current_cursor_x;
    cursor_y = current_cursor_y;

    return VK_SUCCESS;
}

void Application::run() {
    spdlog::debug("Running Application.");

    while (!window->should_close()) {
        window->poll();
        update_uniform_buffers();
        render_frame();

        // TODO: Run this in a separated thread?
        // TODO: Merge into one update_game_data() method?
        update_mouse_input();
        game_camera.update(time_passed);

        time_passed = stopwatch.get_time_step();
    }
}

void Application::cleanup() {
    spdlog::debug("Cleaning up Application.");

    shutdown_vulkan();

    window.reset();

    glfw_context.reset();

    vertex_shader_files.clear();
    fragment_shader_files.clear();
    texture_files.clear();
    shader_files.clear();
    gltf_model_files.clear();
}

} // namespace inexor::vulkan_renderer
