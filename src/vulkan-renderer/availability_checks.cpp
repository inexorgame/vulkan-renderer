#include "inexor/vulkan-renderer/availability_checks.hpp"

#include "inexor/vulkan-renderer/error_handling.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <stdexcept>

namespace inexor::vulkan_renderer {

VkResult AvailabilityChecksManager::create_instance_extensions_cache() {
    // First ask Vulkan how many instance extensions are available on the system.
    VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &m_available_instance_extensions, nullptr);
    vulkan_error_check(result);

    if (m_available_instance_extensions == 0) {
        // It should be a very rare case that no instance extensions are available at all. Still we have to consider
        // this!
        throw std::runtime_error("Error: No Vulkan instance extensions available!");
    } else {
        // Preallocate memory for extension properties.
        m_instance_extensions_cache.resize(m_available_instance_extensions);

        // Get the information about the available instance extensions.
        result = vkEnumerateInstanceExtensionProperties(nullptr, &m_available_instance_extensions,
                                                        m_instance_extensions_cache.data());
        vulkan_error_check(result);
    }

    return VK_SUCCESS;
}

bool AvailabilityChecksManager::has_instance_extension(const std::string &instance_extension_name) {
    assert(!instance_extension_name.empty());

    if (m_instance_extensions_cache.empty()) {
        create_instance_extensions_cache();
    }

    // Loop through all available instance extensions and search for the requested one.
    auto result =
        std::find_if(m_instance_extensions_cache.begin(), m_instance_extensions_cache.end(),
                     [&](const VkExtensionProperties &instance_extension) {
                         // Compare the name of the current instance extension with the requested one.
                         return (strcmp(instance_extension.extensionName, instance_extension_name.c_str()) == 0);
                     });

    // True if instance extension was found and is supported!
    return result != m_instance_extensions_cache.end();
}

VkResult AvailabilityChecksManager::create_instance_layers_cache() {
    // First ask Vulkan how many instance layers are available on the system.
    VkResult result = vkEnumerateInstanceLayerProperties(&m_available_instance_layers, nullptr);
    vulkan_error_check(result);

    if (m_available_instance_layers == 0) {
        // It should be a very rare case that no instance layers are available at all. Still we have to consider this!
        throw std::runtime_error("Error: No Vulkan instance layers available!");
    }
    // Preallocate memory for layer properties.
    m_instance_layers_cache.resize(m_available_instance_layers);

    // Get the information about the available instance layers.
    result = vkEnumerateInstanceLayerProperties(&m_available_instance_layers, m_instance_layers_cache.data());
    vulkan_error_check(result);

    return VK_SUCCESS;
}

bool AvailabilityChecksManager::has_instance_layer(const std::string &instance_layer_name) {
    assert(!instance_layer_name.empty());

    if (m_instance_layers_cache.empty()) {
        create_instance_layers_cache();
    }

    // Loop through all available instance layers and search for the requested one.
    auto result = std::find_if(m_instance_layers_cache.begin(), m_instance_layers_cache.end(),
                               [&](const VkLayerProperties &instance_layer) {
                                   // Compare the name of the current instance layer with the requested one.
                                   return (strcmp(instance_layer.layerName, instance_layer_name.c_str()) == 0);
                               });

    // True if instance layer was found and is supported!
    return result != m_instance_layers_cache.end();
}

VkResult AvailabilityChecksManager::create_device_layers_cache(const VkPhysicalDevice &graphics_card) {
    // First ask Vulkan how many device layers are available on the system.
    VkResult result = vkEnumerateDeviceLayerProperties(graphics_card, &m_available_device_layers, nullptr);
    vulkan_error_check(result);

    if (m_available_device_layers == 0) {
        // It should be a very rare case that no device layers are available at all. Still we have to consider this!
        throw std::runtime_error("Error: No Vulkan device layers available!");
    }
    // Preallocate memory for device layers.
    m_device_layer_properties_cache.resize(m_available_device_layers);

    // Get the information about the available device layers.
    result = vkEnumerateDeviceLayerProperties(graphics_card, &m_available_device_layers,
                                              m_device_layer_properties_cache.data());
    vulkan_error_check(result);

    return VK_SUCCESS;
}

bool AvailabilityChecksManager::has_device_layer(const VkPhysicalDevice &graphics_card,
                                                 const std::string &device_layer_name) {
    assert(graphics_card);
    assert(!device_layer_name.empty());

    if (m_device_layer_properties_cache.empty()) {
        create_device_layers_cache(graphics_card);
    }

    // Loop through all available device layers and search for the requested one.
    auto result = std::find_if(m_device_layer_properties_cache.begin(), m_device_layer_properties_cache.end(),
                               [&](const VkLayerProperties &device_layer) {
                                   return (strcmp(device_layer.layerName, device_layer_name.c_str()) == 0);
                               });

    // True if device layer was found and is supported!
    return result != m_device_layer_properties_cache.end();
}

VkResult AvailabilityChecksManager::create_device_extensions_cache(const VkPhysicalDevice &graphics_card) {
    // First ask Vulkan how many device extensions are available on the system.
    VkResult result =
        vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &m_available_device_extensions, nullptr);
    vulkan_error_check(result);

    if (0 == m_available_device_extensions) {
        // It should be a very rare case that no device extension are available at all. Still we have to consider this!
        throw std::runtime_error("Error: No Vulkan device extensions available!");
    } else {
        // Preallocate memory for device extensions.
        m_device_extensions_cache.resize(m_available_device_extensions);

        // Get the information about the available device extensions.
        result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &m_available_device_extensions,
                                                      m_device_extensions_cache.data());
        vulkan_error_check(result);
    }

    return VK_SUCCESS;
}

bool AvailabilityChecksManager::has_device_extension(const VkPhysicalDevice &graphics_card,
                                                     const std::string &device_extension_name) {
    assert(graphics_card);
    assert(!device_extension_name.empty());

    if (m_device_extensions_cache.empty()) {
        create_device_extensions_cache(graphics_card);
    }

    auto result = std::find_if(m_device_extensions_cache.begin(), m_device_extensions_cache.end(),
                               [&](const VkExtensionProperties &device_extension) {
                                   return (strcmp(device_extension.extensionName, device_extension_name.c_str()) == 0);
                               });

    // True if device extension was found and is supported!
    return result != m_device_extensions_cache.end();
}

bool AvailabilityChecksManager::has_presentation(const VkPhysicalDevice &graphics_card, const VkSurfaceKHR &surface) {
    assert(graphics_card);
    assert(surface);

    VkBool32 presentation_available = false;

    // Query if presentation is supported.
    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, 0, surface, &presentation_available);

    // I don't know what the value of presentation_available might be in case of an error, so handle it explicitely.
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return false;
    }

    return presentation_available;
}

bool AvailabilityChecksManager::has_swapchain(const VkPhysicalDevice &graphics_card) {
    assert(graphics_card);
    return has_device_extension(graphics_card, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

} // namespace inexor::vulkan_renderer
