#include "inexor_keyboard_input.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	InexorKeyboardInputHandler::InexorKeyboardInputHandler()
	{
	}
	
	
	InexorKeyboardInputHandler::~InexorKeyboardInputHandler()
	{
	}


	void InexorKeyboardInputHandler::initialise(GLFWwindow* window, GLFWkeyfun keyboard_input_callback)
	{
		assert(window);

		spdlog::debug("Initialising keyboard input handler.");

		glfwSetKeyCallback(window, keyboard_input_callback);
	}


};
};
