#include "inexor/vulkan-renderer/wrapper/core/device.hpp"

#include "inexor/vulkan-renderer/tools/device_info.hpp"
#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/tools/queue_selection.hpp"
#include "inexor/vulkan-renderer/wrapper/core/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"

#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <spdlog/spdlog.h>

#include <algorithm>
#include <functional>
#include <string_view>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::core {

/// Convert a DebugLabelColor to a rgba value
/// @param color The debug label color
/// @return The converted rgba values
std::array<float, 4> get_debug_label_color(const DebugLabelColor color) {
    switch (color) {
    case DebugLabelColor::RED:
        return {0.98f, 0.60f, 0.60f, 1.0f};
    case DebugLabelColor::BLUE:
        return {0.68f, 0.85f, 0.90f, 1.0f};
    case DebugLabelColor::GREEN:
        return {0.73f, 0.88f, 0.73f, 1.0f};
    case DebugLabelColor::YELLOW:
        return {0.98f, 0.98f, 0.70f, 1.0f};
    case DebugLabelColor::PURPLE:
        return {0.80f, 0.70f, 0.90f, 1.0f};
    case DebugLabelColor::ORANGE:
        return {0.98f, 0.75f, 0.53f, 1.0f};
    case DebugLabelColor::MAGENTA:
        return {0.96f, 0.60f, 0.76f, 1.0f};
    case DebugLabelColor::CYAN:
        return {0.70f, 0.98f, 0.98f, 1.0f};
    case DebugLabelColor::BROWN:
        return {0.82f, 0.70f, 0.55f, 1.0f};
    case DebugLabelColor::PINK:
        return {0.98f, 0.75f, 0.85f, 1.0f};
    case DebugLabelColor::LIME:
        return {0.80f, 0.98f, 0.60f, 1.0f};
    case DebugLabelColor::TURQUOISE:
        return {0.70f, 0.93f, 0.93f, 1.0f};
    case DebugLabelColor::BEIGE:
        return {0.96f, 0.96f, 0.86f, 1.0f};
    case DebugLabelColor::MAROON:
        return {0.76f, 0.50f, 0.50f, 1.0f};
    case DebugLabelColor::OLIVE:
        return {0.74f, 0.75f, 0.50f, 1.0f};
    case DebugLabelColor::NAVY:
        return {0.53f, 0.70f, 0.82f, 1.0f};
    case DebugLabelColor::TEAL:
        return {0.53f, 0.80f, 0.75f, 1.0f};
    default:
        return {0.0f, 0.0f, 0.0f, 1.0f}; // Default to opaque black if the color is not recognized
    }
}

Device::Device(const Instance &inst, const VkSurfaceKHR surface, const VkPhysicalDevice desired_gpu,
               const VkPhysicalDeviceFeatures &required_features, const std::span<const char *> required_extensions)
    : m_enabled_features(required_features) {
    // Lets just be safe and check if these function pointers are really available.
    if (vkCreateDevice == nullptr) {
        throw InexorException("Error: Function pointer 'vkCreateDevice' is not available!");
    }

    // Get information about the desired gpu.
    const auto gpu_info = tools::build_device_info(desired_gpu, surface);

    // Check if this gpu is even suitable for the application's purposes.
    if (!tools::is_gpu_suitable(gpu_info, required_features, required_extensions, true)) {
        spdlog::error("Error: Selected GPU '{}' was evaluated as unsuitable!", gpu_info.name);

        // Attempt to pick another GPU automatically instead.
        m_physical_device = tools::pick_best_physical_device(inst, surface, required_features, required_extensions);
        m_gpu_name = tools::get_physical_device_name(m_physical_device);
        spdlog::warn("GPU '{}' was selected automatically as alternative!", m_gpu_name);
    } else {
        // The desired GPU turned out to be suitable.
        spdlog::trace("Creating physical device using GPU '{}'", gpu_info.name);
        m_physical_device = desired_gpu;
        m_gpu_name = gpu_info.name;
    }

    // Get the device properties
    VkPhysicalDeviceProperties2 device_properties2{};
    device_properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
    vkGetPhysicalDeviceProperties2(m_physical_device, &device_properties2);
    std::memcpy(m_pipeline_cache_uuid.data(), device_properties2.properties.pipelineCacheUUID, VK_UUID_SIZE);

    spdlog::trace("Creating Vulkan queues");

    const auto props = tools::get_queue_family_properties(m_physical_device);
    const auto optimal_queues = tools::determine_queue_family_indices(props);

    // We can live without a transfer queue, we can live without a compute queue, but we can't live without graphics.
    if (!optimal_queues.graphics) {
        throw std::runtime_error("Error: No queue found with VK_QUEUE_TYPE_GRAPHICS_BIT for GPU '" + m_gpu_name + "'");
    }

    m_graphics_queue_family_index = optimal_queues.graphics;
    // This could be std::nullopt!
    m_compute_queue_family_index = optimal_queues.compute;
    // This could be std::nullopt!
    m_transfer_queue_family_index = optimal_queues.transfer;
    // This could be std::nullopt!
    m_sparse_binding_queue_family_index = optimal_queues.sparse_binding;

    // Store the enabled features.
    m_enabled_features = required_features;

    std::vector<const char *> enabled_extensions(required_extensions.begin(), required_extensions.end());
    const auto available_extensions = tools::get_extension_properties(m_physical_device);
    const bool memory_priority_ext_supported =
        tools::is_extension_supported(available_extensions, VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
    bool memory_priority_feature_supported = false;
    if (memory_priority_ext_supported) {
        VkPhysicalDeviceMemoryPriorityFeaturesEXT memory_priority_features{};
        memory_priority_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT;

        VkPhysicalDeviceFeatures2 features2{};
        features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
        features2.pNext = &memory_priority_features;
        vkGetPhysicalDeviceFeatures2(m_physical_device, &features2);

        memory_priority_feature_supported = (memory_priority_features.memoryPriority == VK_TRUE);
    }

    const bool memory_priority_supported = memory_priority_ext_supported && memory_priority_feature_supported;
    if (memory_priority_supported) {
        const auto not_already_enabled = std::find(enabled_extensions.begin(), enabled_extensions.end(),
                                                   VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME) == enabled_extensions.end();
        if (not_already_enabled) {
            enabled_extensions.push_back(VK_EXT_MEMORY_PRIORITY_EXTENSION_NAME);
        }
    }

    // We want to use synchronization2 for vkCmdPipelineBarrier2.
    VkPhysicalDeviceSynchronization2Features sync2_feature{};
    sync2_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES;
    sync2_feature.pNext = nullptr;
    sync2_feature.synchronization2 = VK_TRUE;

    VkPhysicalDeviceMemoryPriorityFeaturesEXT memory_priority_feature{};
    memory_priority_feature.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PRIORITY_FEATURES_EXT;
    memory_priority_feature.pNext = nullptr;
    memory_priority_feature.memoryPriority = memory_priority_supported ? VK_TRUE : VK_FALSE;

    sync2_feature.pNext = memory_priority_supported ? &memory_priority_feature : nullptr;

    using tools::make_info;

    // We want to use dynamic rendering (VK_KHR_dynamic_rendering).
    auto dyn_rendering_feature = make_info<VkPhysicalDeviceDynamicRenderingFeaturesKHR>({
        .pNext = &sync2_feature,
        .dynamicRendering = VK_TRUE,
    });

    const auto device_ci = make_info<VkDeviceCreateInfo>({
        // This is one of those rare cases where pNext is actually not nullptr!
        .pNext = &dyn_rendering_feature, // We use dynamic rendering
        .queueCreateInfoCount = static_cast<std::uint32_t>(optimal_queues.queues_to_create.size()),
        .pQueueCreateInfos = optimal_queues.queues_to_create.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(enabled_extensions.size()),
        .ppEnabledExtensionNames = enabled_extensions.data(),
        .pEnabledFeatures = &m_enabled_features,
    });

    auto print_queue_family_index = [&](const std::optional<std::uint32_t> index) {
        return index ? std::to_string(index.value()) : std::string("NONE");
    };

    spdlog::trace("Selected queue family indices: [graphics: {}, compute: {}, transfer: {}, sparse binding: {}]",
                  print_queue_family_index(m_graphics_queue_family_index),
                  print_queue_family_index(m_compute_queue_family_index),
                  print_queue_family_index(m_transfer_queue_family_index),
                  print_queue_family_index(m_sparse_binding_queue_family_index));

    spdlog::trace("Creating device from GPU '{}'", m_gpu_name);

    if (const auto result = vkCreateDevice(m_physical_device, &device_ci, nullptr, &m_device); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDevice failed!", result);
    }

    spdlog::trace("Loading Vulkan device-level function pointers with volkLoadDevice");
    volkLoadDevice(m_device);

    // Let's just check if these function pointers is really available now.
    // There checks are probably redundant because volkLoadDevice would have thrown an exception already if they were
    // not available. If volk would still not have catched this, it would either be a bug in volk or something with the
    // available Vulkan API runtime is fundamentally wrong.
    if (vkDestroyDevice == nullptr) {
        throw InexorException("Error: Function pointer 'vkDestroyDevice' is not available!");
    }
    if (vkGetDeviceQueue == nullptr) {
        // To ensure the strong exception guarantee, we must destroy the device again here.
        vkDestroyDevice(m_device, nullptr);
        throw InexorException("Error: Function pointer 'vkGetDeviceQueue' is not available!");
    }

    spdlog::trace("Getting Vulkan queues from device");
    // It's important to call vkGetDeviceQueue after volkLoadDevice!

    // We already checked earlier if the graphics queue family index is not std::nullopt.
    vkGetDeviceQueue(m_device, m_graphics_queue_family_index.value(), 0, &m_graphics_queue);
    set_debug_name(m_graphics_queue, "m_graphics_queue");

    // Do we have any queue for compute?
    if (m_compute_queue_family_index) {
        vkGetDeviceQueue(m_device, m_compute_queue_family_index.value(), 0, &m_compute_queue);
        set_debug_name(m_compute_queue, "m_compute_queue");
    }
    // Do we have any queue for compute?
    if (m_transfer_queue_family_index) {
        vkGetDeviceQueue(m_device, m_transfer_queue_family_index.value(), 0, &m_transfer_queue);
        set_debug_name(m_transfer_queue, "m_transfer_queue");
    }
    // Do we have any queue for sparse binding?
    if (m_sparse_binding_queue_family_index) {
        vkGetDeviceQueue(m_device, m_sparse_binding_queue_family_index.value(), 0, &m_sparse_binding_queue);
        set_debug_name(m_sparse_binding_queue, "m_sparse_binding_queue");
    }

    VmaVulkanFunctions vma_vk_functions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
    };

    auto vma_ci = VmaAllocatorCreateInfo{
        .physicalDevice = m_physical_device,
        .device = m_device,
        .pVulkanFunctions = &vma_vk_functions,
        .instance = inst.instance(),
        .vulkanApiVersion = Instance::REQUIRED_VK_API_VERSION,
    };
    if (memory_priority_supported) {
        vma_ci.flags |= VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT;
    }

    spdlog::trace("Creating instance of Vulkan Memory Allocator (VMA)");
    if (const auto result = vmaCreateAllocator(&vma_ci, &m_allocator); result != VK_SUCCESS) {
        // To ensure the strong exception guarantee, we must destroy the device again here.
        vkDestroyDevice(m_device, nullptr);
        throw VulkanException("Error: vmaCreateAllocator failed!", result);
    }

    m_pipeline_cache = std::make_unique<PipelineCache>(*this);

    // Create command pools at here instead of allocating them lazily at first request during runtime
    get_thread_command_pool(VK_QUEUE_GRAPHICS_BIT);
    get_thread_command_pool(VK_QUEUE_TRANSFER_BIT);
    get_thread_command_pool(VK_QUEUE_COMPUTE_BIT);
    get_thread_command_pool(VK_QUEUE_SPARSE_BINDING_BIT);
}

Device::~Device() {
    std::scoped_lock locker(m_mutex);
    // Wait for the device to complete ongoing work
    spdlog::trace("Device::~Device begin");
    wait_idle();
    // Because the device handle must be valid for the destruction of the command pools in the CommandPool destructor,
    // we must destroy the command pools manually here in order to ensure the right order of destruction
    m_cmd_pools.clear();
    // Dump detailed allocator stats before destruction so leaking allocations can be identified by name.
    char *vma_stats_string = nullptr;
    vmaBuildStatsString(m_allocator, &vma_stats_string, VK_TRUE);
    if (vma_stats_string != nullptr) {
        spdlog::warn("VMA allocator stats before destruction:\n{}", vma_stats_string);
        vmaFreeStatsString(m_allocator, vma_stats_string);
    }
    // Now that we destroyed the command pools, we can destroy the allocator and finally the device itself
    spdlog::trace("Device::~Device destroying VMA allocator");
    vmaDestroyAllocator(m_allocator);
    // Shutdown pipeline cache
    m_pipeline_cache.reset();
    // Destroy the device
    vkDestroyDevice(m_device, nullptr);
    spdlog::trace("Device::~Device end");
}

bool Device::is_presentation_supported(const VkSurfaceKHR surface, const std::uint32_t queue_family_index) const {
    // Default to true in this case where a surface is not passed (and therefore presentation isn't cared about)
    VkBool32 supported = VK_TRUE;
    if (const auto result =
            vkGetPhysicalDeviceSurfaceSupportKHR(m_physical_device, queue_family_index, surface, &supported);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceSupportKHR failed!", result);
    }
    return supported == VK_TRUE;
}

VkSurfaceCapabilitiesKHR Device::get_surface_capabilities(const VkSurfaceKHR surface) const {
    VkSurfaceCapabilitiesKHR caps{};
    if (const auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physical_device, surface, &caps);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed!", result);
    }
    return caps;
}

bool Device::surface_supports_usage(const VkSurfaceKHR surface, const VkImageUsageFlagBits usage) const {
    const auto capabilities = get_surface_capabilities(surface);
    return (capabilities.supportedUsageFlags & usage) != 0u;
}

VkFence Device::execute(const VkQueueFlagBits queue_type, const DebugLabelColor dbg_label_color,
                        const std::function<void(const CommandBuffer &cmd_buf)> &on_record,
                        const std::span<const VkSemaphore> wait_semaphores,
                        const std::span<const VkSemaphore> signal_semaphores,
                        const std::source_location source_location) const {
    const auto &cmd_buf = get_thread_command_pool(queue_type).request_command_buffer(source_location.function_name());
    cmd_buf.begin_debug_label_region(source_location.function_name(), get_debug_label_color(dbg_label_color));
    std::invoke(on_record, cmd_buf);
    cmd_buf.end_debug_label_region();
    cmd_buf.end_command_buffer();
    cmd_buf.submit(queue_type, wait_semaphores, signal_semaphores);
    return cmd_buf.submission_fence();
}

VkFence Device::execute(const VkQueueFlagBits queue_type, const DebugLabelColor dbg_label_color,
                        const std::function<void(const CommandBuffer &cmd_buf)> &on_record,
                        const std::span<const VkSemaphore> wait_semaphores,
                        const std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos,
                        const std::source_location source_location) const {
    const auto &cmd_buf = get_thread_command_pool(queue_type).request_command_buffer(source_location.function_name());
    cmd_buf.begin_debug_label_region(source_location.function_name(), get_debug_label_color(dbg_label_color));
    std::invoke(on_record, cmd_buf);
    cmd_buf.end_debug_label_region();
    cmd_buf.end_command_buffer();
    cmd_buf.submit(queue_type, wait_semaphores, signal_semaphore_infos);
    return cmd_buf.submission_fence();
}

VkFence Device::execute(const VkQueueFlagBits queue_type, const DebugLabelColor dbg_label_color,
                        const std::function<void(const CommandBuffer &cmd_buf)> &on_record,
                        const std::span<const QueueSemaphoreWait> wait_semaphores,
                        const std::span<const VkSemaphore> signal_semaphores,
                        const std::source_location source_location) const {
    const auto &cmd_buf = get_thread_command_pool(queue_type).request_command_buffer(source_location.function_name());
    cmd_buf.begin_debug_label_region(source_location.function_name(), get_debug_label_color(dbg_label_color));
    std::invoke(on_record, cmd_buf);
    cmd_buf.end_debug_label_region();
    cmd_buf.end_command_buffer();

    cmd_buf.submit(queue_type, wait_semaphores, signal_semaphores);
    return cmd_buf.submission_fence();
}

VkFence Device::execute(const VkQueueFlagBits queue_type, const DebugLabelColor dbg_label_color,
                        const std::function<void(const CommandBuffer &cmd_buf)> &on_record,
                        const std::span<const QueueSemaphoreWait> wait_semaphores,
                        const std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos,
                        const std::source_location source_location) const {
    const auto &cmd_buf = get_thread_command_pool(queue_type).request_command_buffer(source_location.function_name());
    cmd_buf.begin_debug_label_region(source_location.function_name(), get_debug_label_color(dbg_label_color));
    std::invoke(on_record, cmd_buf);
    cmd_buf.end_debug_label_region();
    cmd_buf.end_command_buffer();

    cmd_buf.submit(queue_type, wait_semaphores, signal_semaphore_infos);
    return cmd_buf.submission_fence();
}

CommandPool &Device::get_thread_command_pool(const VkQueueFlagBits queue_type) const {
    // NOTE: thread_local keyword means that it is implicitely static!
    thread_local CommandPool *thread_graphics_cmd_pool = nullptr;       // NOLINT
    thread_local CommandPool *thread_compute_cmd_pool = nullptr;        // NOLINT
    thread_local CommandPool *thread_transfer_cmd_pool = nullptr;       // NOLINT
    thread_local CommandPool *thread_sparse_binding_cmd_pool = nullptr; // NOLINT

    switch (queue_type) {
    case VK_QUEUE_GRAPHICS_BIT: {
        if (thread_graphics_cmd_pool == nullptr) {
            // NOTE: We checked earlier for a valid queue family index for graphics so no error checks required here.
            auto cmd_pool = std::make_unique<CommandPool>(*this, queue_type, m_graphics_queue_family_index.value(),
                                                          "thread_graphics_cmd_pool");
            std::unique_lock lock(m_mutex);
            thread_graphics_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_graphics_cmd_pool;
    }
    case VK_QUEUE_COMPUTE_BIT: {
        if (thread_compute_cmd_pool == nullptr) {
            if (!has_any_compute_queue()) {
                throw std::runtime_error("Error: GPU '" + m_gpu_name + "' has no compute queue!");
            }
            auto cmd_pool = std::make_unique<CommandPool>(*this, queue_type, m_compute_queue_family_index.value(),
                                                          "thread_compute_cmd_pool");
            std::unique_lock lock(m_mutex);
            thread_compute_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_compute_cmd_pool;
    }
    case VK_QUEUE_TRANSFER_BIT: {
        if (thread_transfer_cmd_pool == nullptr) {
            if (!has_any_transfer_queue()) {
                throw std::runtime_error("Error: GPU '" + m_gpu_name + "' has no transfer queue!");
            }
            auto cmd_pool = std::make_unique<CommandPool>(*this, queue_type, m_transfer_queue_family_index.value(),
                                                          "thread_transfer_cmd_pool");
            std::unique_lock lock(m_mutex);
            thread_transfer_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_transfer_cmd_pool;
    }
    case VK_QUEUE_SPARSE_BINDING_BIT: {
        if (thread_sparse_binding_cmd_pool == nullptr) {
            if (!has_any_sparse_binding_queue()) {
                throw std::runtime_error("Error: GPU '" + m_gpu_name + "' has no sparse binding queue!");
            }
            auto cmd_pool = std::make_unique<CommandPool>(
                *this, queue_type, m_sparse_binding_queue_family_index.value(), "thread_sparse_binding_cmd_pool");
            std::unique_lock lock(m_mutex);
            thread_sparse_binding_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_sparse_binding_cmd_pool;
    }
    }
    throw std::runtime_error("Error: Unknown VuklkanQueueType!");
}

VkPipelineCache Device::pipeline_cache() const {
    return m_pipeline_cache->cache();
}

const CommandBuffer &Device::request_command_buffer(const VkQueueFlagBits queue_type, const std::string &name) {
    return get_thread_command_pool(queue_type).request_command_buffer(name);
}

const CommandBuffer &Device::request_secondary_command_buffer(const VkQueueFlagBits queue_type,
                                                              const std::string &name) {
    return get_thread_command_pool(queue_type).request_secondary_command_buffer(name);
}

void Device::wait_for_submissions(const VkQueueFlagBits queue_type) const {
    get_thread_command_pool(queue_type).wait_for_all_submissions();
}

void Device::update_descriptor_sets(const std::span<VkWriteDescriptorSet> write_descriptor_sets) {
    // NOTE: No error checks are required here because this function is of type void
    vkUpdateDescriptorSets(m_device, static_cast<std::uint32_t>(write_descriptor_sets.size()),
                           write_descriptor_sets.data(), 0, nullptr);
}

void Device::wait_idle(const VkQueue queue) const {
    if (queue == VK_NULL_HANDLE) {
        if (const auto result = vkDeviceWaitIdle(m_device); result != VK_SUCCESS) {
            throw VulkanException("Error: vkDeviceWaitIdle failed!", result);
        }
    } else {
        if (const auto result = vkQueueWaitIdle(queue); result != VK_SUCCESS) {
            throw VulkanException("Error: vkQueueWaitIdle failed!", result);
        }
    }
}

} // namespace inexor::vulkan_renderer::wrapper::core
