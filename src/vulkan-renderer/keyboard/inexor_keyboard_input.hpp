#pragma once

#include <assert.h>

#include <spdlog/spdlog.h>

#include <glfw/glfw3.h>


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorKeyboardInputHandler.
	class InexorKeyboardInputHandler
	{
		public:

			InexorKeyboardInputHandler();
			
			~InexorKeyboardInputHandler();


		protected:

			/// @brief Initialises keyboard input handler.
			/// @param window [in] The glfw window.
			/// @param keyboard_input_callback [in] The glfw keyboard input callback.
			void initialise(GLFWwindow* window, GLFWkeyfun keyboard_input_callback);

	};


};
};
