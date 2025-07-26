#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/wrapper/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/surface.hpp"

#define VMA_IMPLEMENTATION

#include <spdlog/spdlog.h>
#include <vk_mem_alloc.h>

#include <algorithm>
#include <cassert>
#include <fstream>

namespace {

// TODO: Make proper use of queue priorities in the future
constexpr float DEFAULT_QUEUE_PRIORITY = 1.0f;

} // namespace

namespace inexor::vulkan_renderer::wrapper {

// TODO: Refactor to use VkClearColor to have a general color palette
// TODO: Use std::unordered_map instead!
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

namespace {

/// A function for rating physical devices by type
/// @param info The physical device info
/// @return A number from 0 to 2 which rates the physical device (higher is better)
std::uint32_t device_type_rating(const DeviceInfo &info) {
    switch (info.type) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return 2;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return 1;
    default:
        // This means we ignore any other gpu types by default
        return 0;
    }
}

/// Check if a device extension is supported by a physical device
/// @param extensions The device extensions
/// @note If extensions is empty, this function returns ``false``
/// @param extension_name The extension name
/// @return ``true`` if the required device extension is supported
bool is_extension_supported(const std::vector<VkExtensionProperties> &extensions, const std::string &extension_name) {
    return std::find_if(extensions.begin(), extensions.end(), [&](const VkExtensionProperties extension) {
               return std::string(extension.extensionName) == extension_name;
           }) != extensions.end();
}

/// Determine if a physical device is suitable. In order for a physical device to be suitable, it must support all
/// required device features and required extensions.
/// @param info The device info data
/// @param required_features The required device features the physical device must all support
/// @param required_extensions The required device extensions the physical device must all support
/// @param print_message If ``true``, an info message will be printed to the console if a device feature or device
/// extension is not supported (``true`` by default)
/// @return ``true`` if the physical device supports all device features and device extensions
bool is_device_suitable(const DeviceInfo &info,
                        const VkPhysicalDeviceFeatures &required_features,
                        const std::span<const char *> required_extensions,
                        const bool print_info = false) {
    const auto comparable_required_features = tools::get_device_features_as_vector(required_features);
    const auto comparable_available_features = tools::get_device_features_as_vector(info.features);
    constexpr auto FEATURE_COUNT = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);

    // Loop through all physical device features and check if a feature is required but not supported
    for (std::size_t i = 0; i < FEATURE_COUNT; i++) {
        if (comparable_required_features[i] == VK_TRUE && comparable_available_features[i] == VK_FALSE) {
            if (print_info) {
                spdlog::error("Physical device {} does not support {}!", info.name,
                              tools::get_device_feature_description(i));
            }
            return false;
        }
    }
    // Loop through all device extensions and check if an extension is required but not supported
    // We are not checking for duplicated entries but this is not a problem
    for (const auto &extension : required_extensions) {
        if (!is_extension_supported(info.extensions, extension)) {
            if (print_info) {
                spdlog::error("Physical device {} does not support extension {}!", info.name, extension);
            }
            return false;
        }
    }
    return info.presentation_supported && info.swapchain_supported;
}

/// Compare two physical devices and determine which one is preferable
/// @param required_features The required device features which must all be supported by the physical device
/// @param required_extensions The required device extensions which must all be supported by the physical device
/// @param lhs A physical device to compare with the other one
/// @param rhs The other physical device
/// @return ``true`` if `lhs` is more preferable over `rhs`
bool compare_physical_devices(const VkPhysicalDeviceFeatures &required_features,
                              const std::span<const char *> required_extensions,
                              const DeviceInfo &lhs,
                              const DeviceInfo &rhs) {
    if (!is_device_suitable(rhs, required_features, required_extensions)) {
        return true;
    }
    if (!is_device_suitable(lhs, required_features, required_extensions)) {
        return false;
    }
    if (device_type_rating(lhs) > device_type_rating(rhs)) {
        return true;
    }
    if (device_type_rating(lhs) < device_type_rating(rhs)) {
        return false;
    }
    // Device types equal, compare total amount of DEVICE_LOCAL memory
    return lhs.total_device_local >= rhs.total_device_local;
}

/// Build DeviceInfo from a real vulkan physical device (as opposed to a fake one used in the tests).
/// @param physical_device The physical device
/// @param surface The window surface
/// @return The device info data
DeviceInfo build_device_info(const VkPhysicalDevice physical_device, const VkSurfaceKHR surface) {
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);

    VkPhysicalDeviceMemoryProperties memory_properties{};
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    VkPhysicalDeviceFeatures features{};
    vkGetPhysicalDeviceFeatures(physical_device, &features);

    VkDeviceSize total_device_local = 0;
    for (std::size_t i = 0; i < memory_properties.memoryHeapCount; i++) {
        const auto &heap = memory_properties.memoryHeaps[i];
        if ((heap.flags & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT) != 0) {
            total_device_local += heap.size;
        }
    }

    // Default to true in this case where a surface is not passed (and therefore presentation isn't cared about)
    VkBool32 presentation_supported = VK_TRUE;
    if (surface != nullptr) {
        if (const auto result =
                vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, 0, surface, &presentation_supported);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkGetPhysicalDeviceSurfaceSupportKHR failed!", result);
        }
    }

    const auto extensions = tools::get_extension_properties(physical_device);

    const bool is_swapchain_supported =
        surface == nullptr || is_extension_supported(extensions, VK_KHR_SWAPCHAIN_EXTENSION_NAME);

    return DeviceInfo{
        .name = properties.deviceName,
        .physical_device = physical_device,
        .type = properties.deviceType,
        .total_device_local = total_device_local,
        .features = features,
        .extensions = extensions,
        .presentation_supported = presentation_supported == VK_TRUE,
        .swapchain_supported = is_swapchain_supported,
    };
}

} // namespace

VkPhysicalDevice Device::pick_best_physical_device(std::vector<DeviceInfo> &&physical_device_infos,
                                                   const VkPhysicalDeviceFeatures &required_features,
                                                   const std::span<const char *> required_extensions) {
    if (physical_device_infos.empty()) {
        throw InexorException("Error: There are no physical devices available!");
    }
    std::sort(physical_device_infos.begin(), physical_device_infos.end(), [&](const auto &lhs, const auto &rhs) {
        return compare_physical_devices(required_features, required_extensions, lhs, rhs);
    });
    if (!is_device_suitable(physical_device_infos.front(), required_features, required_extensions, true)) {
        throw InexorException("Error: Could not determine a suitable physical device!");
    }
    return physical_device_infos.front().physical_device;
}

VkPhysicalDevice Device::pick_best_physical_device(const Instance &instance,
                                                   const Surface &surface,
                                                   const VkPhysicalDeviceFeatures &required_features,
                                                   const std::span<const char *> required_extensions) {
    // Put together all data that is required to compare the physical devices
    const auto physical_devices = instance.get_physical_devices();
    std::vector<DeviceInfo> infos(physical_devices.size());
    std::transform(
        physical_devices.begin(), physical_devices.end(), infos.begin(),
        [&](const VkPhysicalDevice physical_device) { return build_device_info(physical_device, surface.m_surface); });
    return pick_best_physical_device(std::move(infos), required_features, required_extensions);
}

Device::Device(const Instance &inst,
               const Surface &surface,
               const VkPhysicalDevice physical_device,
               const std::span<const char *> required_extensions,
               const VkPhysicalDeviceFeatures &required_features)
    : m_physical_device(physical_device) {
    const auto device_info = build_device_info(physical_device, surface.m_surface);
    if (!is_device_suitable(device_info, required_features, required_extensions, true)) {
        throw InexorException("Error: The chosen physical device " + device_info.name + " is not suitable!");
    }

    spdlog::trace("Creating Vulkan device queues");
    std::vector<VkDeviceQueueCreateInfo> queues_to_create;

    // Check if there is one queue family which can be used for both graphics and presentation
    auto queue_candidate =
        find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &queue_family) {
            return is_presentation_supported(surface.m_surface, index) &&
                   (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0u;
        });

    if (!queue_candidate) {
        throw InexorException("Error: Could not find a queue for both graphics and presentation!");
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
            return is_presentation_supported(surface.m_surface, index) &&
                   // A queue for data transfer only: no graphics bit, only transfer bit!
                   (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0u &&
                   (queue_family.queueFlags & VK_QUEUE_TRANSFER_BIT) != 0u;
        });

    m_transfer_queue_family_index = m_graphics_queue_family_index;

    VkPhysicalDeviceFeatures available_features{};
    vkGetPhysicalDeviceFeatures(physical_device, &available_features);

    // We want to use dynamic rendering (VK_KHR_dynamic_rendering)
    const auto dyn_rendering_feature = make_info<VkPhysicalDeviceDynamicRenderingFeaturesKHR>({
        .pNext = nullptr,
        .dynamicRendering = VK_TRUE,
    });

    const auto device_ci = make_info<VkDeviceCreateInfo>({
        // This is one of those rare cases where pNext is actually not nullptr!
        .pNext = &dyn_rendering_feature, // We use dynamic rendering
        .queueCreateInfoCount = static_cast<std::uint32_t>(queues_to_create.size()),
        .pQueueCreateInfos = queues_to_create.data(),
        .enabledExtensionCount = static_cast<std::uint32_t>(required_extensions.size()),
        .ppEnabledExtensionNames = required_extensions.data(),
        .pEnabledFeatures = &m_enabled_features,
    });

    m_enabled_features = required_features;

    spdlog::trace("Creating physical device using graphics card: {}", device_info.name);

    if (const auto result = vkCreateDevice(m_physical_device, &device_ci, nullptr, &m_device); result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDevice failed!", result);
    }

    set_debug_name(m_device, "Device");

    spdlog::trace("Loading Vulkan entrypoints directly from driver with volk metaloader");
    volkLoadDevice(m_device);

    // TODO: Refactor: Compute queue but no graphics queue? (Refactor selection)
    auto compute_queue_candidate =
        find_queue_family_index_if([&](const std::uint32_t index, const VkQueueFamilyProperties &queue_family) {
            return (queue_family.queueFlags & VK_QUEUE_COMPUTE_BIT) != 0u;
        });

    if (!compute_queue_candidate) {
        throw InexorException("Error: Could not find a compute queue!");
    }

    m_compute_queue_family_index = compute_queue_candidate.value();

    spdlog::trace("Queue family indices:");
    spdlog::trace("   - Graphics: {}", m_graphics_queue_family_index);
    spdlog::trace("   - Present: {}", m_present_queue_family_index);
    spdlog::trace("   - Transfer: {}", m_transfer_queue_family_index);
    spdlog::trace("   - Compute: {}", m_compute_queue_family_index);

    vkGetDeviceQueue(m_device, m_present_queue_family_index, 0, &m_present_queue);
    vkGetDeviceQueue(m_device, m_graphics_queue_family_index, 0, &m_graphics_queue);
    vkGetDeviceQueue(m_device, m_compute_queue_family_index, 0, &m_compute_queue);
    vkGetDeviceQueue(m_device, m_transfer_queue_family_index, 0, &m_transfer_queue);

    set_debug_name(m_graphics_queue, "Graphics Queue");
    set_debug_name(m_present_queue, "Present Queue");
    set_debug_name(m_compute_queue, "Compute Queue");
    set_debug_name(m_transfer_queue, "Transfer Queue");

    spdlog::trace("Creating VMA allocator");

    const VmaVulkanFunctions vma_vulkan_functions{
        .vkGetInstanceProcAddr = vkGetInstanceProcAddr,
        .vkGetDeviceProcAddr = vkGetDeviceProcAddr,
    };
    const VmaAllocatorCreateInfo vma_allocator_ci{
        .physicalDevice = m_physical_device,
        .device = m_device,
        .pVulkanFunctions = &vma_vulkan_functions,
        .instance = inst.m_instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
    };

    spdlog::trace("Creating Vulkan Memory Allocator (VMA) instance");

    if (const auto result = vmaCreateAllocator(&vma_allocator_ci, &m_allocator); result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateAllocator failed!", result);
    }

    // Store the properties of this physical device
    vkGetPhysicalDeviceProperties(m_physical_device, &m_properties);

    auto determine_max_usable_sample_count = [&]() {
        const auto sample_count =
            m_properties.limits.framebufferColorSampleCounts & m_properties.limits.framebufferDepthSampleCounts;

        const VkSampleCountFlagBits sample_count_flag_bits[] = {
            VK_SAMPLE_COUNT_64_BIT, VK_SAMPLE_COUNT_32_BIT, VK_SAMPLE_COUNT_16_BIT,
            VK_SAMPLE_COUNT_8_BIT,  VK_SAMPLE_COUNT_4_BIT,  VK_SAMPLE_COUNT_2_BIT,
        };

        for (const auto &sample_count_flag_bit : sample_count_flag_bits) {
            if (sample_count & sample_count_flag_bit) {
                return sample_count_flag_bit;
            }
        }
        return VK_SAMPLE_COUNT_1_BIT;
    };

    m_max_available_sample_count = determine_max_usable_sample_count();
}

Device::~Device() {
    std::scoped_lock locker(m_mutex);
    wait_idle();

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

void Device::execute(const std::string &name,
                     const VkQueueFlagBits queue_type,
                     const DebugLabelColor dbg_label_color,
                     const std::function<void(const CommandBuffer &cmd_buf)> &cmd_buf_recording_func,
                     const std::span<const VkSemaphore> wait_semaphores,
                     const std::span<const VkSemaphore> signal_semaphores) const {
    // Request the thread_local command pool for this queue type
    const auto &cmd_pool = thread_local_command_pool(queue_type);
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

const CommandPool &Device::thread_local_command_pool(const VkQueueFlagBits queue_type) const {
    // NOTE: thread_local means implicitly static!
    thread_local CommandPool *cmd_pool_graphics = nullptr; // NOLINT
    thread_local CommandPool *cmd_pool_transfer = nullptr; // NOLINT
    thread_local CommandPool *cmd_pool_compute = nullptr;  // NOLINT

    switch (queue_type) {
    case VK_QUEUE_COMPUTE_BIT: {
        if (cmd_pool_compute == nullptr) {
            auto new_cmd_pool = std::make_unique<CommandPool>(*this, VK_QUEUE_COMPUTE_BIT, "Compute");
            std::scoped_lock locker(m_mutex);
            cmd_pool_compute = m_cmd_pools.emplace_back(std::move(new_cmd_pool)).get();
        }
        return *cmd_pool_compute;
    }
    case VK_QUEUE_TRANSFER_BIT: {
        if (cmd_pool_transfer == nullptr) {
            auto new_cmd_pool = std::make_unique<CommandPool>(*this, VK_QUEUE_TRANSFER_BIT, "Transfer");
            std::scoped_lock locker(m_mutex);
            cmd_pool_transfer = m_cmd_pools.emplace_back(std::move(new_cmd_pool)).get();
        }
        return *cmd_pool_transfer;
    }
    default: {
        // VK_QUEUE_GRAPHICS_BIT and others
        if (cmd_pool_graphics == nullptr) {
            auto new_cmd_pool = std::make_unique<CommandPool>(*this, VK_QUEUE_GRAPHICS_BIT, "Graphics");
            std::scoped_lock locker(m_mutex);
            cmd_pool_graphics = m_cmd_pools.emplace_back(std::move(new_cmd_pool)).get();
        }
        return *cmd_pool_graphics;
    }
    }
}

void Device::set_debug_utils_object_name(const VkObjectType obj_type,
                                         const std::uint64_t obj_handle,
                                         const std::string &name) const {
    if (!obj_handle) {
        throw InexorException("Parameter 'obj_handle' is invalid!");
    }
    const auto dbg_obj_name = wrapper::make_info<VkDebugUtilsObjectNameInfoEXT>({
        .objectType = obj_type,
        .objectHandle = obj_handle,
        .pObjectName = name.c_str(),
    });

    if (const auto result = vkSetDebugUtilsObjectNameEXT(m_device, &dbg_obj_name); result != VK_SUCCESS) {
        throw VulkanException("Error: vkSetDebugUtilsObjectNameEXT failed!", result);
    }
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
