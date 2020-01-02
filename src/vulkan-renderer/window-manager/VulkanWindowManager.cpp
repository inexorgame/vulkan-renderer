#include "VulkanWindowManager.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	VulkanWindowManager::VulkanWindowManager()
	{
		window = nullptr;
		window_width = 0;
		window_height = 0;
	}

	
	VulkanWindowManager::~VulkanWindowManager()
	{
	}


	void VulkanWindowManager::init_window(const int width, const int height, const std::string& window_name)
	{
		// Initialise GLFW library.
		glfwInit();

		// We do not want to use the OpenGL API.
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		// The window shall not be resizable for now.
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// Store the width and height. Classes which inherit from VulkanWindowManager
		// maybe want to know width and height at some point after initialisation.
		window_width = width;
		window_height = height;
		
		// Create the window using GLFW library.
		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
	}


	void VulkanWindowManager::shutdown_window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}


};
};
