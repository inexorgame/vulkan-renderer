#include "inexor/vulkan-renderer/tools/enumerate.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

#include <cassert>
#include <cstdint>
#include <utility>

namespace inexor::vulkan_renderer::tools {

std::vector<VkExtensionProperties> get_extension_properties(const VkPhysicalDevice physical_device) {
    assert(physical_device);
    std::uint32_t count = 0;
    if (const auto result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }
    std::vector<VkExtensionProperties> extensions(count);
    if (const auto result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &count, extensions.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }
    return extensions;
}

std::vector<VkPhysicalDevice> get_physical_devices(const VkInstance inst) {
    assert(inst);
    std::uint32_t count = 0;
    if (const auto result = vkEnumeratePhysicalDevices(inst, &count, nullptr); result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
    }
    std::vector<VkPhysicalDevice> physical_devices(count);
    if (const auto result = vkEnumeratePhysicalDevices(inst, &count, physical_devices.data()); result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
    }
    return physical_devices;
}

std::vector<VkQueueFamilyProperties> get_queue_family_properties(const VkPhysicalDevice physical_device) {
    assert(physical_device);
    std::uint32_t count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &count, queue_families.data());
    return queue_families;
}

std::vector<VkSurfaceFormatKHR> get_surface_formats(const VkPhysicalDevice physical_device,
                                                    const VkSurfaceKHR surface) {
    assert(physical_device);
    std::uint32_t count = 0;
    if (const auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceFormatsKHR failed!", result);
    }
    std::vector<VkSurfaceFormatKHR> surface_formats(count);
    if (const auto result =
            vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &count, surface_formats.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfaceFormatsKHR failed!", result);
    }
    return surface_formats;
}

std::vector<VkPresentModeKHR> get_surface_present_modes(const VkPhysicalDevice physical_device,
                                                        const VkSurfaceKHR surface) {
    assert(physical_device);
    std::uint32_t count = 0;
    if (const auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfacePresentModesKHR failed!", result);
    }
    std::vector<VkPresentModeKHR> present_modes(count);
    if (const auto result =
            vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &count, present_modes.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkGetPhysicalDeviceSurfacePresentModesKHR failed!", result);
    }
    return present_modes;
}

} // namespace inexor::vulkan_renderer::tools
