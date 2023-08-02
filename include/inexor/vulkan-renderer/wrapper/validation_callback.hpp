#pragma once

#include <volk.h>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// RAII wrapper for Vulkan validation layer callbacks using VK_EXT_debug_utils
class ValidationCallback {
private:
    VkDebugUtilsMessengerEXT m_callback{VK_NULL_HANDLE};
    VkInstance m_instance{VK_NULL_HANDLE};

public:
    /// Default constructor
    /// @param instance The Vulkan instance
    /// @param debug_callback The Vulkan validation layer debug callback
    ValidationCallback(VkInstance instance, PFN_vkDebugUtilsMessengerCallbackEXT debug_callback);
    ~ValidationCallback();

    ValidationCallback(const ValidationCallback &) = delete;
    ValidationCallback(ValidationCallback &&) = delete;

    ValidationCallback &operator=(const ValidationCallback &) = delete;
    ValidationCallback &operator=(ValidationCallback &&) = delete;
};

} // namespace inexor::vulkan_renderer::wrapper
