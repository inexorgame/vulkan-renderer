#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#include <assert.h>

#include <string>


namespace inexor {
namespace vulkan_renderer {

	
	// TODO: Support multiple windows in the future.


	/// @class VulkanWindowManager
	/// @brief A class for managing windows.
	class VulkanWindowManager
	{
		public:

			VulkanWindowManager();
			
			~VulkanWindowManager();


		protected:
			
			/// The GLFW window.
			GLFWwindow* window = nullptr;
			
			/// The width of the window.
			uint32_t window_width = 0;

			/// The height of the window.
			uint32_t window_height = 0;

			// The title of the window.
			std::string window_title = "";

			/// @brief Creates a window of with a specific width, height and name.
			/// @param width The width of the window.
			/// @param height The height of the window.
			/// @param window_name The name of the window.
			void create_window(const uint32_t width, const uint32_t height, const std::string& window_name, const bool window_resizable = false);
			
			/// @brief Destroys the window.
			void destroy_window();

	};

};
};
