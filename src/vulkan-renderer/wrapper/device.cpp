#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#define VMA_IMPLEMENTATION

// It makes memory of all new allocations initialized to bit pattern 0xDCDCDCDC.
// Before an allocation is destroyed, its memory is filled with bit pattern 0xEFEFEFEF.
// Memory is automatically mapped and unmapped if necessary.
#define VMA_DEBUG_INITIALIZE_ALLOCATIONS 1 // NOLINT

// Enforce specified number of bytes as a margin before and after every allocation.
#define VMA_DEBUG_MARGIN 16 // NOLINT

// Enable validation of contents of the margins.
#define VMA_DEBUG_DETECT_CORRUPTION 1 // NOLINT

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#include <algorithm>
#include <fstream>

namespace {

// TODO: Make proper use of queue priorities in the future.
constexpr float DEFAULT_QUEUE_PRIORITY = 1.0f;

} // namespace

namespace inexor::vulkan_renderer::wrapper {

bool Device::is_extension_supported(const VkPhysicalDevice graphics_card, const std::string &extension_name) {
    assert(graphics_card);
    assert(!extension_name.empty());

    std::uint32_t device_extension_count = 0;

    // Query how many device extensions are available.
    if (const auto result =
            vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &device_extension_count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }

    if (device_extension_count == 0) {
        // This is not an error. Some platforms simply don't have any device extensions.
        spdlog::info("No Vulkan device extensions available!");
        return false;
    }

    std::vector<VkExtensionProperties> device_extensions(device_extension_count);

    // Store all available device extensions.
    if (const auto result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &device_extension_count,
                                                                 device_extensions.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }

    // Search for the requested device extension.
    return std::find_if(device_extensions.begin(), device_extensions.end(),
                        [&](const VkExtensionProperties device_extension) {
                            return device_extension.extensionName == extension_name;
                        }) != device_extensions.end();
}

bool Device::is_swapchain_supported(const VkPhysicalDevice graphics_card) {
    assert(graphics_card);
    return is_extension_supported(graphics_card, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

bool Device::is_presentation_supported(const VkPhysicalDevice graphics_card, const VkSurfaceKHR surface) {
    assert(graphics_card);
    assert(surface);

    VkBool32 presentation_supported = VK_FALSE;

    // Query if presentation is supported.
    if (const auto result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, 0, surface, &presentation_supported);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceSupportKHR failed!", result);
    }

    return presentation_supported == VK_TRUE;
}

Device::Device(const wrapper::Instance &instance, const VkSurfaceKHR surface, bool enable_vulkan_debug_markers,
               bool prefer_distinct_transfer_queue, const std::optional<std::uint32_t> preferred_physical_device_index)
    : m_surface(surface), m_enable_vulkan_debug_markers(enable_vulkan_debug_markers) {

    const auto selected_gpu =
        VulkanSettingsDecisionMaker::graphics_card(instance.instance(), surface, preferred_physical_device_index);

    if (!selected_gpu) {
        throw std::runtime_error("Error: Could not find suitable graphics card!");
    }

    m_graphics_card = *selected_gpu;

    VkPhysicalDeviceProperties graphics_card_properties;

    // Get the information about that graphics card's properties.
    vkGetPhysicalDeviceProperties(m_graphics_card, &graphics_card_properties);

    spdlog::trace("Creating device using graphics card: {}.", graphics_card_properties.deviceName);

    m_gpu_name = graphics_card_properties.deviceName;

    spdlog::trace("Creating Vulkan device queues.");

    std::vector<VkDeviceQueueCreateInfo> queues_to_create;

    if (prefer_distinct_transfer_queue) {
        spdlog::trace("The application will try to use a distinct data transfer queue if it is available.");
    } else {
        spdlog::warn("The application is forced not to use a distinct data transfer queue!");
    }

    // Check if there is one queue family which can be used for both graphics and presentation.
    std::optional<std::uint32_t> queue_family_index_for_both_graphics_and_presentation =
        VulkanSettingsDecisionMaker::find_queue_family_for_both_graphics_and_presentation(m_graphics_card, surface);

    if (queue_family_index_for_both_graphics_and_presentation) {
        spdlog::trace("One queue for both graphics and presentation will be used.");

        m_graphics_queue_family_index = *queue_family_index_for_both_graphics_and_presentation;
        m_present_queue_family_index = m_graphics_queue_family_index;

        // In this case, there is one queue family which can be used for both graphics and presentation.
        auto device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = *queue_family_index_for_both_graphics_and_presentation;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY;

        queues_to_create.push_back(device_queue_ci);
    } else {
        spdlog::trace("No queue found which supports both graphics and presentation.");
        spdlog::trace("The application will try to use 2 separate queues.");

        // We have to use 2 different queue families.
        // One for graphics and another one for presentation.

        // Check which queue family index can be used for graphics.
        auto queue_candidate = VulkanSettingsDecisionMaker::find_graphics_queue_family(m_graphics_card);

        if (!queue_candidate) {
            throw std::runtime_error("Could not find suitable queue family indices for graphics!");
        }

        m_graphics_queue_family_index = *queue_candidate;

        // Check which queue family index can be used for presentation.
        queue_candidate = VulkanSettingsDecisionMaker::find_presentation_queue_family(m_graphics_card, surface);

        if (!queue_candidate) {
            throw std::runtime_error("Could not find suitable queue family indices for presentation!");
        }

        m_present_queue_family_index = *queue_candidate;

        spdlog::trace("Graphics queue family index: {}.", m_graphics_queue_family_index);
        spdlog::trace("Presentation queue family index: {}.", m_present_queue_family_index);

        // Set up one queue for graphics.
        auto device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = m_graphics_queue_family_index;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY;

        queues_to_create.push_back(device_queue_ci);

        // Set up one queue for presentation.
        device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = m_present_queue_family_index;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY;

        queues_to_create.push_back(device_queue_ci);
    }

    // Add another device queue just for data transfer.
    const auto queue_candidate = VulkanSettingsDecisionMaker::find_distinct_data_transfer_queue_family(m_graphics_card);

    bool use_distinct_data_transfer_queue = false;

    if (queue_candidate && prefer_distinct_transfer_queue) {
        m_transfer_queue_family_index = *queue_candidate;

        spdlog::trace("A separate queue will be used for data transfer.");
        spdlog::trace("Data transfer queue family index: {}.", m_transfer_queue_family_index);

        // We have the opportunity to use a separated queue for data transfer!
        use_distinct_data_transfer_queue = true;

        auto device_queue_ci = make_info<VkDeviceQueueCreateInfo>();
        device_queue_ci.queueFamilyIndex = m_transfer_queue_family_index;
        device_queue_ci.queueCount = 1;
        device_queue_ci.pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY;

        queues_to_create.push_back(device_queue_ci);
    } else {
        // We don't have the opportunity to use a separated queue for data transfer!
        // Do not create a new queue, use the graphics queue instead.
        use_distinct_data_transfer_queue = false;
    }

    if (!use_distinct_data_transfer_queue) {
        spdlog::warn("The application is forces to avoid distinct data transfer queues.");
        spdlog::warn("Because of this, the graphics queue will be used for data transfer.");

        m_transfer_queue_family_index = m_graphics_queue_family_index;
    }

    std::vector<const char *> device_extensions_wishlist = {
        // Since we want to draw on a window, we need the swapchain extension.
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };

#ifndef NDEBUG
    if (enable_vulkan_debug_markers) {
        // Debug markers allow the assignment of internal names to Vulkan resources.
        // These internal names will conveniently be visible in debuggers like RenderDoc.
        // Debug markers are only available if RenderDoc is enabled.
        device_extensions_wishlist.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
#endif

    std::vector<const char *> enabled_device_extensions;

    for (const auto &device_extension_name : device_extensions_wishlist) {
        if (is_extension_supported(m_graphics_card, device_extension_name)) {
            spdlog::debug("Device extension '{}' is available on this system.", device_extension_name);
            enabled_device_extensions.push_back(device_extension_name);
        } else {
            throw std::runtime_error("Device extension " + std::string(device_extension_name) +
                                     " is not available on this system!");
        }
    }

    VkPhysicalDeviceFeatures used_features{};

    // Enable anisotropic filtering.
    used_features.samplerAnisotropy = VK_TRUE;

    auto device_ci = make_info<VkDeviceCreateInfo>();
    device_ci.queueCreateInfoCount = static_cast<std::uint32_t>(queues_to_create.size());
    device_ci.pQueueCreateInfos = queues_to_create.data();
    // Device layers were deprecated in Vulkan some time ago, essentially making all layers instance layers.
    device_ci.enabledLayerCount = 0;
    device_ci.ppEnabledLayerNames = nullptr;
    device_ci.enabledExtensionCount = static_cast<std::uint32_t>(enabled_device_extensions.size());
    device_ci.ppEnabledExtensionNames = enabled_device_extensions.data();
    device_ci.pEnabledFeatures = &used_features;

    spdlog::trace("Creating physical device.");

    if (const auto result = vkCreateDevice(m_graphics_card, &device_ci, nullptr, &m_device); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDevice failed!", result);
    }

#ifndef NDEBUG
    if (m_enable_vulkan_debug_markers) {
        spdlog::trace("Initializing Vulkan debug markers.");

        // The debug marker extension is not part of the core, so function pointers need to be loaded manually.
        m_vk_debug_marker_set_object_tag = reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>( // NOLINT
            vkGetDeviceProcAddr(m_device, "vkDebugMarkerSetObjectTagEXT"));
        assert(m_vk_debug_marker_set_object_tag);

        m_vk_debug_marker_set_object_name = reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>( // NOLINT
            vkGetDeviceProcAddr(m_device, "vkDebugMarkerSetObjectNameEXT"));
        assert(m_vk_debug_marker_set_object_name);

        m_vk_cmd_debug_marker_begin = reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>( // NOLINT
            vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerBeginEXT"));
        assert(m_vk_cmd_debug_marker_begin);

        m_vk_cmd_debug_marker_end = reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>( // NOLINT
            vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerEndEXT"));
        assert(m_vk_cmd_debug_marker_end);

        m_vk_cmd_debug_marker_insert = reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>( // NOLINT
            vkGetDeviceProcAddr(m_device, "vkCmdDebugMarkerInsertEXT"));
        assert(m_vk_cmd_debug_marker_insert);

        m_vk_set_debug_utils_object_name = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>( // NOLINT
            vkGetDeviceProcAddr(m_device, "vkSetDebugUtilsObjectNameEXT"));
        assert(m_vk_set_debug_utils_object_name);
    }
#endif

    spdlog::trace("Graphics queue family index: {}.", m_graphics_queue_family_index);
    spdlog::trace("Presentation queue family index: {}.", m_present_queue_family_index);
    spdlog::trace("Data transfer queue family index: {}.", m_transfer_queue_family_index);

    // Setup the queues for presentation and graphics.
    // Since we only have one queue per queue family, we acquire index 0.
    vkGetDeviceQueue(m_device, m_present_queue_family_index, 0, &m_present_queue);
    vkGetDeviceQueue(m_device, m_graphics_queue_family_index, 0, &m_graphics_queue);

    // The use of data transfer queues can be forbidden by using -no_separate_data_queue.
    if (use_distinct_data_transfer_queue) {
        // Use a separate queue for data transfer to GPU.
        vkGetDeviceQueue(m_device, m_transfer_queue_family_index, 0, &m_transfer_queue);
    }

    spdlog::trace("Creating VMA allocator.");

    // Make sure to set the root directory of this repository as working directory in the debugger!
    // Otherwise, VMA won't be able to open this allocation replay file for writing.
    const std::string vma_replay_file = "vma-replays/vma_replay.csv";
    spdlog::trace("Opening VMA memory recording file for writing.");

    std::ofstream replay_file_test(vma_replay_file, std::ios::out);

    // Check if we can open the csv file.
    // This causes problems when the debugging path is set incorrectly!
    if (!replay_file_test) {
        throw std::runtime_error("Could not open VMA replay file " + vma_replay_file + " !");
    }

    spdlog::trace("VMA memory recording file opened successfully.");

    replay_file_test.close();

    // VMA allows recording memory allocations to a .csv file.
    // This .csv file can be replayed using tools from the repository.
    // This is very useful every time there is a bug in memory management.
    VmaRecordSettings vma_record_settings;

    // We flush the stream after every write operation because we are expecting unforeseen program crashes
    // This might have a negative effect on the application's performance
    vma_record_settings.flags = VMA_RECORD_FLUSH_AFTER_CALL_BIT;
    vma_record_settings.pFilePath = vma_replay_file.c_str();

    VmaAllocatorCreateInfo vma_allocator_ci{};
    vma_allocator_ci.physicalDevice = m_graphics_card;
    vma_allocator_ci.instance = instance.instance();
    vma_allocator_ci.device = m_device;

    // Just tell Vulkan Memory Allocator to use Vulkan 1.1, even if a newer version is specified in instance wrapper
    // This might need to be changed in the future
    vma_allocator_ci.vulkanApiVersion = VK_API_VERSION_1_1;
#if VMA_RECORDING_ENABLED
    vma_allocator_ci.pRecordSettings = &vma_record_settings;
#endif

    spdlog::trace("Creating Vulkan memory allocator instance.");

    if (const auto result = vmaCreateAllocator(&vma_allocator_ci, &m_allocator); result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateAllocator failed!", result);
    }
}

Device::Device(Device &&other) noexcept : m_enable_vulkan_debug_markers(other.m_enable_vulkan_debug_markers) {
    m_device = std::exchange(other.m_device, nullptr);
    m_graphics_card = std::exchange(other.m_graphics_card, nullptr);
    m_surface = other.m_surface;
}

Device::~Device() {
    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);
}

void Device::set_debug_marker_name(void *object, VkDebugReportObjectTypeEXT object_type,
                                   const std::string &name) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(object);
    assert(!name.empty());
    assert(m_vk_debug_marker_set_object_name);

    auto name_info = make_info<VkDebugMarkerObjectNameInfoEXT>();
    name_info.objectType = object_type;
    name_info.object = reinterpret_cast<std::uint64_t>(object); // NOLINT
    name_info.pObjectName = name.c_str();

    if (const auto result = m_vk_debug_marker_set_object_name(m_device, &name_info); result != VK_SUCCESS) {
        throw VulkanException("Failed to assign Vulkan debug marker name " + name + "!", result);
    }
#endif
}

void Device::set_memory_block_attachment(void *object, VkDebugReportObjectTypeEXT object_type, const std::uint64_t name,
                                         const std::size_t memory_size, const void *memory_block) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(name);
    assert(memory_size > 0);
    assert(memory_block);
    assert(m_vk_debug_marker_set_object_tag);

    auto tag_info = make_info<VkDebugMarkerObjectTagInfoEXT>();
    tag_info.object = reinterpret_cast<std::uint64_t>(object); // NOLINT
    tag_info.objectType = object_type;
    tag_info.tagName = name;
    tag_info.tagSize = memory_size;
    tag_info.pTag = memory_block;

    if (const auto result = m_vk_debug_marker_set_object_tag(m_device, &tag_info); result != VK_SUCCESS) {
        throw VulkanException("Failed to assign Vulkan debug marker memory block!", result);
    }
#endif
}

void Device::bind_debug_region(const VkCommandBuffer command_buffer, const std::string &name,
                               const std::array<float, 4> color) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(command_buffer);
    assert(!name.empty());
    assert(m_vk_cmd_debug_marker_begin);

    auto debug_marker = make_info<VkDebugMarkerMarkerInfoEXT>();

    std::copy(color.begin(), color.end(), debug_marker.color);

    debug_marker.pMarkerName = name.c_str();

    m_vk_cmd_debug_marker_begin(command_buffer, &debug_marker);
#endif
}

void Device::insert_debug_marker(const VkCommandBuffer command_buffer, const std::string &name,
                                 const std::array<float, 4> color) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(command_buffer);
    assert(!name.empty());
    assert(m_vk_cmd_debug_marker_insert);

    auto debug_marker = make_info<VkDebugMarkerMarkerInfoEXT>();

    std::copy(color.begin(), color.end(), debug_marker.color);

    debug_marker.pMarkerName = name.c_str();

    m_vk_cmd_debug_marker_insert(command_buffer, &debug_marker);
#endif
}

void Device::end_debug_region(const VkCommandBuffer command_buffer) const {
#ifndef NDEBUG
    if (!m_enable_vulkan_debug_markers) {
        return;
    }

    assert(m_vk_cmd_debug_marker_end);
    m_vk_cmd_debug_marker_end(command_buffer);
#endif
}

void Device::create_command_pool(const VkCommandPoolCreateInfo &command_pool_ci, VkCommandPool *command_pool,
                                 const std::string &name) const {
    if (const auto result = vkCreateCommandPool(m_device, &command_pool_ci, nullptr, command_pool);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateCommandPool failed for command pool " + name + "!", result);
    }

    set_debug_marker_name(&command_pool, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, name);
}

void Device::create_descriptor_pool(const VkDescriptorPoolCreateInfo &descriptor_pool_ci,
                                    VkDescriptorPool *descriptor_pool, const std::string &name) const {
    if (const auto result = vkCreateDescriptorPool(m_device, &descriptor_pool_ci, nullptr, descriptor_pool);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDescriptorPool failed for descriptor pool " + name + " !", result);
    }

    set_debug_marker_name(&descriptor_pool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, name);
}

void Device::create_descriptor_set_layout(const VkDescriptorSetLayoutCreateInfo &descriptor_set_layout_ci,
                                          VkDescriptorSetLayout *descriptor_set_layout, const std::string &name) const {
    if (const auto result =
            vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_ci, nullptr, descriptor_set_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDescriptorSetLayout failed for descriptor " + name + " !", result);
    }

    set_debug_marker_name(&descriptor_set_layout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, name);
}

void Device::create_framebuffer(const VkFramebufferCreateInfo &framebuffer_ci, VkFramebuffer *framebuffer,
                                const std::string &name) const {
    if (const auto result = vkCreateFramebuffer(m_device, &framebuffer_ci, nullptr, framebuffer);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateFramebuffer failed for framebuffer " + name + "!", result);
    }

    set_debug_marker_name(&framebuffer, VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, name);
}

void Device::create_graphics_pipeline(const VkGraphicsPipelineCreateInfo &pipeline_ci, VkPipeline *pipeline,
                                      const std::string &name) const {
    if (const auto result = vkCreateGraphicsPipelines(m_device, nullptr, 1, &pipeline_ci, nullptr, pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateGraphicsPipelines failed for pipeline " + name + " !", result);
    }

    set_debug_marker_name(&pipeline, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name);
}

void Device::create_image_view(const VkImageViewCreateInfo &image_view_ci, VkImageView *image_view,
                               const std::string &name) const {
    if (const auto result = vkCreateImageView(m_device, &image_view_ci, nullptr, image_view); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + name + "!", result);
    }

    set_debug_marker_name(&image_view, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, name);
}

void Device::create_pipeline_layout(const VkPipelineLayoutCreateInfo &pipeline_layout_ci,
                                    VkPipelineLayout *pipeline_layout, const std::string &name) const {
    if (const auto result = vkCreatePipelineLayout(m_device, &pipeline_layout_ci, nullptr, pipeline_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreatePipelineLayout failed for pipeline layout " + name + "!", result);
    }

    set_debug_marker_name(&pipeline_layout, VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name);
}

void Device::create_semaphore(const VkSemaphoreCreateInfo &semaphore_ci, VkSemaphore *semaphore,
                              const std::string &name) const {
    if (const auto result = vkCreateSemaphore(m_device, &semaphore_ci, nullptr, semaphore); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSemaphore failed for " + name + " !", result);
    }

    set_debug_marker_name(&semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
}

} // namespace inexor::vulkan_renderer::wrapper
