#pragma once

#include <string>
#include <vulkan/vulkan.h>

#ifdef _WIN32
#include <Windows.h>
#endif


namespace inexor {
namespace vulkan_renderer {

	// These functions help to abstract error handling in Vulkan.

	// Returns an error text.
	// @param result_code The result code.
	// @return A string which describes the error.
	std::string get_error_description_text(const VkResult& result_code);

	// Displays an error message as a message.
	// @param error_message The error message.
	// @param message_box_title The title of the message box.
	void display_error_message(const std::string& error_message, const std::string& message_box_title = "Error");

	// Generalise error handling.
	// @param result The result which is to be validated.
	void vulkan_error_check(const VkResult& result);

};
};
