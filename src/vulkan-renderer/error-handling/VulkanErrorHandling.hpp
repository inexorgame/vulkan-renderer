#pragma once

#include <string>
#include <vulkan/vulkan.h>

#ifdef _WIN32
#include <Windows.h>
#endif


namespace inexor {
namespace vulkan_renderer {


	// 
	class VulkanErrorHandling
	{
		public:

			VulkanErrorHandling();

			~VulkanErrorHandling();

		protected:

			// Returns an error text.
			std::string get_error_string(const VkResult& result_code) const;

			// Displays an error message.
			void display_error_message(const std::string& error_message, const std::string& message_box_title = "Error") const;

	};

};
};
