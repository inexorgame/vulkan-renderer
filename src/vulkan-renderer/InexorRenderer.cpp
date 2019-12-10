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
		// TODO: Implement!
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
