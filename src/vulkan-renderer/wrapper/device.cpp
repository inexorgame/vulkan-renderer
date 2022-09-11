#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/settings_decision_maker.hpp"
#include "inexor/vulkan-renderer/vk_tools/enumerate.hpp"
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
#include <utility>

namespace {

// TODO: Make proper use of queue priorities in the future.
constexpr float DEFAULT_QUEUE_PRIORITY = 1.0f;

} // namespace

namespace inexor::vulkan_renderer::wrapper {

VkPhysicalDeviceMemoryProperties get_physical_device_memory_properties(const VkPhysicalDevice physical_device) {
    assert(physical_device);
    VkPhysicalDeviceMemoryProperties memory_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_props);
    return memory_props;
}

std::int64_t get_physical_device_memory_score(const VkPhysicalDevice physical_device) {
    // Get information about the physical device's memory
    VkPhysicalDeviceMemoryProperties memory_props;
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_props);
    std::int64_t memory_score = 0;
    for (std::size_t i = 0; i < memory_props.memoryHeapCount; i++) {
        if ((memory_props.memoryHeaps[i].flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0) {
            // Summarize real GPU memory in megabytes as a factor for the rating
            memory_score += memory_props.memoryHeaps[i].size / (1000 * 1000);
        }
    }
    return memory_score;
}

std::string get_physical_device_name(const VkPhysicalDevice physical_device) {
    assert(physical_device);
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physical_device, &props);
    return std::move(std::string(props.deviceName));
}

VkPhysicalDeviceType get_physical_device_type(const VkPhysicalDevice physical_device) {
    assert(physical_device);
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physical_device, &props);
    return props.deviceType;
}

bool is_extension_supported(const VkPhysicalDevice physical_device, const std::string &extension_name) {
    assert(physical_device);
    assert(!extension_name.empty());

    const auto extension_props = vk_tools::get_all_physical_device_extension_properties(physical_device);
    if (extension_props.empty()) {
        spdlog::info("No Vulkan device extensions available!");
        return false;
    }

    // Search for the requested device extension
    return std::find_if(extension_props.begin(), extension_props.end(),
                        [&](const VkExtensionProperties extension_prop) {
                            return extension_prop.extensionName == extension_name;
                        }) != extension_props.end();
}

bool is_presentation_supported(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface) {
    assert(physical_device);
    assert(surface);
    // Query if presentation is supported
    VkBool32 presentation_supported = VK_FALSE;
    if (const auto result = vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, 0, surface, &presentation_supported);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceSupportKHR failed!", result);
    }
    return presentation_supported == VK_TRUE;
}

bool is_swapchain_supported(const VkPhysicalDevice physical_device) {
    return is_extension_supported(physical_device, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

std::optional<VkPhysicalDevice> pick_physical_device(const VkInstance inst, const VkSurfaceKHR surface) {
    assert(inst);
    assert(surface);

    const auto physical_devices = vk_tools::get_all_physical_devices(inst);
    if (physical_devices.empty()) {
        spdlog::error("Error: No physical devices available!");
        return std::nullopt;
    }

    std::vector<VkPhysicalDevice> suitable_physical_devices;
    std::copy_if(physical_devices.begin(), physical_devices.end(), std::back_inserter(suitable_physical_devices),
                 [&](const VkPhysicalDevice candidate) {
                     return is_swapchain_supported(candidate) && is_presentation_supported(candidate, surface);
                 });

    std::sort(suitable_physical_devices.begin(), suitable_physical_devices.end(),
              [&](const VkPhysicalDevice lhs, const VkPhysicalDevice rhs) {
                  // If the physical devices are not of the same type, return the better type
                  const auto lhs_type = get_physical_device_type(lhs);
                  const auto rhs_type = get_physical_device_type(rhs);
                  if (lhs_type != rhs_type) {
                      // Use physical device type as first sorting criteria
                      return rate_physical_device_type(lhs_type) > rate_physical_device_type(rhs_type);
                  }
                  // In case the two physical devices have the same type, sort by memory score
                  return get_physical_device_memory_score(lhs) > get_physical_device_memory_score(rhs);
              });

    return suitable_physical_devices.front();
}

std::uint32_t rate_physical_device_type(const VkPhysicalDeviceType type) {
    switch (type) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return 2;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return 1;
    default:
        break;
    }
    return 0;
}

Device::Device(const Instance &inst, const VkSurfaceKHR surface, bool enable_vulkan_debug_markers,
               bool prefer_distinct_transfer_queue, const std::optional<std::uint32_t> preferred_index) {
    const auto physical_devices = vk_tools::get_all_physical_devices(inst.instance());
    if (physical_devices.empty()) {
        throw std::runtime_error("Error: There are no physical devices available");
    }

    // Check if the user specified a preferred gpu index
    if (preferred_index) {
        // Check if the user specified index is valid
        if (*preferred_index < physical_devices.size()) {
            const auto candidate = physical_devices[*preferred_index];
            // Check if the physical device supports all features
            if (is_swapchain_supported(candidate) && is_presentation_supported(candidate, surface)) {
                m_physical_device = candidate;
            } else {
                spdlog::error("Physical device {} is not suitable!", get_physical_device_name(candidate));
            }
        } else {
            spdlog::error("Preferred physical device index {} is invalid!", *preferred_index);
        }
    }

    // If the user specified no preferred physical device index or if it was invalid,
    // we need to pick a physical device automatically
    if (!m_physical_device) {
        const auto candidate = pick_physical_device(inst.instance(), surface);
        if (!candidate) {
            throw std::runtime_error("Error: Could not find a suitable physical device!");
        }
        m_physical_device = *candidate;
    }

    m_gpu_name = get_physical_device_name(m_physical_device);
    spdlog::trace("Creating device using graphics card: {}", m_gpu_name);

    std::vector<VkDeviceQueueCreateInfo> queues_to_create;
    spdlog::trace("Creating Vulkan device queues");

    if (prefer_distinct_transfer_queue) {
        spdlog::trace("The application will try to use a distinct data transfer queue if it is available");
    } else {
        spdlog::warn("The application is forced not to use a distinct data transfer queue!");
    }

    // Check if there is one queue family which can be used for both graphics and presentation.
    std::optional<std::uint32_t> queue_family_index_for_both_graphics_and_presentation =
        VulkanSettingsDecisionMaker::find_queue_family_for_both_graphics_and_presentation(m_physical_device, surface);

    if (queue_family_index_for_both_graphics_and_presentation) {
        spdlog::trace("One queue for both graphics and presentation will be used");

        m_graphics_queue_family_index = *queue_family_index_for_both_graphics_and_presentation;
        m_present_queue_family_index = m_graphics_queue_family_index;

        // In this case, there is one queue family which can be used for both graphics and presentation.
        queues_to_create.push_back(make_info<VkDeviceQueueCreateInfo>({
            .queueFamilyIndex = *queue_family_index_for_both_graphics_and_presentation,
            .queueCount = 1,
            .pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY,
        }));
    } else {
        spdlog::trace("No queue found which supports both graphics and presentation");
        spdlog::trace("The application will try to use 2 separate queues");

        // We have to use 2 different queue families.
        // One for graphics and another one for presentation.

        // Check which queue family index can be used for graphics.
        auto queue_candidate = VulkanSettingsDecisionMaker::find_graphics_queue_family(m_physical_device);

        if (!queue_candidate) {
            throw std::runtime_error("Could not find suitable queue family indices for graphics!");
        }

        m_graphics_queue_family_index = *queue_candidate;

        // Check which queue family index can be used for presentation.
        queue_candidate = VulkanSettingsDecisionMaker::find_presentation_queue_family(m_physical_device, surface);

        if (!queue_candidate) {
            throw std::runtime_error("Could not find suitable queue family indices for presentation!");
        }

        m_present_queue_family_index = *queue_candidate;

        // Set up one queue for graphics.
        queues_to_create.push_back(make_info<VkDeviceQueueCreateInfo>({
            .queueFamilyIndex = m_graphics_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY,
        }));

        // Set up one queue for presentation.
        queues_to_create.push_back(make_info<VkDeviceQueueCreateInfo>({
            .queueFamilyIndex = m_present_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY,
        }));
    }

    // Add another device queue just for data transfer.
    const auto queue_candidate =
        VulkanSettingsDecisionMaker::find_distinct_data_transfer_queue_family(m_physical_device);

    bool use_distinct_data_transfer_queue = false;

    if (queue_candidate && prefer_distinct_transfer_queue) {
        m_transfer_queue_family_index = *queue_candidate;

        spdlog::trace("A separate queue will be used for data transfer.");

        // We have the opportunity to use a separated queue for data transfer!
        use_distinct_data_transfer_queue = true;

        queues_to_create.push_back({
            .queueFamilyIndex = m_transfer_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY,
        });
    } else {
        // We don't have the opportunity to use a separated queue for data transfer!
        // Do not create a new queue, use the graphics queue instead.
        use_distinct_data_transfer_queue = false;
    }

    if (!use_distinct_data_transfer_queue) {
        spdlog::warn("The application is forced to avoid distinct data transfer queues");
        spdlog::warn("Because of this, the graphics queue will be used for data transfer");

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
        if (is_extension_supported(m_physical_device, device_extension_name)) {
            spdlog::trace("Device extension {} is available on this system", device_extension_name);
            enabled_device_extensions.push_back(device_extension_name);
        } else {
            throw std::runtime_error("Device extension " + std::string(device_extension_name) +
                                     " is not available on this system!");
        }
    }

    const VkPhysicalDeviceFeatures used_features{
        // Enable anisotropic filtering.
        .samplerAnisotropy = VK_TRUE,
    };

    const auto device_ci = make_info<VkDeviceCreateInfo>({
        .queueCreateInfoCount = static_cast<std::uint32_t>(queues_to_create.size()),
        .pQueueCreateInfos = queues_to_create.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(enabled_device_extensions.size()),
        .ppEnabledExtensionNames = enabled_device_extensions.data(),
        .pEnabledFeatures = &used_features,
    });

    spdlog::trace("Creating physical device");

    if (const auto result = vkCreateDevice(m_physical_device, &device_ci, nullptr, &m_device); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDevice failed!", result);
    }

#ifndef NDEBUG
    if (enable_vulkan_debug_markers) {
        spdlog::trace("Initializing Vulkan debug markers");

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

    spdlog::trace("Queue family indices:");
    spdlog::trace("   - Graphics: {}", m_graphics_queue_family_index);
    spdlog::trace("   - Present: {}", m_present_queue_family_index);
    spdlog::trace("   - Transfer: {}", m_transfer_queue_family_index);

    // Setup the queues for presentation and graphics.
    // Since we only have one queue per queue family, we acquire index 0.
    vkGetDeviceQueue(m_device, m_present_queue_family_index, 0, &m_present_queue);
    vkGetDeviceQueue(m_device, m_graphics_queue_family_index, 0, &m_graphics_queue);

    // The use of data transfer queues can be forbidden by using -no_separate_data_queue.
    if (use_distinct_data_transfer_queue) {
        // Use a separate queue for data transfer to GPU.
        vkGetDeviceQueue(m_device, m_transfer_queue_family_index, 0, &m_transfer_queue);
    }

    spdlog::trace("Creating VMA allocator");

    const VmaAllocatorCreateInfo vma_allocator_ci{
        .physicalDevice = m_physical_device,
        .device = m_device,
        .instance = inst.instance(),
        // Just tell Vulkan Memory Allocator to use Vulkan 1.1, even if a newer version is specified in instance wrapper
        // This might need to be changed in the future
        .vulkanApiVersion = VK_API_VERSION_1_1,
    };

    spdlog::trace("Creating Vulkan memory allocator instance");

    if (const auto result = vmaCreateAllocator(&vma_allocator_ci, &m_allocator); result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateAllocator failed!", result);
    }
}

Device::Device(Device &&other) noexcept {
    m_device = std::exchange(other.m_device, nullptr);
    m_physical_device = std::exchange(other.m_physical_device, nullptr);
}

Device::~Device() {
    std::scoped_lock locker(m_mutex);

    // Because the device handle must be valid for the destruction of the command pools in the CommandPool destructor,
    // we must destroy the command pools manually here in order to ensure the right order of destruction
    m_cmd_pools.clear();

    // Now that we destroyed the command pools, we can destroy the allocator and finally the device itself
    vmaDestroyAllocator(m_allocator);
    vkDestroyDevice(m_device, nullptr);
}

void Device::execute(const std::string &name,
                     const std::function<void(const CommandBuffer &cmd_buf)> &cmd_lambda) const {
    // TODO: Support other queues (not just graphics)
    const auto &cmd_buf = thread_graphics_pool().request_command_buffer(name);
    // Execute the lambda
    cmd_lambda(cmd_buf);
    cmd_buf.submit_and_wait();
}

void Device::set_debug_marker_name(void *object, VkDebugReportObjectTypeEXT object_type,
                                   const std::string &name) const {
#ifndef NDEBUG
    if (!m_vk_debug_marker_set_object_name) {
        return;
    }

    assert(object);
    assert(!name.empty());

    const auto name_info = make_info<VkDebugMarkerObjectNameInfoEXT>({
        .objectType = object_type,
        .object = reinterpret_cast<std::uint64_t>(object), // NOLINT
        .pObjectName = name.c_str(),
    });

    if (const auto result = m_vk_debug_marker_set_object_name(m_device, &name_info); result != VK_SUCCESS) {
        throw VulkanException("Failed to assign Vulkan debug marker name " + name + "!", result);
    }
#endif
}

void Device::set_memory_block_attachment(void *object, VkDebugReportObjectTypeEXT object_type, const std::uint64_t name,
                                         const std::size_t memory_size, const void *memory_block) const {
#ifndef NDEBUG
    if (!m_vk_debug_marker_set_object_tag) {
        return;
    }

    assert(name);
    assert(memory_size > 0);
    assert(memory_block);

    const auto tag_info = make_info<VkDebugMarkerObjectTagInfoEXT>({
        .objectType = object_type,
        .object = reinterpret_cast<std::uint64_t>(object), // NOLINT
        .tagName = name,
        .tagSize = memory_size,
        .pTag = memory_block,
    });

    if (const auto result = m_vk_debug_marker_set_object_tag(m_device, &tag_info); result != VK_SUCCESS) {
        throw VulkanException("Failed to assign Vulkan debug marker memory block!", result);
    }
#endif
}

void Device::bind_debug_region(const VkCommandBuffer command_buffer, const std::string &name,
                               const std::array<float, 4> color) const {
#ifndef NDEBUG
    if (!m_vk_cmd_debug_marker_begin) {
        return;
    }

    assert(command_buffer);
    assert(!name.empty());

    auto debug_marker = make_info<VkDebugMarkerMarkerInfoEXT>();

    std::copy(color.begin(), color.end(), debug_marker.color);

    debug_marker.pMarkerName = name.c_str();

    m_vk_cmd_debug_marker_begin(command_buffer, &debug_marker);
#endif
}

void Device::insert_debug_marker(const VkCommandBuffer command_buffer, const std::string &name,
                                 const std::array<float, 4> color) const {
#ifndef NDEBUG
    if (!m_vk_cmd_debug_marker_insert) {
        return;
    }

    assert(command_buffer);
    assert(!name.empty());

    auto debug_marker = make_info<VkDebugMarkerMarkerInfoEXT>();

    std::copy(color.begin(), color.end(), debug_marker.color);

    debug_marker.pMarkerName = name.c_str();

    m_vk_cmd_debug_marker_insert(command_buffer, &debug_marker);
#endif
}

void Device::end_debug_region(const VkCommandBuffer command_buffer) const {
#ifndef NDEBUG
    if (!m_vk_cmd_debug_marker_end) {
        return;
    }

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

void Device::create_fence(const VkFenceCreateInfo &fence_ci, VkFence *fence, const std::string &name) const {
    if (const auto result = vkCreateFence(m_device, &fence_ci, nullptr, fence); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateFence failed for fence " + name + "!", result);
    }

    set_debug_marker_name(&fence, VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, name);
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

void Device::create_render_pass(const VkRenderPassCreateInfo &render_pass_ci, VkRenderPass *render_pass,
                                const std::string &name) const {
    if (const auto result = vkCreateRenderPass(m_device, &render_pass_ci, nullptr, render_pass); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateRenderPass failed for renderpass " + name + " !", result);
    }

    set_debug_marker_name(&render_pass, VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, name);
}

void Device::create_sampler(const VkSamplerCreateInfo &sampler_ci, VkSampler *sampler, const std::string &name) const {
    if (const auto result = vkCreateSampler(m_device, &sampler_ci, nullptr, sampler); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSampler failed for sampler " + name + " !", result);
    }

    set_debug_marker_name(&sampler, VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name);
}

void Device::create_semaphore(const VkSemaphoreCreateInfo &semaphore_ci, VkSemaphore *semaphore,
                              const std::string &name) const {
    if (const auto result = vkCreateSemaphore(m_device, &semaphore_ci, nullptr, semaphore); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSemaphore failed for " + name + " !", result);
    }

    set_debug_marker_name(&semaphore, VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name);
}

void Device::create_shader_module(const VkShaderModuleCreateInfo &shader_module_ci, VkShaderModule *shader_module,
                                  const std::string &name) const {
    if (const auto result = vkCreateShaderModule(m_device, &shader_module_ci, nullptr, shader_module);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateShaderModule failed for shader module " + name + "!", result);
    }

    set_debug_marker_name(&shader_module, VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name);
}

void Device::create_swapchain(const VkSwapchainCreateInfoKHR &swapchain_ci, VkSwapchainKHR *swapchain,
                              const std::string &name) const {
    if (const auto result = vkCreateSwapchainKHR(m_device, &swapchain_ci, nullptr, swapchain); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateSwapchainKHR failed for swapchain " + name + "!", result);
    }

    set_debug_marker_name(&swapchain, VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, name);
}

CommandPool &Device::thread_graphics_pool() const {
    // Note that thread_graphics_pool is implicitely static!
    thread_local CommandPool *thread_graphics_pool = nullptr; // NOLINT
    if (thread_graphics_pool == nullptr) {
        auto cmd_pool = std::make_unique<CommandPool>(*this, "graphics pool");
        std::scoped_lock locker(m_mutex);
        thread_graphics_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
    }
    return *thread_graphics_pool;
}

const CommandBuffer &Device::request_command_buffer(const std::string &name) {
    return thread_graphics_pool().request_command_buffer(name);
}

} // namespace inexor::vulkan_renderer::wrapper
