#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include "inexor/vulkan-renderer/tools/device_info.hpp"
#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
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
        spdlog::error("Error: Selected GPU {} was evaluated as unsuitable!", gpu_info.name);

        // Attempt to pick another GPU automatically instead.
        m_physical_device = tools::pick_best_physical_device(inst, surface, required_features, required_extensions);
        spdlog::warn("GPU {} was selected automatically as alternative!",
                     tools::get_physical_device_name(m_physical_device));
    } else {
        // The desired GPU turned out to be suitable.
        spdlog::trace("Creating physical device using GPU {}", gpu_info.name);
        m_physical_device = desired_gpu;
    }

    // @TODO Migrate to separate code and write tests for finding the right queue family indices!

    spdlog::trace("Creating Vulkan queues");
    std::vector<VkDeviceQueueCreateInfo> queues_to_create;

    // Find a queue family index which supports VK_QUEUE_GRAPHICS_BIT.
    auto graphics_queue_family_index_candidate =
        find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
            // Check if the graphics flag is set for any queue.
            return is_presentation_supported(surface, index) && ((props.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u);
        });

    if (!graphics_queue_family_index_candidate) {
        // This should be extremely unlikely.
        throw std::runtime_error("Error: Could not find any queue with VK_QUEUE_GRAPHICS_BIT!");
    }
    // Select that candidate as queue family index for graphics.
    m_graphics_queue_family_index = graphics_queue_family_index_candidate.value();

    queues_to_create.push_back(make_info<VkDeviceQueueCreateInfo>({
        .queueFamilyIndex = m_graphics_queue_family_index,
        .queueCount = 1,
        .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
    }));

    // Find a queue family index which supports transfer but is not the queue family index for graphics!
    auto transfer_queue_family_index_candidate =
        find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
            return (props.queueFlags & VK_QUEUE_TRANSFER_BIT) && (index != m_graphics_queue_family_index);
        });

    if (!transfer_queue_family_index_candidate) {
        spdlog::warn("Failed to find a distinct transfer queue for GPU {}!", gpu_info.name);
        // This is not an error, so do not throw an exception in this case!
        // It simply means that we can't have a dedicated queue just for transfer which is not the graphics queue!
        // We try again by finding any queue that supports transfer, even if it not distinct from graphics queue.
        transfer_queue_family_index_candidate =
            find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
                return props.queueFlags & VK_QUEUE_TRANSFER_BIT;
            });
        if (!transfer_queue_family_index_candidate) {
            // This should be a very unlikely case, but still.
            spdlog::critical("Error: GPU {} does not have any transfer queue!", gpu_info.name);
            spdlog::warn("Attempting to use graphics queue as transfer queue instead.");
            // We still try to use the graphics queue in this case, which should work.
            transfer_queue_family_index_candidate = m_graphics_queue_family_index;
        }
        // We found a queue which supports transfer, but it is not distinct from the graphics queue.
    }

    m_transfer_queue_family_index = transfer_queue_family_index_candidate.value();

    if (m_transfer_queue_family_index != m_graphics_queue_family_index) {
        queues_to_create.push_back(make_info<VkDeviceQueueCreateInfo>({
            .queueFamilyIndex = m_transfer_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
        }));
    }

    // Find a queue family index which supports compute, but is neither graphics nor transfer queue!
    auto compute_queue_family_index_candidate =
        find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
            return (props.queueFlags & VK_QUEUE_COMPUTE_BIT) && (index != m_graphics_queue_family_index) &&
                   (index != m_transfer_queue_family_index);
        });

    if (!compute_queue_family_index_candidate) {
        spdlog::warn("Failed to find a distinct compute queue for GPU {}!", gpu_info.name);
        compute_queue_family_index_candidate =
            find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
                return (props.queueFlags & VK_QUEUE_COMPUTE_BIT);
            });
        if (!compute_queue_family_index_candidate) {
            spdlog::critical("Error: GPU {} Does not have any compute queue!", gpu_info.name);
            spdlog::warn("Attempting to use graphics queue as compute queue instead.");
            // We still try to use the graphics queue in this case, which should work.
            compute_queue_family_index_candidate = m_graphics_queue_family_index;
        }
        // We found a queue which supports compute, but it is not distinct!
    }

    m_compute_queue_family_index = compute_queue_family_index_candidate.value();

    if (m_compute_queue_family_index != m_graphics_queue_family_index) {
        queues_to_create.push_back(make_info<VkDeviceQueueCreateInfo>({
            .queueFamilyIndex = m_compute_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
        }));
    }

    // Find a queue family index which supports sparse binding, but is not graphics, transfer, or compute queue!
    auto sparse_binding_queue_family_index_candidate =
        find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
            return (props.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) &&
                   (index != graphics_queue_family_index_candidate) && (index != m_transfer_queue_family_index) &&
                   (index != m_compute_queue_family_index);
        });

    if (!sparse_binding_queue_family_index_candidate) {
        spdlog::warn("GPU {} doesn't have a distinct sparse binding queue!", gpu_info.name);
        sparse_binding_queue_family_index_candidate =
            find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &props) {
                return (props.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT);
            });
        if (!sparse_binding_queue_family_index_candidate) {
            spdlog::critical("Error: GPU {} Does not have any sparse binding queue!", gpu_info.name);
            spdlog::warn("Attempting to use graphics queue as compute queue instead.");
            // We still try to use the graphics queue in this case, which should work.
            sparse_binding_queue_family_index_candidate = m_graphics_queue_family_index;
        }
        // We found a queue which supports sparse binding, but it is not distinct!
    }

    m_sparse_binding_queue_family_index = sparse_binding_queue_family_index_candidate.value();

    if (m_sparse_binding_queue_family_index != m_graphics_queue_family_index) {
        queues_to_create.push_back(make_info<VkDeviceQueueCreateInfo>({
            .queueFamilyIndex = m_sparse_binding_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &DEFAULT_QUEUE_PRIORITY,
        }));
    }

    const auto device_ci = make_info<VkDeviceCreateInfo>({
        .queueCreateInfoCount = static_cast<std::uint32_t>(queues_to_create.size()),
        .pQueueCreateInfos = queues_to_create.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(required_extensions.size()),
        .ppEnabledExtensionNames = required_extensions.data(),
        .pEnabledFeatures = &m_enabled_features,
    });

    spdlog::trace("Creating physical device");
    if (const auto result = vkCreateDevice(m_physical_device, &device_ci, nullptr, &m_device); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDevice failed!", result);
    }

    spdlog::trace("Loading Vulkan dynamically using volk metaloader library");
    volkLoadDevice(m_device);

    spdlog::trace("Getting Vulkan queues from device");
    // Get the queues from the device that was just created.
    // It's important to call vkGetDeviceQueue after volkLoadDevice!
    vkGetDeviceQueue(m_device, m_graphics_queue_family_index, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_transfer_queue_family_index, 0, &m_transfer_queue);
    vkGetDeviceQueue(m_device, m_compute_queue_family_index, 0, &m_compute_queue);
    vkGetDeviceQueue(m_device, m_sparse_binding_queue_family_index, 0, &m_sparse_binding_queue);

    // Check if compute or transfer queue can be used for presenting
    if (is_presentation_supported(surface, m_compute_queue_family_index)) {
        spdlog::trace("Using compute queue for vkQueuePresentKHR");
        m_present_queue = m_compute_queue;
    } else if (is_presentation_supported(surface, m_transfer_queue_family_index)) {
        spdlog::trace("Using transfer queue for vkQueuePresentKHR");
        m_present_queue = m_transfer_queue;
    } else {
        // Note that we already checked earlier if graphics queue supports presentation.
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

std::optional<std::uint32_t> Device::find_queue_family_index_if(
    const std::function<bool(std::uint32_t index, const VkQueueFamilyProperties &)> &criteria_lambda) {
    for (std::uint32_t index = 0; const auto &queue_family : tools::get_queue_family_properties(m_physical_device)) {
        if (criteria_lambda(index, queue_family)) {
            return index;
        }
        index++;
    }
    return std::nullopt;
}

CommandPool &Device::get_thread_command_pool(const VulkanQueueType queue_type) const {
    // Note that thread_local implies that thread_graphics_pool is implicitely static!
    thread_local CommandPool *thread_graphics_cmd_pool = nullptr;       // NOLINT
    thread_local CommandPool *thread_compute_cmd_pool = nullptr;        // NOLINT
    thread_local CommandPool *thread_transfer_cmd_pool = nullptr;       // NOLINT
    thread_local CommandPool *thread_sparse_binding_cmd_pool = nullptr; // NOLINT

    switch (queue_type) {
    case VulkanQueueType::QUEUE_TYPE_GRAPHICS: {
        if (thread_graphics_cmd_pool == nullptr) {
            auto cmd_pool =
                std::make_unique<CommandPool>(*this, m_graphics_queue_family_index, "graphics command pool");
            std::unique_lock lock(m_mutex);
            thread_graphics_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_graphics_cmd_pool;
    }
    case VulkanQueueType::QUEUE_TYPE_COMPUTE: {
        if (thread_compute_cmd_pool == nullptr) {
            auto cmd_pool = std::make_unique<CommandPool>(*this, m_compute_queue_family_index, "compute command pool");
            std::unique_lock lock(m_mutex);
            thread_compute_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_compute_cmd_pool;
    }
    case VulkanQueueType::QUEUE_TYPE_TRANSFER: {
        if (thread_transfer_cmd_pool == nullptr) {
            auto cmd_pool =
                std::make_unique<CommandPool>(*this, m_transfer_queue_family_index, "transfer command pool");
            std::unique_lock lock(m_mutex);
            thread_transfer_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_transfer_cmd_pool;
    }
    case VulkanQueueType::QUEUE_TYPE_SPARSE_BINDING: {
        if (thread_sparse_binding_cmd_pool == nullptr) {
            auto cmd_pool = std::make_unique<CommandPool>(*this, m_sparse_binding_queue_family_index,
                                                          "sparse binding command pool");
            std::shared_lock lock(m_mutex);
            thread_sparse_binding_cmd_pool = m_cmd_pools.emplace_back(std::move(cmd_pool)).get();
        }
        std::shared_lock lock(m_mutex);
        return *thread_sparse_binding_cmd_pool;
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
