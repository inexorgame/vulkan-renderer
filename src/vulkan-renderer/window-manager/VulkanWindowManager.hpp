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

	// A class for managing windows.
	class VulkanWindowManager
	{
		public:

			VulkanWindowManager();
			
			~VulkanWindowManager();


		protected:
			
			// The GLFW window.
			GLFWwindow* window;
			
			// The width of the window.
			uint32_t window_width;

			// The height of the window.
			uint32_t window_height;

			// Create a window of with a specific width, height and name.
			// @param width The width of the window.
			// @param height The height of the window.
			// @param window_name The name of the window.
			void init_window(const int width, const int height, const std::string& window_name);
			
			// Destroys the window.
			void shutdown_window();

	};

};
};
