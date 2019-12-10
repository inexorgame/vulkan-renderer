#include "InexorRenderer.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	void InexorRenderer::init_window(const int width, const int height, const std::string& window_name)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
	}


	void InexorRenderer::close_window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}


	void InexorRenderer::init_vulkan()
	{
		// First we need to create a Vulkan Instance.
		VkApplicationInfo appInfo = {};

		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Inexor";
		appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
		appInfo.pEngineName = "Inexor";
		appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);

		// TODO: Should we switch to VK_API_VERSION_1_1 ?
		appInfo.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;

		// Let's create an instance.
		VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

		if(VK_SUCCESS != result)
		{
			spdlog::error("vkCreateInstance failed!");
		}
	}


	void InexorRenderer::shutdown_vulkan()
	{
		vkDestroyInstance(instance, nullptr);
	}


	InexorRenderer::InexorRenderer()
	{
		window = nullptr;
	}


	InexorRenderer::~InexorRenderer()
	{
	}


	void InexorRenderer::init()
	{
		init_window(800, 600, "Inexor Vulkan Renderer");
		init_vulkan();
	}


	void InexorRenderer::run()
	{
		while(!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}


	void InexorRenderer::cleanup()
	{
		close_window();
	}


};
};
