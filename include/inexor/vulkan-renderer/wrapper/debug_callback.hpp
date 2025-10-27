#pragma once

#include <volk.h>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Instance;

/// RAII wrapper for Vulkan debug utils callbacks.
class VulkanDebugUtilsCallback {
private:
    // We need to store the instance as a member to destroy the debug utils messenger callback in the destructor again.
    const Instance &m_instance;
    VkDebugUtilsMessengerEXT m_debug_callback{VK_NULL_HANDLE};

public:
    /// Default constructor.
    /// @param inst The Vulkan instance.
    /// @param debug_callback A pointer to the debug utils messenger callback.
    VulkanDebugUtilsCallback(const Instance &inst, PFN_vkDebugUtilsMessengerCallbackEXT debug_callback);
    ~VulkanDebugUtilsCallback();
};

} // namespace inexor::vulkan_renderer::wrapper
