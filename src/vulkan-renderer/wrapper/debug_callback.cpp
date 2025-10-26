#include "inexor/vulkan-renderer/wrapper/debug_callback.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/instance.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

VulkanDebugUtilsCallback::VulkanDebugUtilsCallback(const Instance &inst,
                                                   const PFN_vkDebugUtilsMessengerCallbackEXT debug_callback)
    : m_instance(inst) {
    // Make sure that those two function pointers are available.
    // If they are not available, you either forgot to specify VK_EXT_debug_utils instance extension
    // or your system does not support VK_EXT_debug_utils.
    if (vkCreateDebugUtilsMessengerEXT == nullptr) {
        throw tools::InexorException("Error: vkCreateDebugUtilsMessengerEXT was not found! Make sure to enable "
                                     "VK_EXT_debug_utils instance extension (if available on the system)!");
    }
    if (vkDestroyDebugUtilsMessengerEXT == nullptr) {
        throw tools::InexorException("Error: vkDestroyDebugUtilsMessengerEXT was not found! Make sure to enable "
                                     "VK_EXT_debug_utils instance extension (if available on the system)!");
    }
    if (debug_callback == nullptr) {
        throw std::runtime_error("Error: Parameter 'debug_callback' is invalid!");
    }

    const auto dbg_messenger_ci = make_info<VkDebugUtilsMessengerCreateInfoEXT>({
        .messageSeverity =
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback,
        .pUserData = nullptr,
    });

    spdlog::trace("Calling vkCreateDebugUtilsMessengerEXT to create messenger callback (VK_EXT_debug_utils)");
    if (const auto result =
            vkCreateDebugUtilsMessengerEXT(m_instance.instance(), &dbg_messenger_ci, nullptr, &m_debug_callback);
        result != VK_SUCCESS) {
        throw tools::VulkanException("Error: vkCreateDebugUtilsMessengerEXT failed!", result);
    }
}

VulkanDebugUtilsCallback::~VulkanDebugUtilsCallback() {
    vkDestroyDebugUtilsMessengerEXT(m_instance.instance(), m_debug_callback, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
