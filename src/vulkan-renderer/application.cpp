#include "inexor/vulkan-renderer/application.hpp"
#include "inexor/vulkan-renderer/debug_callback.hpp"

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
    auto app = reinterpret_cast<VulkanRenderer *>(glfwGetWindowUserPointer(window));
    app->frame_buffer_resized = true;
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
    spdlog::debug("Application version {}.{}.{}", application_version_major, application_version_minor, application_version_patch);

    // Generate an std::uint32_t value from the major, minor and patch version info.
    application_version = VK_MAKE_VERSION(application_version_major, application_version_minor, application_version_patch);

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

    fragment_shader_files = toml::find<std::vector<std::string>>(renderer_configuration, "shaders", "fragment", "files");

    spdlog::debug("Fragment shaders:");

    for (const auto &fragment_shader_file : fragment_shader_files) {
        spdlog::debug("{}", fragment_shader_file);
    }

    // TODO: Load more info from TOML file.

    return VK_SUCCESS;
}

VkResult Application::load_textures() {
    assert(device);
    assert(selected_graphics_card);
    assert(debug_marker_manager);
    assert(vma_allocator);

    // TODO: Refactor! use key from TOML file as name!
    std::size_t texture_number = 1;

    for (const auto &texture_file : texture_files) {

        // Insert the new texture into the list of textures.
        // TODO: Fix this!
        textures.emplace_back(device, selected_graphics_card, vma_allocator, texture_file, std::string("unnamed texture"), gpu_queue_manager->get_data_transfer_queue());
    }

    return VK_SUCCESS;
}

VkResult Application::load_shaders() {
    assert(device);

    spdlog::debug("Loading vertex shaders.");

    if (vertex_shader_files.empty()) {
        spdlog::error("No vertex shaders to load!");
    }

    auto total_number_of_shaders = vertex_shader_files.size() + fragment_shader_files.size();

    // Loop through the list of vertex shaders and initialise all of them.
    for (const auto &vertex_shader_file : vertex_shader_files) {
        spdlog::debug("Loading vertex shader file {}.", vertex_shader_file);

        // Insert the new shader into the list of shaders.
        shaders.emplace_back(device, VK_SHADER_STAGE_VERTEX_BIT, "unnamed vertex shader", vertex_shader_file);
    }

    spdlog::debug("Loading fragment shaders.");

    if (fragment_shader_files.empty()) {
        spdlog::error("No fragment shaders to load!");
    }

    // Loop through the list of fragment shaders and initialise all of them.
    for (const auto &fragment_shader_file : fragment_shader_files) {
        spdlog::debug("Loading fragment shader file {}.", fragment_shader_file);

        // Insert the new shader into the list of shaders.
        shaders.emplace_back(device, VK_SHADER_STAGE_FRAGMENT_BIT, "unnamed fragment shader", fragment_shader_file);
    }

    spdlog::debug("Loading shaders finished.");

    return VK_SUCCESS;
}

/// TODO: Refactor rendering method!
/// TODO: Finish present call using transfer queue.
VkResult Application::render_frame() {
    assert(device);
    assert(gpu_queue_manager->get_graphics_queue());
    assert(gpu_queue_manager->get_present_queue());

    vkWaitForFences(device, 1, &(*in_flight_fences[current_frame]), VK_TRUE, UINT64_MAX);

    std::uint32_t image_index = 0;
    VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, *image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);

    if (images_in_flight[image_index] != VK_NULL_HANDLE) {
        vkWaitForFences(device, 1, &*images_in_flight[image_index], VK_TRUE, UINT64_MAX);
    }

    // Update the data which changes every frame!
    update_uniform_buffers(current_frame);

    // Mark the image as now being in use by this frame.
    images_in_flight[image_index] = in_flight_fences[current_frame];

    // Is it time to regenerate the swapchain because window has been resized or minimized?
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        // VK_ERROR_OUT_OF_DATE_KHR: The swap chain has become incompatible with the surface
        // and can no longer be used for rendering. Usually happens after a window resize.
        return recreate_swapchain();
    }

    // Did something else fail?
    // VK_SUBOPTIMAL_KHR: The swap chain can still be used to successfully present
    // to the surface, but the surface properties are no longer matched exactly.
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        std::string error_message = "Error: Failed to acquire swapchain image!";
        display_error_message(error_message);
        exit(-1);
    }

    const VkPipelineStageFlags wait_stage_mask[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pNext = nullptr;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitDstStageMask = wait_stage_mask;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &command_buffers[image_index];
    submit_info.signalSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &*image_available_semaphores[current_frame];
    submit_info.pSignalSemaphores = &*rendering_finished_semaphores[current_frame];

    vkResetFences(device, 1, &*in_flight_fences[current_frame]);

    result = vkQueueSubmit(gpu_queue_manager->get_graphics_queue(), 1, &submit_info, *in_flight_fences[current_frame]);
    if (result != VK_SUCCESS) {
        return result;
    }

    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.pNext = nullptr;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &*rendering_finished_semaphores[current_frame];
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &swapchain;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;

    result = vkQueuePresentKHR(gpu_queue_manager->get_present_queue(), &present_info);

    // Some notes on frame_buffer_resized:
    // It is important to do this after vkQueuePresentKHR to ensure that the semaphores are
    // in a consistent state, otherwise a signalled semaphore may never be properly waited upon.
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || frame_buffer_resized) {
        frame_buffer_resized = false;
        recreate_swapchain();
    }

    current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;

    auto fps_value = fps_counter.update();

    if (fps_value.has_value()) {
        // Update fps by changing window name.
        std::string window_title = "Inexor Vulkan API renderer demo - " + std::to_string(fps_value.value()) + " FPS";
        glfwSetWindowTitle(window, window_title.c_str());

        spdlog::debug("FPS: {}, window size: {} x {}.", fps_value.value(), window_width, window_height);
    }

    return VK_SUCCESS;
}

VkResult Application::load_octree_geometry() {

    spdlog::debug("Creating octree geometry.");

    std::vector<unsigned char> test = {0xC4, 0x52, 0x03, 0xC0, 0x00, 0x00};

    world::Cube cube = world::Cube::parse(test);
    cube.make_reactive();

    cube.on_change.connect([](world::Cube* c) {
        spdlog::debug("THE WORLD (octree) HAS CHANGED!");
    });

    cube.octants.value()[6]->indentations.value()[4].set_z(4);
    cube.octants.value()[6]->indentations.value()[4] += {1, 1, -3};

    std::vector<std::array<glm::vec3, 3>> polygons = cube.polygons();
    std::vector<OctreeVertex> octree_vertices;
    octree_vertices.resize(polygons.size() * 3);
    OctreeVertex *current_vertex = octree_vertices.data();

    for (auto triangle : polygons) {
        for (auto vertex : triangle) {
            glm::vec3 color = {
                static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
            };
            *current_vertex = {vertex, color};
            current_vertex++;
        }
    }

    // Create a mesh buffer for octree vertex geometry.
    octree_mesh = std::make_shared<MeshBuffer>(device, gpu_queue_manager->get_data_transfer_queue(), gpu_queue_manager->get_data_transfer_queue_family_index().value(),
                                               vma_allocator, std::string("octree mesh"), sizeof(OctreeVertex), octree_vertices.size(), octree_vertices.data());

    return VK_SUCCESS;
}

VkResult Application::load_models() {
    assert(debug_marker_manager);

    spdlog::debug("Loading models.");

    // TODO: Load models from TOML list.
    // TODO: Load glTF 2 models here.

    spdlog::debug("Loading models finished.");

    return VK_SUCCESS;
}

VkResult Application::check_application_specific_features() {
    assert(selected_graphics_card);

    VkPhysicalDeviceFeatures graphics_card_features;

    vkGetPhysicalDeviceFeatures(selected_graphics_card, &graphics_card_features);

    // Check if anisotropic filtering is available!
    if (!graphics_card_features.samplerAnisotropy) {
        spdlog::warn("The selected graphics card does not support anisotropic filtering!");
    } else {
        spdlog::debug("The selected graphics card does support anisotropic filtering.");
    }

    // TODO: Add more checks if necessary.

    return VK_SUCCESS;
}

VkResult Application::init() {
    spdlog::debug("Initialising vulkan-renderer.");

    spdlog::debug("Initialising thread-pool with {} threads.", std::thread::hardware_concurrency());

    // TOOD: Implement -threads <N> command line argument.

    // Initialise Inexor thread-pool.
    thread_pool = std::make_shared<ThreadPool>();

    // Load the configuration from the TOML file.
    VkResult result = load_toml_configuration_file("configuration/renderer.toml");

    vulkan_error_check(result);

    spdlog::debug("Creating window.");

    // Initialise GLFW library.
    glfwInit();

    // We do not want to use the OpenGL API.
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_VISIBLE, false);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create the window using GLFW library.
    window = glfwCreateWindow(window_width, window_height, window_title.c_str(), nullptr, nullptr);

    spdlog::debug("Storing GLFW window user pointer.");

    // Store the current Application instance in the GLFW window user pointer.
    // Since GLFW is a C-style API, we can't use a class method as callback for window resize!
    // TODO: Refactor! Don't use callback functions! use manual polling in the render loop instead.
    glfwSetWindowUserPointer(window, this);

    spdlog::debug("Setting up framebuffer resize callback.");

    // Setup callback for window resize.
    // Since GLFW is a C-style API, we can't use a class method as callback for window resize!
    glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);

    spdlog::debug("Checking for '-renderdoc' command line argument.");

    bool enable_renderdoc_instance_layer = false;

    // If the user specified command line argument "-renderdoc", the RenderDoc instance layer will be enabled.
    std::optional<bool> enable_renderdoc = is_command_line_argument_specified("-renderdoc");

    if (enable_renderdoc.has_value()) {
#if !defined(_DEBUG)
        spdlog::warn("You can't use -renderdoc command line argument in release mode. You have to download the code and compile it yourself in debug mode.");
#else
        if (enable_renderdoc.value()) {
            spdlog::debug("RenderDoc command line argument specified.");
            enable_renderdoc_instance_layer = true;
        }
#endif
    }

    spdlog::debug("Checking for '-novalidation' command line argument.");

    bool enable_khronos_validation_instance_layer = true;

    // If the user specified command line argument "-novalidation", the Khronos validation instance layer will be disabled.
    // For debug builds, this is not advisable! Always use validation layers during development!
    std::optional<bool> disable_validation = is_command_line_argument_specified("-novalidation");

    if (disable_validation.has_value() && disable_validation.value()) {
        spdlog::warn("Vulkan validation layers DISABLED by command line argument -novalidation!.");
        enable_khronos_validation_instance_layer = false;
    }

    spdlog::debug("Creating Vulkan instance.");

    result = create_vulkan_instance(application_name, engine_name, application_version, engine_version, enable_khronos_validation_instance_layer,
                                    enable_renderdoc_instance_layer);
    vulkan_error_check(result);

#if defined(_DEBUG)
    // Check if validation is enabled check for availabiliy of VK_EXT_debug_utils.
    if (enable_khronos_validation_instance_layer) {
        spdlog::debug("Khronos validation layer is enabled.");

        if (availability_checks_manager->has_instance_extension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME)) {
            VkDebugReportCallbackCreateInfoEXT debug_report_create_info = {};

            debug_report_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
            debug_report_create_info.flags =
                VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
            debug_report_create_info.pfnCallback = (PFN_vkDebugReportCallbackEXT)&vulkan_debug_message_callback;

            // We have to explicitly load this function.
            PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
                reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

            if (nullptr != vkCreateDebugReportCallbackEXT) {
                // Create the debug report callback.
                VkResult result = vkCreateDebugReportCallbackEXT(instance, &debug_report_create_info, nullptr, &debug_report_callback);
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

    // Create a window surface using GLFW library.
    // The window surface needs to be created right after the instance creation,
    // because it can actually influence the physical device selection.
    result = create_window_surface(instance, window, surface);
    if (result != VK_SUCCESS) {
        vulkan_error_check(result);
        return result;
    }

    spdlog::debug("Checking for -gpu command line argument.");

    // The user can specify with "-gpu <number>" which graphics card to prefer.
    std::optional<std::uint32_t> prefered_graphics_card = get_command_line_argument_uint32("-gpu");

    if (prefered_graphics_card.has_value()) {
        spdlog::debug("Preferential graphics card index {} specified.", prefered_graphics_card.value());
    }

    // Let's see if there is a graphics card that is suitable for us.
    std::optional<VkPhysicalDevice> graphics_card_candidate =
        settings_decision_maker->decide_which_graphics_card_to_use(instance, surface, prefered_graphics_card);

    // Check if we found a graphics card candidate.
    if (graphics_card_candidate.has_value()) {
        selected_graphics_card = graphics_card_candidate.value();
    } else {
        // No graphics card suitable!
        std::string error_message = "Error: Could not find any suitable GPU!";
        display_fatal_error_message(error_message);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    bool display_graphics_card_info = true;

    spdlog::debug("Checking for -nostats command line argument.");

    // If the user specified command line argument "-nostats", no information will be
    // displayed about all the graphics cards which are available on the system.
    std::optional<bool> hide_gpu_stats = is_command_line_argument_specified("-nostats");

    if (hide_gpu_stats.has_value() && hide_gpu_stats.value()) {
        spdlog::debug("No extended information about graphics cards will be shown.");
        display_graphics_card_info = false;
    }

    spdlog::debug("Checking for -vsync command line argument.");

    // If the user specified command line argument "-vsync", the presentation engine waits
    // for the next vertical blanking period to update the current image.
    std::optional<bool> enable_vertical_synchronisation = is_command_line_argument_specified("-nostats");

    if (enable_vertical_synchronisation.has_value() && enable_vertical_synchronisation.value()) {
        spdlog::debug("V-sync enabled!.");
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
        gpu_info_manager->print_all_physical_devices(instance, surface);
    }

    spdlog::debug("Checking for -no_separate_data_queue command line argument.");

    // Ignore distinct data transfer queue.
    std::optional<bool> forbid_distinct_data_transfer_queue = is_command_line_argument_specified("-no_separate_data_queue");

    bool use_distinct_data_transfer_queue = true;

    if (forbid_distinct_data_transfer_queue.has_value() && forbid_distinct_data_transfer_queue.value()) {
        spdlog::warn("Command line argument -no_separate_data_queue specified.");
        spdlog::warn("This will force the application to avoid using a distinct queue for data transfer to GPU.");
        spdlog::warn("Performance loss might be a result of this!");
        use_distinct_data_transfer_queue = false;
    }

    result = gpu_queue_manager->init(settings_decision_maker);
    vulkan_error_check(result);

    result = gpu_queue_manager->prepare_queues(selected_graphics_card, surface, use_distinct_data_transfer_queue);
    vulkan_error_check(result);

    spdlog::debug("Checking for -no_vk_debug_markers command line argument.");

    bool enable_debug_marker_device_extension = true;

    if (!enable_renderdoc_instance_layer) {
        // Debug markers are only available if RenderDoc is enabled.
        enable_debug_marker_device_extension = false;
    }

    // Check if Vulkan debug markers should be disabled.
    // Those are only available if RenderDoc instance layer is enabled!
    std::optional<bool> no_vulkan_debug_markers = is_command_line_argument_specified("-no_vk_debug_markers");

    if (no_vulkan_debug_markers.has_value() && no_vulkan_debug_markers.value()) {
        spdlog::warn("Vulkan debug markers are disabled because -no_vk_debug_markers was specified.");
        enable_debug_marker_device_extension = false;
    }

    result = create_physical_device(selected_graphics_card, enable_debug_marker_device_extension);
    vulkan_error_check(result);

    result = check_application_specific_features();
    vulkan_error_check(result);

    // Vulkan debug markes will be very useful when debugging with RenderDoc!
    result = initialise_debug_marker_manager(enable_debug_marker_device_extension);
    vulkan_error_check(result);

    result = create_vma_allocator();
    vulkan_error_check(result);

    result = gpu_queue_manager->setup_queues(device);
    vulkan_error_check(result);

    result = create_swapchain();
    vulkan_error_check(result);

    result = create_depth_buffer();
    vulkan_error_check(result);

    spdlog::debug("Starting to load textures using threadpool.");

    result = load_textures();
    vulkan_error_check(result);

    result = load_shaders();
    vulkan_error_check(result);

    result = descriptor_manager->init(device, number_of_images_in_swapchain, debug_marker_manager);
    vulkan_error_check(result);

    result = create_descriptor_pool();
    vulkan_error_check(result);

    result = descriptor_manager->create_descriptor_bundle("inexor_global_descriptor_bundle", global_descriptor_pool, global_descriptor);
    vulkan_error_check(result);

    result = create_descriptor_set_layouts();
    vulkan_error_check(result);

    result = create_pipeline();
    vulkan_error_check(result);

    result = create_frame_buffers();
    vulkan_error_check(result);

    result = create_command_pool();
    vulkan_error_check(result);

    result = create_uniform_buffers();
    vulkan_error_check(result);

    result = create_descriptor_writes();
    vulkan_error_check(result);

    result = create_descriptor_sets();
    vulkan_error_check(result);

    result = create_command_buffers();
    vulkan_error_check(result);

    result = load_models();
    vulkan_error_check(result);

    result = load_octree_geometry();
    vulkan_error_check(result);

    result = record_command_buffers();
    vulkan_error_check(result);

    result = fence_manager->init(device, debug_marker_manager);
    vulkan_error_check(result);

    result = semaphore_manager->init(device, debug_marker_manager);
    vulkan_error_check(result);

    result = create_synchronisation_objects();
    vulkan_error_check(result);

    spdlog::debug("Vulkan initialisation finished.");

    spdlog::debug("Showing window.");

    glfwShowWindow(window);

    // We must store the window user pointer to be able to call the window resize callback.
    // TODO: Use window queue instead?
    glfwSetWindowUserPointer(window, this);

    return recreate_swapchain();
}

VkResult Application::update_uniform_buffers(const std::size_t current_image) {
    float time = time_step.get_time_step_since_initialisation();

    UniformBufferObject ubo = {};

    // Rotate the model as a function of time.
    ubo.model = glm::rotate(glm::mat4(1.0f), /*time */ glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    ubo.view = game_camera.matrices.view;
    ubo.proj = game_camera.matrices.perspective;
    ubo.proj[1][1] *= -1;

    // TODO!
    // Update the world matrices!
    // ..

    return VK_SUCCESS;
}

VkResult Application::update_mouse_input() {

    double current_cursor_x;
    double current_cursor_y;

    glfwGetCursorPos(window, &current_cursor_x, &current_cursor_y);

    double cursor_delta_x = current_cursor_x - cursor_x;
    double cursor_delta_y = current_cursor_y - cursor_y;

    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);

    if (state == GLFW_PRESS) {
        game_camera.rotate(glm::vec3(cursor_delta_y * game_camera.rotation_speed, -cursor_delta_x * game_camera.rotation_speed, 0.0f));
    }

    state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);

    cursor_x = current_cursor_x;
    cursor_y = current_cursor_y;

    return VK_SUCCESS;
}

VkResult Application::update_keyboard_input() {
    int key_w_status = glfwGetKey(window, GLFW_KEY_W);

    /*
    if (key_w_status == GLFW_PRESS) {
        //game_camera_1.start_camera_movement();
    }
    if (key_w_status == GLFW_RELEASE) {
        //game_camera_1.end_camera_movement();
    }
    */

    return VK_SUCCESS;
}

void Application::run() {
    spdlog::debug("Running Application.");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        render_frame();

        // TODO: Run this in a separated thread?
        // TODO: Merge into one update_game_data() method?
        update_keyboard_input();
        update_mouse_input();

        update_cameras();

        time_passed = stopwatch.get_time_step();
    }
}

void Application::cleanup() {
    spdlog::debug("Cleaning up Application.");

    shutdown_vulkan();

    glfwDestroyWindow(window);
    glfwTerminate();

    vertex_shader_files.clear();
    fragment_shader_files.clear();
    texture_files.clear();
    shader_files.clear();
    gltf_model_files.clear();
}

} // namespace inexor::vulkan_renderer
