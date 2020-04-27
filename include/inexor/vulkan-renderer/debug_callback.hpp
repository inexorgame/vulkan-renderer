#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

namespace inexor::vulkan_renderer {

/// @brief Vulkan validation layer callback.
static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugMessageCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, uint64_t object,
                                                                 size_t location, int32_t message_code, const char *layer_prefix, const char *message,
                                                                 void *user_data) {
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
        spdlog::info(message);
    } else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
        spdlog::debug(message);
    } else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        spdlog::error(message);
    } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
        spdlog::warn(message);
    } else {
        spdlog::warn(message);
    }

    return VK_FALSE;
}

} // namespace inexor::vulkan_renderer
