#pragma once

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

namespace inexor::vulkan_renderer {

/// @brief Vulkan validation layer callback.
VKAPI_ATTR VkBool32 VKAPI_CALL vulkan_debug_message_callback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT object_type, std::uint64_t object,
                                                                 std::size_t location, std::int32_t message_code, const char *layer_prefix, const char *message,
                                                                 void *user_data);
} // namespace inexor::vulkan_renderer
