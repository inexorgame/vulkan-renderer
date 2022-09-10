#include "inexor/vulkan-renderer/vk_tools/enumerate.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::vk_tools {

std::vector<VkExtensionProperties> get_all_device_extension_properties(const VkPhysicalDevice physical_device) {
    assert(physical_device);
    // Query how many device extensions are available for the physical device
    std::uint32_t count = 0;
    // Because device layers are deprecated, we will never fill the pLayerName parameter
    if (const auto result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }
    /// Query device extension properties of the physical device
    std::vector<VkExtensionProperties> extensions(count);
    if (const auto result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }
    return std::move(extensions);
}

std::vector<VkPhysicalDevice> get_all_physical_devices(const VkInstance inst) {
    assert(inst);
    // Query how many physical devices are available on the system
    std::uint32_t count = 0;
    if (const auto result = vkEnumeratePhysicalDevices(inst, &count, nullptr); result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
    }
    // Query physical devices of the system
    std::vector<VkPhysicalDevice> physical_devices(count);
    if (const auto result = vkEnumeratePhysicalDevices(inst, &count, physical_devices.data()); result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
    }
    return std::move(physical_devices);
}

} // namespace inexor::vulkan_renderer::vk_tools
