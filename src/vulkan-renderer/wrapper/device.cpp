#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include "inexor/vulkan-renderer/tools/device_info.hpp"
#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/queue_selection.hpp"
#include "inexor/vulkan-renderer/tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#define VMA_IMPLEMENTATION

// Enforce specified number of bytes as a margin before and after every allocation.
#define VMA_DEBUG_MARGIN 16 // NOLINT

// Enable validation of contents of the margins.
#define VMA_DEBUG_DETECT_CORRUPTION 1 // NOLINT

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#include <algorithm>
#include <cassert>
#include <fstream>

using inexor::vulkan_renderer::tools::VulkanException;

namespace inexor::vulkan_renderer::wrapper {

Device::Device(const Instance &inst, const VkSurfaceKHR surface, const VkPhysicalDevice desired_gpu,
               const VkPhysicalDeviceFeatures &required_features, const std::span<const char *> required_extensions) {
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

    spdlog::trace("Creating Vulkan queues");

    const auto props = tools::get_queue_family_properties(m_physical_device);
    const auto optimal_queues = tools::determine_queue_family_indices(props, gpu_info.name);

    // We can live without a transfer queue, we can live without a compute queue, but we can't live without graphics.
    if (!optimal_queues.graphics) {
        throw std::runtime_error("Error: No queue found with VK_QUEUE_TYPE_GRAPHICS_BIT for GPU '" + m_gpu_name + "'");
    }

    m_graphics_queue_family_index = optimal_queues.graphics;
    // This could be std::nullopt!
    m_compute_queue_family_index = optimal_queues.compute;
    // This could be std::nullopt!
    m_transfer_queue_family_index = optimal_queues.transfer;
    // @TODO Implement sparse binding queue and further queue types.

    const auto device_ci = make_info<VkDeviceCreateInfo>({
        .queueCreateInfoCount = static_cast<std::uint32_t>(optimal_queues.queues_to_create.size()),
        .pQueueCreateInfos = optimal_queues.queues_to_create.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(required_extensions.size()),
        .ppEnabledExtensionNames = required_extensions.data(),
        .pEnabledFeatures = &m_enabled_features,
    });

    auto print_queue_family_index = [&](const std::optional<std::uint32_t> index) {
        return index ? std::to_string(index.value()) : std::string("NONE");
    };

    spdlog::trace("Creating device from GPU '{}' with queue family indices: [graphics: {}, compute: {}, transfer: {}]",
                  m_gpu_name, print_queue_family_index(m_graphics_queue_family_index),
                  print_queue_family_index(m_compute_queue_family_index),
                  print_queue_family_index(m_transfer_queue_family_index));
    if (const auto result = vkCreateDevice(m_physical_device, &device_ci, nullptr, &m_device); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDevice failed!", result);
    }

    spdlog::trace("Loading Vulkan dynamically using volk metaloader library");
    volkLoadDevice(m_device);

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
    // @TODO Implement sparse binding queue and further queue types.

    // Check if compute or transfer queue can be used for presenting
    if (m_compute_queue_family_index && is_presentation_supported(surface, m_compute_queue_family_index.value())) {
        spdlog::trace("Using compute queue for vkQueuePresentKHR");
        m_present_queue = m_compute_queue;
    } else if (m_transfer_queue_family_index &&
               is_presentation_supported(surface, m_transfer_queue_family_index.value())) {
        spdlog::trace("Using transfer queue for vkQueuePresentKHR");
        m_present_queue = m_transfer_queue;
    } else {
        if (!is_presentation_supported(surface, m_graphics_queue_family_index.value())) {
            // This should be extremely unlikely.
            throw std::runtime_error("Error: Graphics queue does not support present on GPU '" + m_gpu_name + "'");
        }
        spdlog::trace("Using graphics queue for vkQueuePresentKHR");
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

void Device::execute(const std::string &name, const VulkanQueueType queue_type,
                     const std::function<void(const CommandBuffer &cmd_buf)> &cmd_lambda) const {
    const auto &cmd_buf = get_thread_command_pool(queue_type).request_command_buffer(name);
    cmd_lambda(cmd_buf);
    cmd_buf.submit_and_wait();
}

CommandPool &Device::get_thread_command_pool(const VulkanQueueType queue_type) const {
    // Note that thread_local implies that thread_graphics_pool is implicitely static!
    thread_local CommandPool *thread_graphics_cmd_pool = nullptr; // NOLINT
    thread_local CommandPool *thread_compute_cmd_pool = nullptr;  // NOLINT
    thread_local CommandPool *thread_transfer_cmd_pool = nullptr; // NOLINT

    switch (queue_type) {
    case VulkanQueueType::QUEUE_TYPE_GRAPHICS: {
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
    case VulkanQueueType::QUEUE_TYPE_COMPUTE: {
        if (thread_compute_cmd_pool == nullptr) {
            if (!has_compute_queue()) {
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
    case VulkanQueueType::QUEUE_TYPE_TRANSFER: {
        if (thread_transfer_cmd_pool == nullptr) {
            if (!has_transfer_queue()) {
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
    }
    throw std::runtime_error("Error: Unknown VuklkanQueueType!");
}

const CommandBuffer &Device::request_command_buffer(const VulkanQueueType queue_type, const std::string &name) {
    return get_thread_command_pool(queue_type).request_command_buffer(name);
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
