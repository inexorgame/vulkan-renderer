#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#include <string>


namespace inexor {
namespace vulkan_renderer {

	// 
	class VulkanWindowManager
	{
		public:

			// 
			VulkanWindowManager();
			
			// 
			~VulkanWindowManager();


		protected:
			
			// 
			GLFWwindow* window;

			// 
			uint32_t window_width;

			// 
			uint32_t window_height;

			// 
			void init_window(const int width, const int height, const std::string& window_name);
			
			// 
			void shutdown_window();

	};

};
};
