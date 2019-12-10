#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <spdlog/spdlog.h>


namespace inexor {
namespace vulkan_renderer {

	// 
	class InexorRenderer
	{
		private:

			// 
			GLFWwindow* window;

			// 
			void init_vulkan();

			// 
			void init_window(const int width, const int height, const std::string& window_name);
			
			// 
			void close_window();


		public:
			
			// 
			InexorRenderer();
			
			// 
			~InexorRenderer();

			// 
			void init();

			// 
			void run();

			// 
			void cleanup();

	};

};
};
