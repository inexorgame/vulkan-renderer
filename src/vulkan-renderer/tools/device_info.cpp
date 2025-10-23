#include "inexor/vulkan-renderer/tools/device_info.hpp"

#include "inexor/vulkan-renderer/tools/enumerate.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/representation.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <cstring>

namespace inexor::vulkan_renderer::tools {

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

    const auto extensions = get_extension_properties(physical_device);

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

bool compare_physical_devices(const VkPhysicalDeviceFeatures &required_features,
                              const std::span<const char *> required_extensions, const DeviceInfo &lhs,
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

std::uint32_t device_type_rating(const DeviceInfo &info) {
    switch (info.type) {
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return 2;
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return 1;
    default:
        return 0;
    }
}

std::vector<VkBool32> get_device_features_as_vector(const VkPhysicalDeviceFeatures &features) {
    std::vector<VkBool32> comparable_features(sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32));
    std::memcpy(comparable_features.data(), &features, sizeof(VkPhysicalDeviceFeatures));
    return comparable_features;
}

std::string get_physical_device_name(const VkPhysicalDevice physical_device) {
    assert(physical_device);
    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(physical_device, &properties);
    return properties.deviceName;
}

bool is_device_suitable(const DeviceInfo &info, const VkPhysicalDeviceFeatures &required_features,
                        const std::span<const char *> required_extensions, const bool print_info) {
    const auto comparable_required_features = get_device_features_as_vector(required_features);
    const auto comparable_available_features = get_device_features_as_vector(info.features);
    constexpr auto FEATURE_COUNT = sizeof(VkPhysicalDeviceFeatures) / sizeof(VkBool32);

    // Loop through all physical device features and check if a feature is required but not supported
    for (std::size_t i = 0; i < FEATURE_COUNT; i++) {
        if (comparable_required_features[i] == VK_TRUE && comparable_available_features[i] == VK_FALSE) {
            if (print_info) {
                spdlog::warn("Physical device {} does not support {}!", info.name, get_device_feature_description(i));
            }
            return false;
        }
    }
    // Loop through all device extensions and check if an extension is required but not supported
    for (const auto &extension : required_extensions) {
        if (!is_extension_supported(info.extensions, extension)) {
            if (print_info) {
                spdlog::warn("Physical device {} does not support extension {}!", info.name, extension);
            }
            return false;
        }
    }
    return info.presentation_supported && info.swapchain_supported;
}
bool is_extension_supported(const std::vector<VkExtensionProperties> &extensions, const std::string &extension_name) {
    return std::find_if(extensions.begin(), extensions.end(), [&](const VkExtensionProperties extension) {
               return extension.extensionName == extension_name;
           }) != extensions.end();
}

VkPhysicalDevice pick_best_physical_device(std::vector<DeviceInfo> &&physical_device_infos,
                                           const VkPhysicalDeviceFeatures &required_features,
                                           const std::span<const char *> required_extensions) {
    if (physical_device_infos.empty()) {
        throw std::runtime_error("Error: There are no physical devices available!");
    }
    std::sort(physical_device_infos.begin(), physical_device_infos.end(), [&](const auto &lhs, const auto &rhs) {
        return compare_physical_devices(required_features, required_extensions, lhs, rhs);
    });
    if (!is_device_suitable(physical_device_infos.front(), required_features, required_extensions, true)) {
        throw std::runtime_error("Error: Could not determine a suitable physical device!");
    }
    return physical_device_infos.front().physical_device;
}

VkPhysicalDevice pick_best_physical_device(const Instance &inst, const VkSurfaceKHR surface,
                                           const VkPhysicalDeviceFeatures &required_features,
                                           const std::span<const char *> required_extensions) {
    // Put together all data that is required to compare the physical devices
    const auto physical_devices = tools::get_physical_devices(inst.instance());
    std::vector<DeviceInfo> infos(physical_devices.size());
    std::transform(physical_devices.begin(), physical_devices.end(), infos.begin(),
                   [&](const VkPhysicalDevice physical_device) { return build_device_info(physical_device, surface); });
    return pick_best_physical_device(std::move(infos), required_features, required_extensions);
}

} // namespace inexor::vulkan_renderer::tools
