#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer {

// These functions help to abstract error handling in Vulkan.

// @brief Returns an error text.
// @param result_code The result code.
// @return A string which describes the error.
[[nodiscard]] std::string error_description_text(const VkResult &result_code);

/// @brief
/// @param error_message
/// @param message_box_title
void display_error_message(const std::string &error_message, const std::string &message_box_title = "Error");

/// @brief
/// @param error_message
/// @param message_box_title
void display_fatal_error_message(const std::string &error_message,
                                 const std::string &message_box_title = "Fatal Error");

/// @brief
/// @param warning_message
/// @param message_box_title
void display_warning_message(const std::string &warning_message, const std::string &message_box_title = "Warning");

/// @brief
/// @param result
void vulkan_error_check(const VkResult &result);

} // namespace inexor::vulkan_renderer
