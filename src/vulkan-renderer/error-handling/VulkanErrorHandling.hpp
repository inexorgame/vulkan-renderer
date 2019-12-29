#pragma once

#include <string>
#include <vulkan/vulkan.h>

#ifdef _WIN32
#include <Windows.h>
#endif


namespace inexor {
namespace vulkan_renderer {


	// Returns an error text.
	std::string get_error_string(const VkResult& result_code);

	// Displays an error message.
	void display_error_message(const std::string& error_message, const std::string& message_box_title = "Error");

	// Generalise error handling.
	void vulkan_error_check(const VkResult& result);

};
};
