#include "inexor/vulkan-renderer/availability_checks.hpp"

namespace inexor::vulkan_renderer {

VkResult AvailabilityChecksManager::create_instance_extensions_cache() {
    // First ask Vulkan how many instance extensions are available on the system.
    VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &available_instance_extensions, NULL);
    vulkan_error_check(result);

    if (available_instance_extensions == 0) {
        // It should be a very rare case that no instance extensions are available at all. Still we have to consider this!
        display_error_message("Error: No Vulkan instance extensions available!");

        // Since there are no instance extensions available at all, the desired one is not supported either.
        return VK_ERROR_INITIALIZATION_FAILED;
    } else {
        // Preallocate memory for extension properties.
        instance_extensions_cache.resize(available_instance_extensions);

        // Get the information about the available instance extensions.
        result = vkEnumerateInstanceExtensionProperties(NULL, &available_instance_extensions, instance_extensions_cache.data());
        vulkan_error_check(result);
    }

    return VK_SUCCESS;
}

bool AvailabilityChecksManager::has_instance_extension(const std::string &instance_extension_name) {
    assert(!instance_extension_name.empty());

    if (instance_extensions_cache.empty()) {
        create_instance_extensions_cache();
    }

    // Loop through all available instance extensions and search for the requested one.
    for (const VkExtensionProperties &instance_extension : instance_extensions_cache) {
        // Compare the name of the current instance extension with the requested one.
        if (strcmp(instance_extension.extensionName, instance_extension_name.c_str()) == 0) {
            // Yes, this instance extension is supported!
            return true;
        }
    }

    // No, this instance extension could not be found and thus is not supported!
    return false;
}

VkResult AvailabilityChecksManager::create_instance_layers_cache() {
    // First ask Vulkan how many instance layers are available on the system.
    VkResult result = vkEnumerateInstanceLayerProperties(&available_instance_layers, nullptr);
    vulkan_error_check(result);

    if (available_instance_layers == 0) {
        // It should be a very rare case that no instance layers are available at all. Still we have to consider this!
        display_error_message("Error: No Vulkan instance layers available!");

        // Since there are no instance layers available at all, the desired one is not supported either.
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    // Preallocate memory for layer properties.
    instance_layers_cache.resize(available_instance_layers);

    // Get the information about the available instance layers.
    result = vkEnumerateInstanceLayerProperties(&available_instance_layers, instance_layers_cache.data());
    vulkan_error_check(result);

    return VK_SUCCESS;
}

bool AvailabilityChecksManager::has_instance_layer(const std::string &instance_layer_name) {
    assert(!instance_layer_name.empty());

    if (instance_layers_cache.empty()) {
        create_instance_layers_cache();
    }

    // Loop through all available instance layers and search for the requested one.
    for (const VkLayerProperties &instance_layer : instance_layers_cache) {
        // Compare the name of the current instance extension with the requested one.
        if (strcmp(instance_layer.layerName, instance_layer_name.c_str()) == 0) {
            // Yes, this instance extension is supported!
            return true;
        }
    }

    // No, this instance layer could not be found and thus is not supported!
    return false;
}

VkResult AvailabilityChecksManager::create_device_layers_cache(const VkPhysicalDevice &graphics_card) {
    // First ask Vulkan how many device layers are available on the system.
    VkResult result = vkEnumerateDeviceLayerProperties(graphics_card, &available_device_layers, nullptr);
    vulkan_error_check(result);

    if (available_device_layers == 0) {
        // It should be a very rare case that no device layers are available at all. Still we have to consider this!
        display_error_message("Error: No Vulkan device layers available!");

        // Since there are no device layers available at all, the desired one is not supported either.
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    // Preallocate memory for device layers.
    device_layer_properties_cache.resize(available_device_layers);

    // Get the information about the available device layers.
    result = vkEnumerateDeviceLayerProperties(graphics_card, &available_device_layers, device_layer_properties_cache.data());
    vulkan_error_check(result);

    return VK_SUCCESS;
}

bool AvailabilityChecksManager::has_device_layer(const VkPhysicalDevice &graphics_card, const std::string &device_layer_name) {
    assert(graphics_card);
    assert(!device_layer_name.empty());

    if (device_layer_properties_cache.empty()) {
        create_device_layers_cache(graphics_card);
    }

    // Loop through all available device layers and search for the requested one.
    for (const VkLayerProperties &device_layer : device_layer_properties_cache) {
        if (0 == strcmp(device_layer.layerName, device_layer_name.c_str())) {
            // Yes, this device layer is supported!
            return true;
        }
    }

    // No, this device layer could not be found and thus is not supported!
    return false;
}

VkResult AvailabilityChecksManager::create_device_extensions_cache(const VkPhysicalDevice &graphics_card) {
    // First ask Vulkan how many device extensions are available on the system.
    VkResult result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &available_device_extensions, nullptr);
    vulkan_error_check(result);

    if (0 == available_device_extensions) {
        // It should be a very rare case that no device extension are available at all. Still we have to consider this!
        display_error_message("Error: No Vulkan device extensions available!");

        // Since there are no device extensions available at all, the desired one is not supported either.
        return VK_ERROR_INITIALIZATION_FAILED;
    } else {
        // Preallocate memory for device extensions.
        device_extensions_cache.resize(available_device_extensions);

        // Get the information about the available device extensions.
        result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &available_device_extensions, device_extensions_cache.data());
        vulkan_error_check(result);
    }

    return VK_SUCCESS;
}

bool AvailabilityChecksManager::has_device_extension(const VkPhysicalDevice &graphics_card, const std::string &device_extension_name) {
    assert(graphics_card);
    assert(!device_extension_name.empty());

    if (device_extensions_cache.empty()) {
        create_device_extensions_cache(graphics_card);
    }

    // Loop through all available device extensions and search for the requested one.
    for (const VkExtensionProperties &device_extension : device_extensions_cache) {
        if (0 == strcmp(device_extension.extensionName, device_extension_name.c_str())) {
            // Yes, this device extension is supported!
            return true;
        }
    }

    // No, this device extension could not be found and thus is not supported!
    return false;
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
