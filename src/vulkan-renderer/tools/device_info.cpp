#include "inexor/vulkan-renderer/tools/device_info.hpp"

#include <cassert>
#include <cstring>

namespace inexor::vulkan_renderer::tools {

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

} // namespace inexor::vulkan_renderer::tools
