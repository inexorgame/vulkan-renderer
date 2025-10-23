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

namespace {

// TODO: Make proper use of queue priorities in the future.
constexpr float DEFAULT_QUEUE_PRIORITY = 1.0f;

} // namespace

namespace inexor::vulkan_renderer::wrapper {

Device::Device(const Instance &inst, const VkSurfaceKHR surface, const bool prefer_distinct_transfer_queue,
               const VkPhysicalDevice physical_device, const std::span<const char *> required_extensions,
               const VkPhysicalDeviceFeatures &required_features, const VkPhysicalDeviceFeatures &optional_features)
    : m_physical_device(physical_device) {

    const auto device_info = tools::build_device_info(physical_device, surface);

    if (!is_device_suitable(device_info, required_features, required_extensions, true)) {
        throw std::runtime_error("Error: The chosen physical device " + device_info.name + " is not suitable!");
    }

    m_gpu_name = tools::get_physical_device_name(m_physical_device);
    spdlog::trace("Creating device using graphics card: {}", m_gpu_name);

    spdlog::trace("Creating Vulkan device queues");
    std::vector<VkDeviceQueueCreateInfo> queues_to_create;

    if (prefer_distinct_transfer_queue) {
        spdlog::trace("The application will try to use a distinct data transfer queue if it is available");
    } else {
        spdlog::warn("The application is forced not to use a distinct data transfer queue!");
    }

    // Check if there is one queue family which can be used for both graphics and presentation
    auto queue_candidate =
        find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &queue_family) {
            return is_presentation_supported(surface, index) && (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u;
        });

    if (!queue_candidate) {
        throw std::runtime_error("Error: Could not find a queue for both graphics and presentation!");
    }

    spdlog::trace("One queue for both graphics and presentation will be used");

    m_graphics_queue_family_index = *queue_candidate;
    m_present_queue_family_index = m_graphics_queue_family_index;

    // In this case, there is one queue family which can be used for both graphics and presentation
    queues_to_create.push_back(make_info<VkDeviceQueueCreateInfo>({
        .queueFamilyIndex = *queue_candidate,
        .queueCount = 1,
        .pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY,
    }));

    // Add another device queue just for data transfer
    queue_candidate =
        find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &queue_family) {
            return is_presentation_supported(surface, index) &&
                   // No graphics bit, only transfer bit
                   (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0u &&
                   (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u;
        });

    bool use_distinct_data_transfer_queue = false;

    if (queue_candidate && prefer_distinct_transfer_queue) {
        m_transfer_queue_family_index = *queue_candidate;

        spdlog::trace("A separate queue will be used for data transfer.");

        // We have the opportunity to use a separated queue for data transfer!
        use_distinct_data_transfer_queue = true;

        queues_to_create.push_back(make_info<VkDeviceQueueCreateInfo>({
            .queueFamilyIndex = m_transfer_queue_family_index,
            .queueCount = 1,
            .pQueuePriorities = &::DEFAULT_QUEUE_PRIORITY,
        }));
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

    VkPhysicalDeviceFeatures available_features{};
    vkGetPhysicalDeviceFeatures(physical_device, &available_features);

    const auto comparable_required_features = tools::get_device_features_as_vector(required_features);
    const auto comparable_optional_features = tools::get_device_features_as_vector(optional_features);
    const auto comparable_available_features = tools::get_device_features_as_vector(available_features);

    constexpr auto FEATURE_COUNT = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);
    std::vector<VkBool32> features_to_enable(FEATURE_COUNT, VK_FALSE);

    for (std::size_t i = 0; i < FEATURE_COUNT; i++) {
        if (comparable_required_features[i] == VK_TRUE) {
            features_to_enable[i] = VK_TRUE;
        }
        if (comparable_optional_features[i] == VK_TRUE) {
            if (comparable_available_features[i] == VK_TRUE) {
                features_to_enable[i] = VK_TRUE;
            } else {
                spdlog::warn("The physical device {} does not support {}!",
                             tools::get_physical_device_name(physical_device),
                             tools::get_device_feature_description(i));
            }
        }
    }

    std::memcpy(&m_enabled_features, features_to_enable.data(), features_to_enable.size());

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

    spdlog::trace("Loading Vulkan entrypoints directly from driver (bypass Vulkan loader dispatch code)");
    volkLoadDevice(m_device);

    const bool enable_debug_markers =
        std::find_if(required_extensions.begin(), required_extensions.end(), [&](const char *extension) {
            return std::string(extension) == std::string(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
        }) != required_extensions.end();

    if (enable_debug_markers) {
        spdlog::trace("Initializing Vulkan debug markers");
    }

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

    VmaVulkanFunctions vma_vulkan_functions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
    };

    const VmaAllocatorCreateInfo vma_allocator_ci{
        .physicalDevice = m_physical_device,
        .device = m_device,
        .pVulkanFunctions = &vma_vulkan_functions,
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

Device::Device(Device &&other) noexcept : m_cmd_pools(std::move(other.m_cmd_pools)) {
    // After moving the cmd_pools, the memory is in a valid but unspecified state.
    // By calling .clear(), we bring it back into a defined state again.
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

bool Device::format_supports_feature(const VkFormat format, const VkFormatFeatureFlagBits feature) const {
    VkFormatProperties properties{};
    vkGetPhysicalDeviceFormatProperties(m_physical_device, format, &properties);
    return (properties.optimalTilingFeatures & feature) != 0u;
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
    for (std::uint32_t index = 0; const auto queue_family : tools::get_queue_family_properties(m_physical_device)) {
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
