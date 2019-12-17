#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>

#include <vulkan/vulkan.h>

#ifdef _WIN32
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

// Change these definitions if you want to fork the renderer!
// These definitions will be used in the create_vulkan_instance() method.
#define INEXOR_ENGINE_VERSION VK_MAKE_VERSION(1,0,0)
#define INEXOR_APPLICATION_VERSION VK_MAKE_VERSION(1,0,0)
#define INEXOR_APPLICATION_NAME "Inexor-Application"
#define INEXOR_ENGINE_NAME "Inexor-Engine"


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

			// 
			VkResult create_vulkan_instance(const std::string& application_name,
			                                const std::string& engine_name,
											const uint32_t application_version,
											const uint32_t engine_version,
											bool enable_validation_layers = true);

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
