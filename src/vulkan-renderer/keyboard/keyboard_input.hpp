#pragma once

#include <cassert>

#include <spdlog/spdlog.h>

#include <glfw/glfw3.h>


namespace inexor {
namespace vulkan_renderer {


	/// This needs some serious redesign!
	/// Should we use polling instead of callbacks?
	/// This would resolve the issue of using static void callback functions instead of class methods!
	/// Also, we might could run this in a separate thread.


	/// @class InexorKeyboardInputHandler.
	class InexorKeyboardInputHandler
	{
		public:

			InexorKeyboardInputHandler() = default;
			
			~InexorKeyboardInputHandler() = default;


		protected:

			/// @brief Initialises keyboard input handler.
			/// @param window [in] The glfw window.
			/// @param keyboard_input_callback [in] The glfw keyboard input callback.
			void initialise(GLFWwindow* window, GLFWkeyfun keyboard_input_callback);

	};


};
};
