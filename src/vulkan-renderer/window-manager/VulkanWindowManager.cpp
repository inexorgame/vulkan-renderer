#include "VulkanWindowManager.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	VulkanWindowManager::VulkanWindowManager()
	{
	}

	
	VulkanWindowManager::~VulkanWindowManager()
	{
	}


	void VulkanWindowManager::create_window(const uint32_t width, const uint32_t height, const std::string& window_name, const bool window_resizable)
	{
		// Initialise GLFW library.
		glfwInit();

		// We do not want to use the OpenGL API.
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// Don't show window until Vulkan initialisation is finished.
		glfwWindowHint(GLFW_VISIBLE, false);


		// Give the window a fixed size, if resize is not desired.
		if(!window_resizable)
		{
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		}

		// Store the width and height. Classes which inherit from VulkanWindowManager
		// maybe want to know width and height at some point after initialisation.
		window_width = width;
		window_height = height;
		
		// Create the window using GLFW library.
		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
	}


	void VulkanWindowManager::destroy_window()
	{
		assert(window);

		glfwDestroyWindow(window);
		glfwTerminate();
	}


};
};
