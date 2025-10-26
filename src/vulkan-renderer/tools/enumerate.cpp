#include "inexor/vulkan-renderer/tools/enumerate.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"

#include <cassert>
#include <cstdint>
#include <utility>

namespace inexor::vulkan_renderer::tools {

std::vector<VkExtensionProperties> get_extension_properties(const VkPhysicalDevice physical_device) {
    if (physical_device == VK_NULL_HANDLE) {
        throw tools::InexorException("Error: Parameter 'physical_device' is invalid!");
    }
    std::vector<VkExtensionProperties> extension_props;
    std::uint32_t device_extension_count = 0;
    if (const auto result =
            vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count, nullptr);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
    }
    if (device_extension_count != 0) {
        // We must call resize here, not reserve!
        extension_props.resize(device_extension_count);
        if (const auto result = vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &device_extension_count,
                                                                     extension_props.data());
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkEnumerateDeviceExtensionProperties failed!", result);
        }
    }
    return extension_props;
}

std::vector<VkExtensionProperties> get_instance_extensions() {
    static std::vector<VkExtensionProperties> instance_extensions;
    static bool already_initialized = false;
    if (instance_extensions.empty() && !already_initialized) {
        if (vkEnumerateInstanceExtensionProperties == nullptr) {
            throw InexorException("Error: vkEnumerateInstanceExtensionProperties is not available!");
        }
        std::uint32_t instance_extension_count = 0;
        if (const auto result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkEnumerateInstanceExtensionProperties failed!", result);
        }
        if (instance_extension_count == 0) {
            // This is not an error. Some platforms simply don't have any instance extensions.
            // In this case, we set this bool so that the enumeration is not checked unnecessarily again.
            // We don't need to set this to true because in the other case, the vector will be filled with data.
            already_initialized = true;
        } else {
            // We must call resize here, not reserve!
            instance_extensions.resize(instance_extension_count);
            // Store all available instance extensions.
            if (const auto result = vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count,
                                                                           instance_extensions.data());
                result != VK_SUCCESS) {
                throw VulkanException("Error: vkEnumerateInstanceExtensionProperties failed!", result);
            }
        }
    }
    return instance_extensions;
}

std::vector<VkLayerProperties> get_instance_layers() {
    static std::vector<VkLayerProperties> instance_layers;
    static bool already_initialized = false;
    if (instance_layers.empty() && !already_initialized) {
        if (vkEnumerateInstanceLayerProperties == nullptr) {
            throw InexorException("Error: vkEnumerateInstanceLayerProperties is not available!");
        }
        std::uint32_t instance_layer_count = 0;
        if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkEnumerateInstanceLayerProperties failed!", result);
        }
        if (instance_layer_count == 0) {
            // This is not an error. Some platforms simply don't have any instance layers.
            // In this case, we set this bool so that the enumeration is not checked unnecessarily again.
            // We don't need to set this to true because in the other case, the vector will be filled with data.
            already_initialized = true;
        } else {
            // We need to call resize here, not reserve!
            instance_layers.resize(instance_layer_count);
            // Store all available instance layers.
            if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layers.data());
                result != VK_SUCCESS) {
                throw VulkanException("Error: vkEnumerateInstanceLayerProperties failed!", result);
            }
        }
    }
    return instance_layers;
}

std::vector<VkPhysicalDevice> get_physical_devices(const VkInstance inst) {
    if (inst == VK_NULL_HANDLE) {
        throw tools::InexorException("Error: Parameter 'inst' is invalid!");
    }
    std::uint32_t physical_device_count = 0;
    std::vector<VkPhysicalDevice> physical_devices;
    if (const auto result = vkEnumeratePhysicalDevices(inst, &physical_device_count, nullptr); result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
    }
    if (physical_device_count != 0) {
        // We must call resize here, not reserve!
        physical_devices.resize(physical_device_count);
        if (const auto result = vkEnumeratePhysicalDevices(inst, &physical_device_count, physical_devices.data());
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkEnumeratePhysicalDevices failed!", result);
        }
    }
    return physical_devices;
}

std::vector<VkQueueFamilyProperties> get_queue_family_properties(const VkPhysicalDevice physical_device) {
    std::vector<VkQueueFamilyProperties> queue_families;
    if (physical_device == VK_NULL_HANDLE) {
        throw tools::InexorException("Error: Parameter 'physical_device' is invalid!");
    }
    std::uint32_t props_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &props_count, nullptr);
    if (props_count != 0) {
        // We must call resize here, not reserve!
        queue_families.resize(props_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &props_count, queue_families.data());
    }
    return queue_families;
}

std::vector<VkSurfaceFormatKHR> get_surface_formats(const VkPhysicalDevice physical_device,
                                                    const VkSurfaceKHR surface) {
    if (physical_device == VK_NULL_HANDLE) {
        throw tools::InexorException("Error: Parameter 'physical_device' is invalid!");
    }
    if (surface == VK_NULL_HANDLE) {
        throw tools::InexorException("Error: Parameter 'physical_device' is invalid!");
    }
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
    if (physical_device == VK_NULL_HANDLE) {
        throw tools::InexorException("Error: Parameter 'physical_device' is invalid!");
    }
    if (surface == VK_NULL_HANDLE) {
        throw tools::InexorException("Error: Parameter 'surface' is invalid!");
    }
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
