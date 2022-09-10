#include "inexor/vulkan-renderer/vk_tools/get_info.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::vk_tools {

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

} // namespace inexor::vulkan_renderer::vk_tools
