#pragma once

#include <vulkan/vulkan_core.h>

#include <string>

namespace inexor::vulkan_renderer {

// These functions help to abstract error handling in Vulkan.

// @brief Returns an error text.
// @param result_code The result code.
// @return A string which describes the error.
[[nodiscard]] std::string error_description_text(const VkResult &result_code);

// @brief Displays an error message as a message.
// @param error_message The error message.
// @param message_box_title The title of the message box.
void display_error_message(const std::string &error_message, const std::string &message_box_title = "Error");

void display_fatal_error_message(const std::string &error_message,
                                 const std::string &message_box_title = "Fatal Error");

void display_warning_message(const std::string &warning_message, const std::string &message_box_title = "Warning");

// @brief Generalises error handling.
// @param result The result which is to be validated.
void vulkan_error_check(const VkResult &result);

} // namespace inexor::vulkan_renderer
