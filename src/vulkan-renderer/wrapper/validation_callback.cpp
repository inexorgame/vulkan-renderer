#include "inexor/vulkan-renderer/wrapper/validation_callback.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

namespace inexor::vulkan_renderer::wrapper {

ValidationCallback::ValidationCallback(const VkInstance instance,
                                       const PFN_vkDebugUtilsMessengerCallbackEXT debug_callback)
    : m_instance(instance) {
    if (instance == nullptr) {
        throw std::invalid_argument("Error: Invalid Vulkan instance!");
    }
    if (debug_callback == nullptr) {
        throw std::invalid_argument("Error: Invalid debug utils messenger callback!");
    }

    const auto dbg_messenger_ci = make_info<VkDebugUtilsMessengerCreateInfoEXT>({
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback,
        .pUserData = nullptr,
    });

    if (const auto result = vkCreateDebugUtilsMessengerEXT(m_instance, &dbg_messenger_ci, nullptr, &m_callback); result != VK_SUCCESS) {
        throw VulkanException(
            "Error: Could not create Vulkan validation layer debug callback! (vkCreateDebugUtilsMessengerEXT failed!)",
            result);
    }
}

ValidationCallback::~ValidationCallback() {
    vkDestroyDebugUtilsMessengerEXT(m_instance, m_callback, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
