#include "inexor/vulkan-renderer/renderer.hpp"

// Vulkan Memory Allocator (VMA) library.
#define VMA_IMPLEMENTATION

// Disable warning C4005 (macro redefinition of VMA_RECORDING_ENABLED) in MS Visual Studio.
#ifdef _MSC_VER
#pragma warning(disable : 4005)
#endif

// It makes memory of all new allocations initialized to bit pattern 0xDCDCDCDC.
// Before an allocation is destroyed, its memory is filled with bit pattern 0xEFEFEFEF.
// Memory is automatically mapped and unmapped if necessary.
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1

// Enforce specified number of bytes as a margin before and after every allocation.
#define VMA_DEBUG_MARGIN 16

// Enable validation of contents of the margins.
#define VMA_DEBUG_DETECT_CORRUPTION 1

// Vulkan Memory Allocator library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// License: MIT.
#include <vma/vk_mem_alloc.h>

namespace inexor::vulkan_renderer {

VkResult VulkanRenderer::create_vulkan_instance(const std::string &application_name, const std::string &engine_name, const uint32_t application_version,
                                                const uint32_t engine_version, bool enable_validation_instance_layers, bool enable_renderdoc_instance_layer) {
    assert(!application_name.empty());
    assert(!engine_name.empty());

    // Get the major, minor and patch version of the application.
    uint32_t app_major = VK_VERSION_MAJOR(application_version);
    uint32_t app_minor = VK_VERSION_MINOR(application_version);
    uint32_t app_patch = VK_VERSION_PATCH(application_version);

    // Get the major, minor and patch version of the engine.
    uint32_t engine_major = VK_VERSION_MAJOR(engine_version);
    uint32_t engine_minor = VK_VERSION_MINOR(engine_version);
    uint32_t engine_patch = VK_VERSION_PATCH(engine_version);

    spdlog::debug("Initialising Vulkan instance.");
    spdlog::debug("Application name: '{}'", application_name.c_str());
    spdlog::debug("Application version: {}.{}.{}", app_major, app_minor, app_patch);
    spdlog::debug("Engine name: '{}'", engine_name.c_str());
    spdlog::debug("Engine version: {}.{}.{}", engine_major, engine_minor, engine_patch);

    // TODO: Switch to VOLK one day? This would allow for dynamic initialisation during runtime without linking vulkan libraries.
    // This would also resolve the issue of checking which version of Vulkan can be initialised.
    // https://github.com/zeux/volk

    // https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkApplicationInfo.html
    // "Because Vulkan 1.0 implementations may fail with VK_ERROR_INCOMPATIBLE_DRIVER,
    // applications should determine the version of Vulkan available before calling vkCreateInstance.
    // If the vkGetInstanceProcAddr returns NULL for vkEnumerateInstanceVersion, it is a Vulkan 1.0 implementation.
    // Otherwise, the application can call vkEnumerateInstanceVersion to determine the version of Vulkan."
    // -This can also be resolved by using VOLK!

    // Structure specifying application's Vulkan API info.
    VkApplicationInfo app_info = {};

    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pNext = nullptr;
    app_info.pApplicationName = application_name.c_str();
    app_info.applicationVersion = application_version;
    app_info.pEngineName = engine_name.c_str();
    app_info.engineVersion = engine_version;
    app_info.apiVersion = VK_API_VERSION_1_1;

    // A vector of strings which represent the enabled instance extensions.
    std::vector<const char *> enabled_instance_extensions;

    // The extensions that we would like to enable.
    std::vector<const char *> instance_extension_wishlist = {
#ifndef NDEBUG
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
        // TODO: Add more instance extensions here.
    };

    // Query which extensions are needed by GLFW.
    uint32_t number_of_GLFW_extensions = 0;

    auto glfw_extensions = glfwGetRequiredInstanceExtensions(&number_of_GLFW_extensions);

    spdlog::debug("Required GLFW instance extensions:");

    for (std::size_t i = 0; i < number_of_GLFW_extensions; i++) {
        spdlog::debug(glfw_extensions[i]);

        // Add instance extensions required by GLFW to our wishlist.
        instance_extension_wishlist.push_back(glfw_extensions[i]);
    }

    for (const auto &instance_extension : instance_extension_wishlist) {
        // TODO: Why is this taking so long?
        // TODO: Limit the number of function calls?
        if (availability_checks_manager->is_instance_extension_available(instance_extension)) {
            spdlog::debug("Adding '{}' to instance extension wishlist.", instance_extension);
            enabled_instance_extensions.push_back(instance_extension);
        } else {
            std::string error_message = "Error: Required instance extension '" + std::string(instance_extension) + "' is not available!";
            display_warning_message(error_message);
        }
    }

    // A vector of strings which represent the enabled instance layers.
    std::vector<const char *> enabled_instance_layers;

    // The layers that we would like to enable.
    std::vector<const char *> instance_layers_wishlist = {
        // RenderDoc instance layer can be specified using -renderdoc command line argument.
        // TODO: Add instance layers if neccesary..
    };

    /// RenderDoc is a modern graphics debugger written by Baldur Karlsson.
    /// It comes with many useful debugging functions!
    /// https://renderdoc.org/
    /// https://github.com/baldurk/renderdoc
#ifndef NDEBUG
    if (enable_renderdoc_instance_layer) {
        const char renderdoc_layer_name[] = "VK_LAYER_RENDERDOC_Capture";

        spdlog::debug("Adding '{}' to instance extension wishlist.", renderdoc_layer_name);
        instance_layers_wishlist.push_back(renderdoc_layer_name);
    }
#endif

    // If validation is requested, we need to add the validation layer as instance extension!
    // For more information on Vulkan validation layers see:
    // https://vulkan.lunarg.com/doc/view/1.0.39.0/windows/layers.html
#ifndef NDEBUG
    if (enable_validation_instance_layers) {
        const char validation_layer_name[] = "VK_LAYER_KHRONOS_validation";

        spdlog::debug("Adding '{}' to instance extension wishlist.", validation_layer_name);
        instance_layers_wishlist.push_back(validation_layer_name);
    }
#endif

    // We now have to check which instance layers of our wishlist are really supported on the current system!
    // Loop through the wishlist and check for availabiliy.
    for (auto current_layer : instance_layers_wishlist) {
        if (availability_checks_manager->is_instance_layer_available(current_layer)) {
            spdlog::debug("Instance layer '{}' is supported.", current_layer);

            // This instance layer is available!
            // Add it to the list of enabled instance layers!
            enabled_instance_layers.push_back(current_layer);
        } else {
#ifdef NDEBUG
            if (0 == std::string(current_layer).compare(VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
                display_warning_message(
                    "You can't use -renderdoc command line argument in release mode. You have to download the code and compile it yourself in debug mode.");
            }
#endif

            std::string error_message = "Error: Instance layer '" + std::string(current_layer) + "' is not available!";
            display_error_message(error_message);
        }
    }

    VkInstanceCreateInfo instance_create_info = {};

    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pNext = nullptr;
    instance_create_info.flags = 0;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.ppEnabledExtensionNames = enabled_instance_extensions.data();
    instance_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_instance_extensions.size());
    instance_create_info.ppEnabledLayerNames = enabled_instance_layers.data();
    instance_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_instance_layers.size());

    VkResult result = vkCreateInstance(&instance_create_info, nullptr, &instance);
    vulkan_error_check(result);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_window_surface(const VkInstance &instance, GLFWwindow *window, VkSurfaceKHR &surface) {
    assert(window);
    assert(instance);

    spdlog::debug("Creating window surface.");

    // Create a window surface using GLFW library.
    return glfwCreateWindowSurface(instance, window, nullptr, &surface);
}

VkResult VulkanRenderer::create_physical_device(const VkPhysicalDevice &graphics_card, const bool enable_debug_markers) {
    assert(graphics_card);

    spdlog::debug("Creating physical device (graphics card interface).");

    VkPhysicalDeviceFeatures used_features = {};

    // Enable anisotropic filtering.
    used_features.samplerAnisotropy = VK_TRUE;

    // Our wishlist of device extensions that we would like to enable.
    std::vector<const char *> device_extensions_wishlist = {
        // Since we actually want a window to draw on, we need this swapchain extension.
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

    if (enable_debug_markers) {
        // Debug markers are only present if RenderDoc is enabled.
        device_extensions_wishlist.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }

    // The actual list of enabled device extensions.
    std::vector<const char *> enabled_device_extensions;

    for (auto device_extension_name : device_extensions_wishlist) {
        if (availability_checks_manager->is_device_extension_available(graphics_card, device_extension_name)) {
            spdlog::debug("Device extension '{}' is supported!", device_extension_name);

            // This device layer is supported!
            // Add it to the list of enabled device layers.
            enabled_device_extensions.push_back(device_extension_name);
        } else {
            // This device layer is not supported!
            std::string error_message = "Device extension '" + std::string(device_extension_name) + " not supported!";
            display_error_message(error_message);
        }
    }

    VkDeviceCreateInfo device_create_info = {};

    auto queues_to_create = gpu_queue_manager->get_queues_to_create();

    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = nullptr;
    device_create_info.flags = 0;
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queues_to_create.size());
    device_create_info.pQueueCreateInfos = queues_to_create.data();
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_device_extensions.size());
    device_create_info.ppEnabledExtensionNames = enabled_device_extensions.data();
    device_create_info.pEnabledFeatures = &used_features;

    VkResult result = vkCreateDevice(graphics_card, &device_create_info, nullptr, &device);
    vulkan_error_check(result);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::initialise_debug_marker_manager(const bool enable_debug_markers) {
    assert(device);
    assert(selected_graphics_card);

    spdlog::debug("Initialising debug marker manager.");

#ifndef NDEBUG
    if (!enable_debug_markers) {
        spdlog::warn("Vulkan debug markers are not enabled!");
        spdlog::warn("This will be of disadvantage when debugging the application with e.g. RenderDoc.");
    }
#endif

    debug_marker_manager->init(device, selected_graphics_card, enable_debug_markers);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_command_pool() {
    assert(device);
    assert(debug_marker_manager);
    assert(gpu_queue_manager->get_graphics_family_index().has_value());

    spdlog::debug("Creating command pool for rendering.");

    VkCommandPoolCreateInfo command_pool_create_info = {};

    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    command_pool_create_info.queueFamilyIndex = gpu_queue_manager->get_graphics_family_index().value();

    VkResult result = vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool);
    vulkan_error_check(result);

    // Give this command pool an appropriate name.
    debug_marker_manager->set_object_name(device, (uint64_t)(command_pool), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, "Core engine command pool.");

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_depth_buffer() {
    VkImageTiling tiling = VK_IMAGE_TILING_OPTIMAL;

    VkFormatFeatureFlags format = VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT;

    VkImageUsageFlags image_usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    // Supported candidates for depth buffer format.
    const std::vector<VkFormat> depth_buffer_format_candidates = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT,
                                                                  VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};

    // Try to find an appropriate format for the depth buffer.
    auto depth_buffer_format_candidate =
        settings_decision_maker->find_depth_buffer_format(selected_graphics_card, depth_buffer_format_candidates, tiling, format);

    assert(depth_buffer_format_candidate.has_value());

    depth_buffer.format = depth_buffer_format_candidate.value();

    VkImageCreateInfo depth_buffer_image_create_info = {};

    depth_buffer_image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_buffer_image_create_info.imageType = VK_IMAGE_TYPE_2D;
    depth_buffer_image_create_info.extent.width = swapchain_image_extent.width;
    depth_buffer_image_create_info.extent.height = swapchain_image_extent.height;
    depth_buffer_image_create_info.extent.depth = 1;
    depth_buffer_image_create_info.mipLevels = 1;
    depth_buffer_image_create_info.arrayLayers = 1;
    depth_buffer_image_create_info.format = depth_buffer.format;
    depth_buffer_image_create_info.tiling = tiling;
    depth_buffer_image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_buffer_image_create_info.usage = image_usage;
    depth_buffer_image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_buffer_image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Image creation does not allocate memory for the image automatically.
    // This is done in the following code part:
    depth_buffer.allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    depth_buffer.allocation_create_info.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;

    std::string depth_buffer_image_name = "Depth buffer image.";

    depth_buffer.allocation_create_info.pUserData = depth_buffer_image_name.data();

    VkResult result = vmaCreateImage(vma_allocator, &depth_buffer_image_create_info, &depth_buffer.allocation_create_info, &depth_buffer.image,
                                     &depth_buffer.allocation, &depth_buffer.allocation_info);
    vulkan_error_check(result);

    // Give this depth buffer image an appropriate name.
    debug_marker_manager->set_object_name(device, (uint64_t)(depth_buffer.image), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, "Depth buffer image.");

    VkImageViewCreateInfo view_info = {};

    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = depth_buffer.image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = depth_buffer.format;
    view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    result = vkCreateImageView(device, &view_info, nullptr, &depth_buffer.image_view);
    vulkan_error_check(result);

    // Give this buffer image view an appropriate name.
    debug_marker_manager->set_object_name(device, (uint64_t)(depth_buffer.image_view), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, "Depth buffer image view.");

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_command_buffers() {
    assert(device);
    assert(debug_marker_manager);
    assert(number_of_images_in_swapchain > 0);

    spdlog::debug("Allocating command buffers.");
    spdlog::debug("Number of images in swapchain: {}.", number_of_images_in_swapchain);

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.pNext = nullptr;
    command_buffer_allocate_info.commandPool = command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = number_of_images_in_swapchain;

    command_buffers.clear();
    command_buffers.resize(number_of_images_in_swapchain);

    VkResult result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, command_buffers.data());
    vulkan_error_check(result);

    for (std::size_t i = 0; i < command_buffers.size(); i++) {
        std::string command_buffer_name = "Command buffer " + std::to_string(i) + " for core engine.";

        debug_marker_manager->set_object_name(device, (uint64_t)(command_buffers[i]), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                              command_buffer_name.c_str());
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_vma_allocator() {
    assert(device);
    assert(selected_graphics_card);
    assert(debug_marker_manager);

    spdlog::debug("Initialising Vulkan memory allocator.");

    // VMA memory recording and replay.
    VmaRecordSettings vma_record_settings;

    const std::string vma_replay_file = "vma-replays/vma_replay.csv";

    std::ofstream replay_file_test;
    replay_file_test.open(vma_replay_file, std::ios::out);

    // Check if we can open the csv file.
    // This causes problems when the debugging path is set incorrectly!
    if (!replay_file_test.is_open()) {
        spdlog::error("Could not open VMA replay file {}", vma_replay_file);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    replay_file_test.close();

    vma_record_settings.pFilePath = vma_replay_file.c_str();

    // We flush the stream after every write operation because we are expecting unforseen program crashes.
    // This might has a negative effect on the application's performance but it's worth it for now.
    vma_record_settings.flags = VMA_RECORD_FLUSH_AFTER_CALL_BIT;

    VmaAllocatorCreateInfo allocator_info = {};

    allocator_info.physicalDevice = selected_graphics_card;
    allocator_info.device = device;
#if VMA_RECORDING_ENABLED
    allocator_info.pRecordSettings = &vma_record_settings;
#endif
    allocator_info.instance = instance;

    // Create an instance of Vulkan memory allocator.
    VkResult result = vmaCreateAllocator(&allocator_info, &vma_allocator);
    vulkan_error_check(result);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::record_command_buffers() {
    assert(debug_marker_manager);
    assert(window_width > 0);
    assert(window_height > 0);

    spdlog::debug("Recording command buffers.");

    VkCommandBufferBeginInfo command_buffer_begin_info = {};

    command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    command_buffer_begin_info.pNext = nullptr;
    command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    command_buffer_begin_info.pInheritanceInfo = nullptr;

    // TODO: Setup clear colors by TOML configuration file.
    std::array<VkClearValue, 3> clear_values;

    // Note that the order of clear_values must be identical to the order of the attachments.
    if (multisampling_enabled) {
        clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_values[1].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_values[2].depthStencil = {1.0f, 0};
    } else {
        clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clear_values[1].depthStencil = {1.0f, 0};
    }

    VkRenderPassBeginInfo render_pass_begin_info = {};

    render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin_info.pNext = nullptr;
    render_pass_begin_info.renderPass = render_pass;
    render_pass_begin_info.renderArea.offset = {0, 0};
    render_pass_begin_info.renderArea.extent = {window_width, window_height};
    render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
    render_pass_begin_info.pClearValues = clear_values.data();

    VkViewport viewport{};

    viewport.width = (float)window_width;
    viewport.height = (float)window_height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};

    scissor.extent.width = window_width;
    scissor.extent.height = window_height;

    for (std::size_t i = 0; i < number_of_images_in_swapchain; i++) {
        spdlog::debug("Recording command buffer #{}.", i);

        // TODO: Fix debug marker regions in RenderDoc.
        // Start binding the region with Vulkan debug markers.
        debug_marker_manager->bind_region(command_buffers[i], "Beginning of rendering.", INEXOR_DEBUG_MARKER_GREEN);

        VkResult result = vkBeginCommandBuffer(command_buffers[i], &command_buffer_begin_info);
        if (VK_SUCCESS != result)
            return result;

        // Update only the necessary parts of VkRenderPassBeginInfo.
        render_pass_begin_info.framebuffer = frame_buffers[i];

        vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        // ----------------------------------------------------------------------------------------------------------------
        // Begin of render pass.
        {
            vkCmdSetViewport(command_buffers[i], 0, 1, &viewport);

            vkCmdSetScissor(command_buffers[i], 0, 1, &scissor);

            // TODO: Render skybox!

            vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

            // TODO: This does not specify the order of rendering!
            gltf_model_manager->render_all_models(command_buffers[i], pipeline_layout, i);

            // TODO: Draw imgui user interface.
        }
        // End of render pass.
        // ----------------------------------------------------------------------------------------------------------------

        vkCmdEndRenderPass(command_buffers[i]);

        result = vkEndCommandBuffer(command_buffers[i]);
        if (VK_SUCCESS != result)
            return result;

        debug_marker_manager->end_region(command_buffers[i]);
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_synchronisation_objects() {
    assert(number_of_images_in_swapchain > 0);
    assert(debug_marker_manager);

    spdlog::debug("Creating synchronisation objects: semaphores and fences.");
    spdlog::debug("Number of images in swapchain: {}.", number_of_images_in_swapchain);

    in_flight_fences.clear();
    image_available_semaphores.clear();
    rendering_finished_semaphores.clear();

    // TODO: Add method to create several fences/semaphores.

    for (std::size_t i = 0; i < INEXOR_MAX_FRAMES_IN_FLIGHT; i++) {
        // Here we create the semaphores and fences which are neccesary for synchronisation.
        // Cleanup will be handled by VulkanSynchronisationManager.
        std::string image_available_semaphore_name = "image_available_semaphores_" + std::to_string(i);
        std::string rendering_finished_semaphore_name = "rendering_finished_semaphores_" + std::to_string(i);
        std::string in_flight_fence_name = "in_flight_fences_" + std::to_string(i);

        auto in_flight_fence = fence_manager->create_fence(in_flight_fence_name, true);
        auto new_image_available_semaphore = semaphore_manager->create_semaphore(image_available_semaphore_name);
        auto new_rendering_finished_semaphore = semaphore_manager->create_semaphore(rendering_finished_semaphore_name);

        in_flight_fences.push_back(in_flight_fence.value());
        image_available_semaphores.push_back(new_image_available_semaphore.value());
        rendering_finished_semaphores.push_back(new_rendering_finished_semaphore.value());
    }

    images_in_flight.clear();

    // Note: Images in flight do not need to be initialised!
    images_in_flight.resize(number_of_images_in_swapchain, VK_NULL_HANDLE);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_swapchain() {
    assert(device);
    assert(surface);
    assert(selected_graphics_card);
    assert(debug_marker_manager);

    // TODO: Implement command line argument!
    bool vsync = false;

    // Store the old swapchain. This allows us to pass it
    // to VkSwapchainCreateInfoKHR::oldSwapchain to speed up swapchain recreation.
    // TODO: Is it a problem that this value is VK_NULL_HANDLE in the beginning?
    VkSwapchainKHR old_swapchain = swapchain;

    // Select extent of swapchain images.
    settings_decision_maker->decide_width_and_height_of_swapchain_extent(selected_graphics_card, surface, window_width, window_height, swapchain_image_extent);

    // TODO: Remove std::optional from this method!
    // Select a present mode for the swapchain.
    std::optional<VkPresentModeKHR> present_mode =
        settings_decision_maker->decide_which_presentation_mode_to_use(selected_graphics_card, surface, vsync_enabled);

    assert(present_mode.has_value());

    selected_present_mode = present_mode.value();

    // Decide how many images in swapchain to use.
    number_of_images_in_swapchain = settings_decision_maker->decide_how_many_images_in_swapchain_to_use(selected_graphics_card, surface);

    // Find the transformation of the surface.
    auto pre_transform = settings_decision_maker->decide_which_image_transformation_to_use(selected_graphics_card, surface);

    // Find a supported composite alpha format (not all devices support alpha opaque)
    auto composite_alpha_format = settings_decision_maker->find_composite_alpha_format(selected_graphics_card, surface);

    // TODO: Remove std::optional from this method?
    // Find a color format.
    auto selected_image_format_candidate = settings_decision_maker->decide_which_surface_color_format_in_swapchain_to_use(selected_graphics_card, surface);

    assert(selected_image_format_candidate.has_value());

    selected_color_space = selected_image_format_candidate.value().colorSpace;
    selected_image_format = selected_image_format_candidate.value().format;

    VkSwapchainCreateInfoKHR swapchain_create_info = {};

    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = nullptr;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = number_of_images_in_swapchain;
    swapchain_create_info.imageFormat = selected_image_format;
    swapchain_create_info.imageColorSpace = selected_color_space;
    swapchain_create_info.imageExtent.width = swapchain_image_extent.width;
    swapchain_create_info.imageExtent.height = swapchain_image_extent.height;
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = (VkSurfaceTransformFlagBitsKHR)pre_transform;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = nullptr;
    swapchain_create_info.presentMode = selected_present_mode;
    swapchain_create_info.oldSwapchain = old_swapchain;
    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area.
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.compositeAlpha = composite_alpha_format;

    // Set additional usage flag for blitting from the swapchain images if supported.
    VkFormatProperties formatProps;

    vkGetPhysicalDeviceFormatProperties(selected_graphics_card, selected_image_format, &formatProps);

    if ((formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) || (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    VkResult result = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);
    vulkan_error_check(result);

    // If an existing swapchain is recreated, destroy the old swapchain.
    // This also cleans up all the presentable images.
    if (VK_NULL_HANDLE != old_swapchain) {
        if (swapchain_image_views.size() > 0) {
            for (auto image_view : swapchain_image_views) {
                if (VK_NULL_HANDLE != image_view) {
                    vkDestroyImageView(device, image_view, nullptr);
                    image_view = VK_NULL_HANDLE;
                }
            }

            swapchain_image_views.clear();
        }

        vkDestroySwapchainKHR(device, old_swapchain, nullptr);

        swapchain_images.clear();
    }

    result = vkGetSwapchainImagesKHR(device, swapchain, &number_of_images_in_swapchain, nullptr);
    vulkan_error_check(result);

    swapchain_images.resize(number_of_images_in_swapchain);

    result = vkGetSwapchainImagesKHR(device, swapchain, &number_of_images_in_swapchain, swapchain_images.data());
    vulkan_error_check(result);

    for (std::size_t i = 0; i < swapchain_images.size(); i++) {
        std::string debug_marker_name = "Swapchain image #" + std::to_string(i);

        // Assign an appropriate debug marker name to the swapchain images.
        debug_marker_manager->set_object_name(device, (uint64_t)(swapchain_images[i]), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, debug_marker_name.c_str());
    }

    spdlog::debug("Creating swapchainm image views.");
    spdlog::debug("Number of images in swapchain: {}.", number_of_images_in_swapchain);

    // Preallocate memory for the image views.
    swapchain_image_views.clear();
    swapchain_image_views.resize(number_of_images_in_swapchain);

    for (std::size_t i = 0; i < number_of_images_in_swapchain; i++) {
        spdlog::debug("Creating swapchain image #{}.", i);

        VkImageViewCreateInfo image_view_create_info = {};

        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.pNext = nullptr;
        image_view_create_info.flags = 0;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = selected_image_format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;
        image_view_create_info.image = swapchain_images[i];

        VkResult result = vkCreateImageView(device, &image_view_create_info, nullptr, &swapchain_image_views[i]);
        if (VK_SUCCESS != result)
            return result;

        std::string swapchain_image_view_name = "Swapchain image view #" + std::to_string(i) + ".";

        // Use Vulkan debug markers to assign an appropriate name to this swapchain image view.
        debug_marker_manager->set_object_name(device, (uint64_t)(swapchain_image_views[i]), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT,
                                              swapchain_image_view_name.c_str());
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::cleanup_swapchain() {
    spdlog::debug("Cleaning up swapchain.");

    spdlog::debug("Waiting for device to be idle.");

    vkDeviceWaitIdle(device);

    spdlog::debug("Device is idle.");

    spdlog::debug("Destroying frame buffer.");

    if (frame_buffers.size() > 0) {
        for (auto frame_buffer : frame_buffers) {
            if (VK_NULL_HANDLE != frame_buffer) {
                vkDestroyFramebuffer(device, frame_buffer, nullptr);
                frame_buffer = VK_NULL_HANDLE;
            }
        }

        frame_buffers.clear();
    }

    spdlog::debug("Destroying depth buffer image view.");

    if (VK_NULL_HANDLE != depth_buffer.image_view) {
        vkDestroyImageView(device, depth_buffer.image_view, nullptr);
        depth_buffer.image_view = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying depth buffer image.");

    if (VK_NULL_HANDLE != depth_buffer.image) {
        vmaDestroyImage(vma_allocator, depth_buffer.image, depth_buffer.allocation);
        depth_buffer.image = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying depth stencil image view.");

    if (VK_NULL_HANDLE != depth_stencil.image_view) {
        vkDestroyImageView(device, depth_stencil.image_view, nullptr);
        depth_stencil.image_view = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying depth stencil image.");

    if (VK_NULL_HANDLE != depth_stencil.image) {
        vmaDestroyImage(vma_allocator, depth_stencil.image, depth_stencil.allocation);
        depth_stencil.image = VK_NULL_HANDLE;
    }

    if (multisampling_enabled) {
        spdlog::debug("Destroying multisampling color target image view.");

        if (VK_NULL_HANDLE != msaa_target_buffer.color.image_view) {
            vkDestroyImageView(device, msaa_target_buffer.color.image_view, nullptr);
            msaa_target_buffer.color.image_view = VK_NULL_HANDLE;
        }

        spdlog::debug("Destroying multisampling color target image.");

        if (VK_NULL_HANDLE != msaa_target_buffer.color.image) {
            vmaDestroyImage(vma_allocator, msaa_target_buffer.color.image, msaa_target_buffer.color.allocation);
            msaa_target_buffer.color.allocation = VK_NULL_HANDLE;
        }

        spdlog::debug("Destroying multisampling depth target image view.");

        if (VK_NULL_HANDLE != msaa_target_buffer.depth.image_view) {
            vkDestroyImageView(device, msaa_target_buffer.depth.image_view, nullptr);
            msaa_target_buffer.depth.image_view = VK_NULL_HANDLE;
        }

        spdlog::debug("Destroying multisampling depth target image.");

        if (VK_NULL_HANDLE != msaa_target_buffer.depth.image) {
            vmaDestroyImage(vma_allocator, msaa_target_buffer.depth.image, msaa_target_buffer.depth.allocation);
            msaa_target_buffer.depth.allocation = VK_NULL_HANDLE;
        }
    }

    spdlog::debug("Destroying command buffers.");

    // We do not need to reset the command buffers explicitly, since it is covered by vkDestroyCommandPool.
    if (command_buffers.size() > 0) {
        // The size of the command buffer is equal to the number of image in swapchain.
        vkFreeCommandBuffers(device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());

        command_buffers.clear();
    }

    // TODO: Pack every shutdown of a resource into an own function.

    spdlog::debug("Destroying depth buffer image view.");

    if (VK_NULL_HANDLE != depth_buffer.image_view) {
        vkDestroyImageView(device, depth_buffer.image_view, nullptr);
        depth_buffer.image_view = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying depth buffer image.");

    if (VK_NULL_HANDLE != depth_buffer.image) {
        vmaDestroyImage(vma_allocator, depth_buffer.image, depth_buffer.allocation);
        depth_buffer.image = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying pipeline.");

    if (VK_NULL_HANDLE != pipeline) {
        vkDestroyPipeline(device, pipeline, nullptr);
        pipeline = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying pipeline layout.");

    if (VK_NULL_HANDLE != pipeline_layout) {
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        pipeline_layout = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying render pass.");

    if (VK_NULL_HANDLE != render_pass) {
        vkDestroyRenderPass(device, render_pass, nullptr);
        render_pass = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying image views.");

    if (swapchain_image_views.size() > 0) {
        for (auto image_view : swapchain_image_views) {
            if (VK_NULL_HANDLE != image_view) {
                vkDestroyImageView(device, image_view, nullptr);
                image_view = VK_NULL_HANDLE;
            }
        }

        swapchain_image_views.clear();
    }

    swapchain_images.clear();

    spdlog::debug("Destroying swapchain.");

    if (VK_NULL_HANDLE != swapchain) {
        vkDestroySwapchainKHR(device, swapchain, nullptr);
        swapchain = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying uniform buffers.");

    uniform_buffer_manager->shutdown_uniform_buffers();

    spdlog::debug("Destroying descriptor sets and layouts.");

    descriptor_manager->shutdown_descriptors();

    return VK_SUCCESS;
}

VkResult VulkanRenderer::update_cameras() {
    game_camera_1.update(time_passed);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::recreate_swapchain() {
    assert(device);

    vkDeviceWaitIdle(device);

    // TODO: outsource cleanup_swapchain() methods!

    int current_window_width = 0;
    int current_window_height = 0;

    spdlog::debug("Querying new window size.");

    // If window is minimized, wait until it is visible again.
    while (current_window_width == 0 || current_window_height == 0) {
        glfwGetFramebufferSize(window, &current_window_width, &current_window_height);
        glfwWaitEvents();
    }

    window_width = current_window_width;
    window_height = current_window_height;

    spdlog::debug("New window size: width: {}, height: {}.", window_width, window_height);

    VkResult result = create_swapchain();
    vulkan_error_check(result);

    spdlog::debug("Destroying depth buffer image view.");

    if (VK_NULL_HANDLE != depth_buffer.image_view) {
        vkDestroyImageView(device, depth_buffer.image_view, nullptr);
        depth_buffer.image_view = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying depth buffer image.");

    if (VK_NULL_HANDLE != depth_buffer.image) {
        vmaDestroyImage(vma_allocator, depth_buffer.image, depth_buffer.allocation);
        depth_buffer.image = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying depth stencil image view.");

    if (VK_NULL_HANDLE != depth_stencil.image_view) {
        vkDestroyImageView(device, depth_stencil.image_view, nullptr);
        depth_stencil.image_view = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying depth stencil image.");

    if (VK_NULL_HANDLE != depth_stencil.image) {
        vmaDestroyImage(vma_allocator, depth_stencil.image, depth_stencil.allocation);
        depth_stencil.image = VK_NULL_HANDLE;
    }

    if (multisampling_enabled) {
        spdlog::debug("Destroying multisampling color target image view.");

        if (VK_NULL_HANDLE != msaa_target_buffer.color.image_view) {
            vkDestroyImageView(device, msaa_target_buffer.color.image_view, nullptr);
            msaa_target_buffer.color.image_view = VK_NULL_HANDLE;
        }

        spdlog::debug("Destroying multisampling color target image.");

        if (VK_NULL_HANDLE != msaa_target_buffer.color.image) {
            vmaDestroyImage(vma_allocator, msaa_target_buffer.color.image, msaa_target_buffer.color.allocation);
            msaa_target_buffer.color.allocation = VK_NULL_HANDLE;
        }

        spdlog::debug("Destroying multisampling depth target image view.");

        if (VK_NULL_HANDLE != msaa_target_buffer.depth.image_view) {
            vkDestroyImageView(device, msaa_target_buffer.depth.image_view, nullptr);
            msaa_target_buffer.depth.image_view = VK_NULL_HANDLE;
        }

        spdlog::debug("Destroying multisampling depth target image.");

        if (VK_NULL_HANDLE != msaa_target_buffer.depth.image) {
            vmaDestroyImage(vma_allocator, msaa_target_buffer.depth.image, msaa_target_buffer.depth.allocation);
            msaa_target_buffer.depth.allocation = VK_NULL_HANDLE;
        }
    }

    // TODO: Create methods for destroying resources as well!
    spdlog::debug("Destroying frame buffer.");

    if (frame_buffers.size() > 0) {
        for (auto frame_buffer : frame_buffers) {
            if (VK_NULL_HANDLE != frame_buffer) {
                vkDestroyFramebuffer(device, frame_buffer, nullptr);
                frame_buffer = VK_NULL_HANDLE;
            }
        }

        frame_buffers.clear();
    }

    result = create_depth_buffer();
    vulkan_error_check(result);

    result = create_frame_buffers();
    vulkan_error_check(result);

    vkDeviceWaitIdle(device);

    // Calculate the new aspect ratio so we can update game camera matrices.
    float aspect_ratio = window_width / static_cast<float>(window_height);

    spdlog::debug("New aspect ratio: {}.", aspect_ratio);

    vkDeviceWaitIdle(device);

    game_camera_1.set_aspect_ratio(aspect_ratio);
    game_camera_1.update_matrices();

    result = record_command_buffers();
    vulkan_error_check(result);

    vkDeviceWaitIdle(device);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_descriptor_pool() {
    std::vector<VkDescriptorPoolSize> pool_sizes = {};

    pool_sizes.resize(2);

    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = number_of_images_in_swapchain;

    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = number_of_images_in_swapchain;

    // Create the descriptor pool first.
    VkResult result = descriptor_manager->create_descriptor_pool("global_descriptor_pool", pool_sizes, global_descriptor_pool);
    vulkan_error_check(result);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_descriptor_set_layouts() {
    std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layouts;
    descriptor_set_layouts.resize(2);

    descriptor_set_layouts[0].binding = 0;
    descriptor_set_layouts[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_set_layouts[0].descriptorCount = 1;
    descriptor_set_layouts[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    descriptor_set_layouts[0].pImmutableSamplers = nullptr;

    descriptor_set_layouts[1].binding = 1;
    descriptor_set_layouts[1].descriptorCount = 1;
    descriptor_set_layouts[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_set_layouts[1].pImmutableSamplers = nullptr;
    descriptor_set_layouts[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkResult result;

    for (const auto &descriptor_set_layout : descriptor_set_layouts) {
        result = descriptor_manager->add_descriptor_set_layout_binding(descriptor_bundles.scene, descriptor_set_layout);
        vulkan_error_check(result);
    }

    result = descriptor_manager->create_descriptor_set_layouts(descriptor_bundles.scene);
    vulkan_error_check(result);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_descriptor_writes() {
    std::array<VkWriteDescriptorSet, 2> descriptor_writes = {};

    uniform_buffer_info.buffer = matrices->buffer;
    uniform_buffer_info.offset = 0;
    uniform_buffer_info.range = sizeof(UniformBufferObject);

    descriptor_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[0].dstSet = 0; // This will be overwritten automatically by descriptor_set_builder.
    descriptor_writes[0].dstBinding = 0;
    descriptor_writes[0].dstArrayElement = 0;
    descriptor_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[0].descriptorCount = 1;
    descriptor_writes[0].pBufferInfo = &uniform_buffer_info;

    VkResult result = descriptor_manager->add_write_descriptor_set(descriptor_bundles.scene, descriptor_writes[0]);
    vulkan_error_check(result);

    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    image_info.imageView = texture_manager->get_texture_view("example_texture_1").value();
    image_info.sampler = texture_manager->get_texture_sampler("example_texture_1").value();

    descriptor_writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[1].dstSet = 0; // This will be overwritten automatically by descriptor_set_builder.
    descriptor_writes[1].dstBinding = 1;
    descriptor_writes[1].dstArrayElement = 0;
    descriptor_writes[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_writes[1].descriptorCount = 1;
    descriptor_writes[1].pImageInfo = &image_info;

    result = descriptor_manager->add_write_descriptor_set(descriptor_bundles.scene, descriptor_writes[1]);
    vulkan_error_check(result);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_descriptor_sets() {
    VkResult result = descriptor_manager->create_descriptor_sets(descriptor_bundles.scene);
    vulkan_error_check(result);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_uniform_buffers() {
    spdlog::debug("Creating uniform buffers.");

    // The uniform buffer for the world matrices.
    VkDeviceSize matrices_buffer_size = sizeof(UniformBufferObject);

    VkResult result = uniform_buffer_manager->create_uniform_buffer("matrices", matrices_buffer_size, matrices);
    vulkan_error_check(result);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_pipeline() {
    // TODO: Support multisampling!
    // TODO: VulkanPipelineManager!
    assert(device);
    assert(debug_marker_manager);

    spdlog::debug("Creating graphics pipeline.");

    shader_stages.clear();

    // Loop through all shaders in Vulkan shader manager's list and add them to the setup.
    auto list_of_shaders = shader_manager->get_all_shaders();

    assert(!list_of_shaders.empty());

    spdlog::debug("Setting up shader stages.");

    for (const auto &shader : list_of_shaders) {
        VkPipelineShaderStageCreateInfo shader_stage_create_info = {};

        shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stage_create_info.pNext = nullptr;
        shader_stage_create_info.flags = 0;
        shader_stage_create_info.stage = shader->type;
        shader_stage_create_info.module = shader->module;
        shader_stage_create_info.pName = shader->entry_name.c_str();
        shader_stage_create_info.pSpecializationInfo = nullptr;

        shader_stages.push_back(shader_stage_create_info);
    }

    VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {};

    auto vertex_binding_description = gltf_model::InexorModelVertex::get_vertex_binding_description();
    auto attribute_binding_description = gltf_model::InexorModelVertex::get_attribute_binding_description();

    vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_create_info.pNext = nullptr;
    vertex_input_create_info.flags = 0;
    vertex_input_create_info.vertexBindingDescriptionCount = 1;
    vertex_input_create_info.pVertexBindingDescriptions = &vertex_binding_description;
    vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_binding_description.size());
    vertex_input_create_info.pVertexAttributeDescriptions = attribute_binding_description.data();

    VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};

    input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_create_info.pNext = nullptr;
    input_assembly_create_info.flags = 0;
    input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

    VkViewport view_port = {};

    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = static_cast<float>(window_width);
    view_port.height = static_cast<float>(window_height);
    view_port.minDepth = 0.0f;
    view_port.maxDepth = 1.0f;

    VkRect2D scissor = {};

    scissor.offset = {0, 0};
    scissor.extent = {window_width, window_height};

    // TODO: Multiple viewports (and scissors) - InexorViewportManager and InexorRenderSceneManager
    VkPipelineViewportStateCreateInfo pipeline_viewport_viewport_state_info = {};

    pipeline_viewport_viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    pipeline_viewport_viewport_state_info.pNext = nullptr;
    pipeline_viewport_viewport_state_info.flags = 0;
    pipeline_viewport_viewport_state_info.viewportCount = 1;
    pipeline_viewport_viewport_state_info.pViewports = &view_port;
    pipeline_viewport_viewport_state_info.scissorCount = 1;
    pipeline_viewport_viewport_state_info.pScissors = &scissor;

    VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info = {};

    pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    pipeline_rasterization_state_create_info.pNext = nullptr;
    pipeline_rasterization_state_create_info.flags = 0;
    pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
    pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;

    // TODO: Implement -wireframe command line argument.
    // Because the pipeline in Vulkan is immutable, this guides us to record a second command line with wireframe enabled.
    pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
    pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
    pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
    pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
    pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
    pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
    pipeline_rasterization_state_create_info.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo multisample_create_info = {};

    multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.pNext = nullptr;
    multisample_create_info.flags = 0;
    multisample_create_info.sampleShadingEnable = VK_FALSE;
    multisample_create_info.minSampleShading = 1.0f;
    multisample_create_info.pSampleMask = nullptr;
    multisample_create_info.alphaToCoverageEnable = VK_FALSE;
    multisample_create_info.alphaToOneEnable = VK_FALSE;

    if (multisampling_enabled) {
        multisample_create_info.rasterizationSamples = multisampling_sample_count;
    } else {
        multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }

    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};

    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState color_blend_attachment = {};

    // TODO: Do we need this yet?
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};

    color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.pNext = nullptr;
    color_blend_state_create_info.flags = 0;
    color_blend_state_create_info.logicOpEnable = VK_FALSE;
    color_blend_state_create_info.logicOp = VK_LOGIC_OP_NO_OP;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments = &color_blend_attachment;
    color_blend_state_create_info.blendConstants[0] = 0.0f;
    color_blend_state_create_info.blendConstants[1] = 0.0f;
    color_blend_state_create_info.blendConstants[2] = 0.0f;
    color_blend_state_create_info.blendConstants[3] = 0.0f;

    const std::vector<VkDescriptorSetLayout> set_layouts = {
        descriptor_bundles.scene->descriptor_set_layout,
        // descriptor_bundles.material->descriptor_set_layout,
        // descriptor_bundles.node->descriptor_set_layout,
    };

    VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};

    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pNext = nullptr;
    pipeline_layout_create_info.flags = 0;
    pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(set_layouts.size());
    pipeline_layout_create_info.pSetLayouts = set_layouts.data();
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;

    spdlog::debug("Setting up pipeline layout.");

    VkResult result = vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout);
    if (VK_SUCCESS != result)
        return result;

    // Use Vulkan debug markers to assign an appropriate name to this pipeline.
    debug_marker_manager->set_object_name(device, (uint64_t)(pipeline_layout), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT,
                                          "Pipeline layout for core engine.");

    if (multisampling_enabled) {
        spdlog::debug("Multisampling is enabled.");

        //
        std::array<VkAttachmentDescription, 4> attachments = {};

        // Multisampled attachment that we render to
        attachments[0].format = selected_image_format;
        attachments[0].samples = multisampling_sample_count;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // This is the frame buffer attachment to where the multisampled image
        // will be resolved to and which will be presented to the swapchain.
        attachments[1].format = selected_image_format;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Multisampled depth attachment we render to
        attachments[2].format = depth_buffer.format;
        attachments[2].samples = multisampling_sample_count;
        attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[2].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Depth resolve attachment
        attachments[3].format = depth_buffer.format;
        attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_reference = {};

        color_reference.attachment = 0;
        color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_reference = {};

        depth_reference.attachment = 2;
        depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        // Resolve attachment reference for the color attachment.
        VkAttachmentReference resolve_reference = {};

        resolve_reference.attachment = 1;
        resolve_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};

        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_reference;
        subpass.pResolveAttachments = &resolve_reference;
        subpass.pDepthStencilAttachment = &depth_reference;

        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderpass_create_info = {};

        renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderpass_create_info.pAttachments = attachments.data();
        renderpass_create_info.subpassCount = 1;
        renderpass_create_info.pSubpasses = &subpass;
        renderpass_create_info.dependencyCount = 2;
        renderpass_create_info.pDependencies = dependencies.data();

        spdlog::debug("Creating renderpass.");

        result = vkCreateRenderPass(device, &renderpass_create_info, nullptr, &render_pass);
        vulkan_error_check(result);
    } else {
        spdlog::debug("Multisampling is disabled.");

        std::array<VkAttachmentDescription, 2> attachments = {};

        // Color attachment.
        attachments[0].format = selected_image_format;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        // Depth attachment.
        attachments[1].format = depth_buffer.format;
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_reference = {};

        color_reference.attachment = 0;
        color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_reference = {};

        depth_reference.attachment = 1;
        depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpassDescription = {};

        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &color_reference;
        subpassDescription.pDepthStencilAttachment = &depth_reference;
        subpassDescription.inputAttachmentCount = 0;
        subpassDescription.pInputAttachments = nullptr;
        subpassDescription.preserveAttachmentCount = 0;
        subpassDescription.pPreserveAttachments = nullptr;
        subpassDescription.pResolveAttachments = nullptr;

        // Subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderpass_create_info{};

        renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderpass_create_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderpass_create_info.pAttachments = attachments.data();
        renderpass_create_info.subpassCount = 1;
        renderpass_create_info.pSubpasses = &subpassDescription;
        renderpass_create_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderpass_create_info.pDependencies = dependencies.data();

        spdlog::debug("Creating renderpass.");

        result = vkCreateRenderPass(device, &renderpass_create_info, nullptr, &render_pass);
        vulkan_error_check(result);
    }

    // Use Vulkan debug markers to assign an appropriate name to this renderpass.
    debug_marker_manager->set_object_name(device, (uint64_t)(render_pass), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, "Render pass for core engine.");

    // Tell Vulkan that we want to change viewport and scissor during runtime so it's a dynamic state.
    const std::vector<VkDynamicState> enabled_dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR

    };

    // TODO: Wrap all this into InexorRenderingPipelineManager instead of loading from TOML file?
    // InexorRenderingPipelineManager could verify if all structures were filled correctly.

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};

    dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.pDynamicStates = enabled_dynamic_states.data();
    dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(enabled_dynamic_states.size());

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};

    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    graphics_pipeline_create_info.pNext = nullptr;
    graphics_pipeline_create_info.flags = 0;
    graphics_pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
    graphics_pipeline_create_info.pStages = shader_stages.data();
    graphics_pipeline_create_info.pVertexInputState = &vertex_input_create_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
    graphics_pipeline_create_info.pTessellationState = nullptr;
    graphics_pipeline_create_info.pViewportState = &pipeline_viewport_viewport_state_info;
    graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
    graphics_pipeline_create_info.pMultisampleState = &multisample_create_info;
    graphics_pipeline_create_info.pDepthStencilState = &depth_stencil;
    graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
    graphics_pipeline_create_info.pDynamicState = &dynamic_state_create_info;
    graphics_pipeline_create_info.layout = pipeline_layout;
    graphics_pipeline_create_info.renderPass = render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
    graphics_pipeline_create_info.basePipelineIndex = -1;

    spdlog::debug("Creating pipeline cache.");

    VkPipelineCacheCreateInfo pipeline_cache_create_info = {};

    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    /// Vulkan Spec:
    /// "Pipeline cache objects allow the result of pipeline construction to be reused between pipelines and between runs of an application.
    /// Reuse between pipelines is achieved by passing the same pipeline cache object when creating multiple related pipelines. Reuse across
    /// runs of an application is achieved by retrieving pipeline cache contents in one run of an application, saving the contents, and using
    /// them to preinitialize a pipeline cache on a subsequent run. The contents of the pipeline cache objects are managed by the implementation.
    /// Applications can manage the host memory consumed by a pipeline cache object and control the amount of data retrieved from a pipeline cache object."
    result = vkCreatePipelineCache(device, &pipeline_cache_create_info, nullptr, &pipeline_cache);
    vulkan_error_check(result);

    spdlog::debug("Finalizing graphics pipeline.");

    // TODO: Create multiple pipelines at once?
    result = vkCreateGraphicsPipelines(device, pipeline_cache, 1, &graphics_pipeline_create_info, nullptr, &pipeline);
    if (VK_SUCCESS != result)
        return result;

    // Use Vulkan debug markers to assign an appropriate name to this pipeline.
    debug_marker_manager->set_object_name(device, (uint64_t)(pipeline), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, "Graphics pipeline for core engine.");

    // TODO: We could destroy shader modules here already.
    // TODO: Create alpha blend pipeline.
    // TODO: Create PBR pipeline.

    return VK_SUCCESS;
}

VkResult VulkanRenderer::create_frame_buffers() {
    assert(device);
    assert(window_width > 0);
    assert(window_height > 0);
    assert(debug_marker_manager);
    assert(number_of_images_in_swapchain > 0);

    VkResult result;

    // MSAA setup.
    // TODO: Pack all this to InexorMultisamplingManager?
    if (multisampling_enabled) {
        // Check if device supports requested sample count for color and depth frame buffer
        // assert((deviceProperties.limits.framebufferColorSampleCounts >= sampleCount) && (deviceProperties.limits.framebufferDepthSampleCounts >=
        // sampleCount));

        VkImageCreateInfo image_create_info = {};

        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = selected_image_format;
        image_create_info.extent.width = window_width;
        image_create_info.extent.height = window_height;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.samples = multisampling_sample_count;
        image_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        msaa_target_buffer.color.allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        msaa_target_buffer.color.allocation_create_info.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;

        std::string msaa_target_color_image_name = "MSAA target color image.";

        msaa_target_buffer.color.allocation_create_info.pUserData = msaa_target_color_image_name.data();

        result = vmaCreateImage(vma_allocator, &image_create_info, &msaa_target_buffer.color.allocation_create_info, &msaa_target_buffer.color.image,
                                &msaa_target_buffer.color.allocation, &msaa_target_buffer.color.allocation_info);
        vulkan_error_check(result);

        // Create image view for the MSAA target
        VkImageViewCreateInfo image_view_create_info = {};

        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = msaa_target_buffer.color.image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = selected_image_format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;

        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.layerCount = 1;

        result = vkCreateImageView(device, &image_view_create_info, nullptr, &msaa_target_buffer.color.image_view);
        vulkan_error_check(result);

        // Depth target.
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = depth_buffer.format;
        image_create_info.extent.width = window_width;
        image_create_info.extent.height = window_height;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_create_info.samples = multisampling_sample_count;
        image_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        msaa_target_buffer.depth.allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        msaa_target_buffer.depth.allocation_create_info.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;

        std::string msaa_target_depth_image = "MSAA target depth image.";

        msaa_target_buffer.depth.allocation_create_info.pUserData = msaa_target_depth_image.data();

        result = vmaCreateImage(vma_allocator, &image_create_info, &msaa_target_buffer.depth.allocation_create_info, &msaa_target_buffer.depth.image,
                                &msaa_target_buffer.depth.allocation, &msaa_target_buffer.depth.allocation_info);
        vulkan_error_check(result);

        // Create image view for the MSAA target.
        image_view_create_info.image = msaa_target_buffer.depth.image;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = depth_buffer.format;
        image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
        image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
        image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
        image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;

        image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        image_view_create_info.subresourceRange.levelCount = 1;
        image_view_create_info.subresourceRange.layerCount = 1;

        result = vkCreateImageView(device, &image_view_create_info, nullptr, &msaa_target_buffer.depth.image_view);
        vulkan_error_check(result);
    }

    // Depth/Stencil attachment is the same for all frame buffers.
    VkImageCreateInfo image_create_info = {};

    image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.pNext = nullptr;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = depth_buffer.format;
    image_create_info.extent.width = window_width;
    image_create_info.extent.height = window_height;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = 0;

    depth_stencil.allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_COPY;
    depth_stencil.allocation_create_info.flags = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;

    std::string depth_stencil_image_name = "Depth stencil image.";

    depth_stencil.allocation_create_info.pUserData = depth_stencil_image_name.data();

    result = vmaCreateImage(vma_allocator, &image_create_info, &depth_stencil.allocation_create_info, &depth_stencil.image, &depth_stencil.allocation,
                            &depth_stencil.allocation_info);
    vulkan_error_check(result);

    VkImageViewCreateInfo depth_stencil_view_create_info = {};

    depth_stencil_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depth_stencil_view_create_info.pNext = nullptr;
    depth_stencil_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depth_stencil_view_create_info.format = depth_buffer.format;
    depth_stencil_view_create_info.flags = 0;
    depth_stencil_view_create_info.subresourceRange = {};
    depth_stencil_view_create_info.image = depth_stencil.image;
    depth_stencil_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depth_stencil_view_create_info.subresourceRange.baseMipLevel = 0;
    depth_stencil_view_create_info.subresourceRange.levelCount = 1;
    depth_stencil_view_create_info.subresourceRange.baseArrayLayer = 0;
    depth_stencil_view_create_info.subresourceRange.layerCount = 1;

    result = vkCreateImageView(device, &depth_stencil_view_create_info, nullptr, &depth_stencil.image_view);
    vulkan_error_check(result);

    VkImageView attachments[4];

    if (multisampling_enabled) {
        attachments[0] = msaa_target_buffer.color.image_view;
        attachments[2] = msaa_target_buffer.depth.image_view;
        attachments[3] = depth_stencil.image_view;
    } else {
        attachments[1] = depth_stencil.image_view;
    }

    VkFramebufferCreateInfo frame_buffer_create_info = {};

    frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frame_buffer_create_info.pNext = nullptr;
    frame_buffer_create_info.renderPass = render_pass;
    frame_buffer_create_info.attachmentCount = multisampling_enabled ? 4 : 2;
    frame_buffer_create_info.pAttachments = attachments;
    frame_buffer_create_info.width = window_width;
    frame_buffer_create_info.height = window_height;
    frame_buffer_create_info.layers = 1;

    spdlog::debug("Creating frame buffers.");
    spdlog::debug("Number of images in swapchain: {}.", number_of_images_in_swapchain);

    frame_buffers.clear();
    frame_buffers.resize(number_of_images_in_swapchain);

    // Create frame buffers for every swap chain image.
    for (std::size_t i = 0; i < number_of_images_in_swapchain; i++) {
        spdlog::debug("Creating framebuffer #{}.", i);

        if (multisampling_enabled) {
            attachments[1] = swapchain_image_views[i];
        } else {
            attachments[0] = swapchain_image_views[i];
        }

        VkResult result = vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &frame_buffers[i]);
        vulkan_error_check(result);

        std::string frame_buffer_name = "Frame buffer #" + std::to_string(i) + ".";

        // Use Vulkan debug markers to assign an appropriate name to this frame buffer.
        debug_marker_manager->set_object_name(device, (uint64_t)(frame_buffers[i]), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, frame_buffer_name.c_str());
    }

    return VK_SUCCESS;
}

VkResult VulkanRenderer::calculate_memory_budget() {
    VmaStats memory_stats;

    spdlog::debug("------------------------------------------------------------------------------------------------------------");
    spdlog::debug("Calculating memory statistics before shutdown.");

    // Use Vulkan memory allocator's statistics.
    vmaCalculateStats(vma_allocator, &memory_stats);

    spdlog::debug("VMA heap:");

    spdlog::debug("Number of `VkDeviceMemory` Vulkan memory blocks allocated: {}", memory_stats.memoryHeap->blockCount);
    spdlog::debug("Number of #VmaAllocation allocation objects allocated: {}", memory_stats.memoryHeap->allocationCount);
    spdlog::debug("Number of free ranges of memory between allocations: {}", memory_stats.memoryHeap->unusedRangeCount);
    spdlog::debug("Total number of bytes occupied by all allocations: {}", memory_stats.memoryHeap->usedBytes);
    spdlog::debug("Total number of bytes occupied by unused ranges: {}", memory_stats.memoryHeap->unusedBytes);
    spdlog::debug("memory_stats.memoryHeap->allocationSizeMin: {}", memory_stats.memoryHeap->allocationSizeMin);
    spdlog::debug("memory_stats.memoryHeap->allocationSizeAvg: {}", memory_stats.memoryHeap->allocationSizeAvg);
    spdlog::debug("memory_stats.memoryHeap->allocationSizeMax: {}", memory_stats.memoryHeap->allocationSizeMax);
    spdlog::debug("memory_stats.memoryHeap->unusedRangeSizeMin: {}", memory_stats.memoryHeap->unusedRangeSizeMin);
    spdlog::debug("memory_stats.memoryHeap->unusedRangeSizeAvg: {}", memory_stats.memoryHeap->unusedRangeSizeAvg);
    spdlog::debug("memory_stats.memoryHeap->unusedRangeSizeMax: {}", memory_stats.memoryHeap->unusedRangeSizeMax);

    spdlog::debug("VMA memory type:");

    spdlog::debug("Number of `VkDeviceMemory` Vulkan memory blocks allocated: {}", memory_stats.memoryType->blockCount);
    spdlog::debug("Number of #VmaAllocation allocation objects allocated: {}", memory_stats.memoryType->allocationCount);
    spdlog::debug("Number of free ranges of memory between allocations: {}", memory_stats.memoryType->unusedRangeCount);
    spdlog::debug("Total number of bytes occupied by all allocations: {}", memory_stats.memoryType->usedBytes);
    spdlog::debug("Total number of bytes occupied by unused ranges: {}", memory_stats.memoryType->unusedBytes);
    spdlog::debug("memory_stats.memoryType->allocationSizeMin: {}", memory_stats.memoryType->allocationSizeMin);
    spdlog::debug("memory_stats.memoryType->allocationSizeAvg: {}", memory_stats.memoryType->allocationSizeAvg);
    spdlog::debug("memory_stats.memoryType->allocationSizeMax: {}", memory_stats.memoryType->allocationSizeMax);
    spdlog::debug("memory_stats.memoryType->unusedRangeSizeMin: {}", memory_stats.memoryType->unusedRangeSizeMin);
    spdlog::debug("memory_stats.memoryType->unusedRangeSizeAvg: {}", memory_stats.memoryType->unusedRangeSizeAvg);
    spdlog::debug("memory_stats.memoryType->unusedRangeSizeMax: {}", memory_stats.memoryType->unusedRangeSizeMax);

    spdlog::debug("VMA total:");

    spdlog::debug("Number of `VkDeviceMemory` Vulkan memory blocks allocated: {}", memory_stats.total.blockCount);
    spdlog::debug("Number of #VmaAllocation allocation objects allocated: {}", memory_stats.total.allocationCount);
    spdlog::debug("Number of free ranges of memory between allocations: {}", memory_stats.total.unusedRangeCount);
    spdlog::debug("Total number of bytes occupied by all allocations: {}", memory_stats.total.usedBytes);
    spdlog::debug("Total number of bytes occupied by unused ranges: {}", memory_stats.total.unusedBytes);
    spdlog::debug("memory_stats.total.allocationSizeMin: {}", memory_stats.total.allocationSizeMin);
    spdlog::debug("memory_stats.total.allocationSizeAvg: {}", memory_stats.total.allocationSizeAvg);
    spdlog::debug("memory_stats.total.allocationSizeMax: {}", memory_stats.total.allocationSizeMax);
    spdlog::debug("memory_stats.total.unusedRangeSizeMin: {}", memory_stats.total.unusedRangeSizeMin);
    spdlog::debug("memory_stats.total.unusedRangeSizeAvg: {}", memory_stats.total.unusedRangeSizeAvg);
    spdlog::debug("memory_stats.total.unusedRangeSizeMax: {}", memory_stats.total.unusedRangeSizeMax);

    char *vma_stats_string = nullptr;

    vmaBuildStatsString(vma_allocator, &vma_stats_string, true);

    std::ofstream vma_memory_dump;

    std::string memory_dump_file_name = "vma-dumps/inexor_VMA_dump_" + std::to_string(vma_dump_index) + ".json";

    vma_memory_dump.open(memory_dump_file_name, std::ios::out);

    vma_memory_dump.write(vma_stats_string, strlen(vma_stats_string));

    vma_memory_dump.close();

    vma_dump_index++;

    vmaFreeStatsString(vma_allocator, vma_stats_string);

    return VK_SUCCESS;
}

VkResult VulkanRenderer::shutdown_vulkan() {
    // It is important to destroy the objects in reversal of the order of creation.
    spdlog::debug("------------------------------------------------------------------------------------------------------------");
    spdlog::debug("Shutting down Vulkan API.");

    cleanup_swapchain();

    spdlog::debug("Destroying swapchain images.");
    if (swapchain_images.size() > 0) {
        for (auto image : swapchain_images) {
            if (VK_NULL_HANDLE != image) {
                vkDestroyImage(device, image, nullptr);
                image = VK_NULL_HANDLE;
            }
        }

        swapchain_images.clear();
    }

    spdlog::debug("Destroying textures.");
    texture_manager->shutdown_textures();

    spdlog::debug("Destroying descriptor set layout.");
    descriptor_manager->shutdown_descriptors(true);

    spdlog::debug("Destroying vertex buffers.");
    mesh_buffer_manager->shutdown_vertex_and_index_buffers();

    spdlog::debug("Destroying semaphores.");
    semaphore_manager->shutdown_semaphores();

    spdlog::debug("Destroying fences.");
    fence_manager->shutdown_fences();

    spdlog::debug("Destroying Vulkan shader objects.");
    shader_manager->shutdown_shaders();

    spdlog::debug("Destroying window surface.");
    if (VK_NULL_HANDLE != surface) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }

    vmaDestroyAllocator(vma_allocator);

    spdlog::debug("Destroying Vulkan pipeline cache.");

    if (VK_NULL_HANDLE != pipeline_cache) {
        vkDestroyPipelineCache(device, pipeline_cache, nullptr);
        pipeline_cache = VK_NULL_HANDLE;
    }

    spdlog::debug("Destroying Vulkan command pool.");
    if (VK_NULL_HANDLE != command_pool) {
        vkDestroyCommandPool(device, command_pool, nullptr);
        command_pool = VK_NULL_HANDLE;
    }

    // Device queues are implicitly cleaned up when the device is destroyed,
    // so we don’t need to do anything in cleanup.
    spdlog::debug("Destroying Vulkan device.");
    if (VK_NULL_HANDLE != device) {
        vkDestroyDevice(device, nullptr);
        device = VK_NULL_HANDLE;
    }

    descriptor_manager->shutdown_descriptors();

    // Destroy Vulkan debug callback.
    if (debug_report_callback_initialised) {
        // We have to explicitly load this function.
        PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
            reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

        if (nullptr != vkDestroyDebugReportCallbackEXT) {
            vkDestroyDebugReportCallbackEXT(instance, debug_report_callback, nullptr);
            debug_report_callback_initialised = false;
        }
    }

    spdlog::debug("Destroying Vulkan instance.");
    if (VK_NULL_HANDLE != instance) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }

    spdlog::debug("Shutdown finished.");
    spdlog::debug("------------------------------------------------------------------------------------------------------------");

    images_in_flight.clear();
    in_flight_fences.clear();
    image_available_semaphores.clear();
    rendering_finished_semaphores.clear();

    return VK_SUCCESS;
}

} // namespace inexor::vulkan_renderer
