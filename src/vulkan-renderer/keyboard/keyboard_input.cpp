#include "keyboard_input.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	void InexorKeyboardInputHandler::initialise(GLFWwindow* window, GLFWkeyfun keyboard_input_callback)
	{
		assert(window);

		spdlog::debug("Initialising keyboard input handler.");

		glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);

		glfwSetKeyCallback(window, keyboard_input_callback);
	}


};
};
