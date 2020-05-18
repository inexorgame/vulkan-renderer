#include "inexor/vulkan-renderer/debug_callback.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer {
VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_message_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, std::uint64_t object,
                                                             std::size_t location, std::int32_t message_code, const char *layer_prefix, const char *message,
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
