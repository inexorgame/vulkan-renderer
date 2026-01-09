#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include "inexor/vulkan-renderer/tools/device_info.hpp"
#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/queue_selection.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#define VMA_DEBUG_MARGIN 16
#define VMA_DEBUG_DETECT_CORRUPTION 1
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include <spdlog/spdlog.h>

#include <functional>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

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
    VkPhysicalDeviceProperties device_properties{};
    vkGetPhysicalDeviceProperties(m_physical_device, &device_properties);
    std::memcpy(m_pipeline_cache_uuid.data(), device_properties.pipelineCacheUUID, VK_UUID_SIZE);

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

    // We want to use dynamic rendering (VK_KHR_dynamic_rendering)
    const auto dyn_rendering_feature = make_info<VkPhysicalDeviceDynamicRenderingFeaturesKHR>({
        .pNext = nullptr,
        .dynamicRendering = VK_TRUE,
    });

    const auto device_ci = make_info<VkDeviceCreateInfo>({
        // This is one of those rare cases where pNext is actually not nullptr!
        .pNext = &dyn_rendering_feature, // We use dynamic rendering
        .queueCreateInfoCount = static_cast<std::uint32_t>(optimal_queues.queues_to_create.size()),
        .pQueueCreateInfos = optimal_queues.queues_to_create.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(required_extensions.size()),
        .ppEnabledExtensionNames = required_extensions.data(),
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

    // Do we have any queue for compute?
    if (m_compute_queue_family_index) {
        vkGetDeviceQueue(m_device, m_compute_queue_family_index.value(), 0, &m_compute_queue);
    }
    // Do we have any queue for compute?
    if (m_transfer_queue_family_index) {
        vkGetDeviceQueue(m_device, m_transfer_queue_family_index.value(), 0, &m_transfer_queue);
    }
    // Do we have any queue for sparse binding?
    if (m_sparse_binding_queue_family_index) {
        vkGetDeviceQueue(m_device, m_sparse_binding_queue_family_index.value(), 0, &m_sparse_binding_queue);
    }

    // Check if compute or transfer queue can be used for presenting
    if (m_compute_queue_family_index && is_presentation_supported(surface, m_compute_queue_family_index.value())) {
        spdlog::trace("Using compute queue [queue family index: {}] for vkQueuePresentKHR",
                      m_compute_queue_family_index.value());
        m_present_queue = m_compute_queue;
    } else if (m_transfer_queue_family_index &&
               is_presentation_supported(surface, m_transfer_queue_family_index.value())) {
        spdlog::trace("Using transfer queue [queue family index: {}] for vkQueuePresentKHR",
                      m_transfer_queue_family_index.value());
        m_present_queue = m_transfer_queue;
    } else if (m_sparse_binding_queue_family_index &&
               is_presentation_supported(surface, m_sparse_binding_queue_family_index.value())) {
        spdlog::trace("Using sparse binding queue [queue family index: {}] for vkQueuePresentKHR",
                      m_sparse_binding_queue_family_index.value());
        m_present_queue = m_transfer_queue;
    } else {
        if (!is_presentation_supported(surface, m_graphics_queue_family_index.value())) {
            // This should be extremely unlikely.
            throw std::runtime_error("Error: Graphics queue does not support present on GPU '" + m_gpu_name + "'");
        }
        spdlog::trace("Using graphics queue [queue family index: {}] for vkQueuePresentKHR",
                      m_graphics_queue_family_index.value());
        m_present_queue = m_graphics_queue;
    }

    VmaVulkanFunctions vma_vk_functions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
    };

    const VmaAllocatorCreateInfo vma_ci{
        .physicalDevice = m_physical_device,
        .device = m_device,
        .pVulkanFunctions = &vma_vk_functions,
        .instance = inst.instance(),
        .vulkanApiVersion = Instance::REQUIRED_VK_API_VERSION,
    };

    spdlog::trace("Creating instance of Vulkan Memory Allocator (VMA)");
    if (const auto result = vmaCreateAllocator(&vma_ci, &m_allocator); result != VK_SUCCESS) {
        // To ensure the strong exception guarantee, we must destroy the device again here.
        vkDestroyDevice(m_device, nullptr);
        throw VulkanException("Error: vmaCreateAllocator failed!", result);
    }
}

Device::Device(Device &&other) noexcept : m_cmd_pools(std::move(other.m_cmd_pools)) {
    // After moving the cmd_pools, the memory is in a valid but unspecified state.
    // By calling .clear(), we bring it back into a defined state again.
    // This is not strictly necessary, but let's be safe in case we use it again.
    other.m_cmd_pools.clear();

    // TODO: Check me
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

void Device::execute(const std::string &name, const VkQueueFlagBits queue_type, const DebugLabelColor dbg_label_color,
                     const std::function<void(const CommandBuffer &cmd_buf)> &cmd_buf_recording_func,
                     const std::span<const VkSemaphore> wait_semaphores,
                     const std::span<const VkSemaphore> signal_semaphores) const {
    // Request the thread_local command pool for this queue type
    auto &cmd_pool = get_thread_command_pool(queue_type);
    // Start recording the command buffer
    const auto &cmd_buf = cmd_pool.request_command_buffer(name);
    // Begin a debug label region (visible in graphics debuggers like RenderDoc)
    cmd_buf.begin_debug_label_region(name, get_debug_label_color(dbg_label_color));
    // Call the external code
    std::invoke(cmd_buf_recording_func, cmd_buf);
    // End the debug label region
    cmd_buf.end_debug_label_region();
    // Submit the command buffer and do necessary synchronization
    cmd_buf.submit_and_wait(queue_type, wait_semaphores, signal_semaphores);
}

CommandPool &Device::get_thread_command_pool(const VkQueueFlagBits queue_type) const {
    // Note that thread_local means that it is implicitely static!
    thread_local CommandPool *thread_graphics_cmd_pool = nullptr;       // NOLINT
    thread_local CommandPool *thread_compute_cmd_pool = nullptr;        // NOLINT
    thread_local CommandPool *thread_transfer_cmd_pool = nullptr;       // NOLINT
    thread_local CommandPool *thread_sparse_binding_cmd_pool = nullptr; // NOLINT

    switch (queue_type) {
    case VK_QUEUE_GRAPHICS_BIT: {
        if (thread_graphics_cmd_pool == nullptr) {
            // Note that we checked during construction that there is a valid queue family index for graphics.
            // There is no need for additional error checking here.
            auto cmd_pool =
                std::make_unique<CommandPool>(*this, m_graphics_queue_family_index.value(), "graphics command pool");
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
            auto cmd_pool =
                std::make_unique<CommandPool>(*this, m_compute_queue_family_index.value(), "compute command pool");
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
            auto cmd_pool =
                std::make_unique<CommandPool>(*this, m_transfer_queue_family_index.value(), "transfer command pool");
            std::unique_lock lock(m_mutex);
            thread_transfer_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_transfer_cmd_pool;
    }
    case VK_QUEUE_SPARSE_BINDING_BIT: {
        if (thread_transfer_cmd_pool == nullptr) {
            if (!has_any_sparse_binding_queue()) {
                throw std::runtime_error("Error: GPU '" + m_gpu_name + "' has no sparse binding queue!");
            }
            auto cmd_pool = std::make_unique<CommandPool>(*this, m_sparse_binding_queue_family_index.value(),
                                                          "sparse binding command pool");
            std::unique_lock lock(m_mutex);
            thread_transfer_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_transfer_cmd_pool;
    }
    }
    throw std::runtime_error("Error: Unknown VuklkanQueueType!");
}

const CommandBuffer &Device::request_command_buffer(const VkQueueFlagBits queue_type, const std::string &name) {
    return get_thread_command_pool(queue_type).request_command_buffer(name);
}

void Device::update_descriptor_sets(const std::span<VkWriteDescriptorSet> write_descriptor_sets) {
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

} // namespace inexor::vulkan_renderer::wrapper
