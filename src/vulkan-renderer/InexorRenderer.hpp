#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>


namespace inexor {
namespace vulkan_renderer {

	// 
	class InexorRenderer
	{
		private:

			// 
			GLFWwindow* window;

			// 
			VkInstance vulkan_instance;

			// 
			VkDevice device;

			// The number of graphics cards on the machine.
			uint32_t number_of_physical_devices;

			// The graphics cards on this machine.
			std::vector<VkPhysicalDevice> graphics_cards;

			// 
			bool init_vulkan();

			// 
			void shutdown_vulkan();

			// 
			void init_window(const int width, const int height, const std::string& window_name);
			
			// 
			void close_window();

			// Gets the information on the graphics card and prints it to the console.
			void print_graphics_card_info(const VkPhysicalDevice& graphics_card);


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
