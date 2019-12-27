#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <vulkan/vulkan.h>
#ifdef _WIN32
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

#include "../error-handling/VulkanErrorHandling.hpp"
#include "../graphics-card-info/VulkanGraphicsCardInfoViewer.hpp"

#include <string>
#include <vector>
#include <iostream>
using namespace std;

// Change these definitions if you want to fork the renderer!
// These definitions will be used in the create_vulkan_instance() method.
#define INEXOR_ENGINE_VERSION      VK_MAKE_VERSION(1,0,0)
#define INEXOR_APPLICATION_VERSION VK_MAKE_VERSION(1,0,0)
#define INEXOR_APPLICATION_NAME    "Inexor-Application"
#define INEXOR_ENGINE_NAME         "Inexor-Engine"


namespace inexor {
namespace vulkan_renderer {


	// 
	class VulkanInitialisation : public VulkanErrorHandling,
	                             public VulkanGraphicsCardInfoViewer
	{
		public:

			// 
			VulkanInitialisation();

			// 
			~VulkanInitialisation();


		protected:
		
			// 
			VkInstance vulkan_instance;

			// 
			VkDevice vulkan_device;
			
			// 
			VkPhysicalDevice selected_graphics_card;

			// The number of graphics cards available on the machine.
			uint32_t number_of_physical_devices;
			
			// The graphics cards available on the machine.
			std::vector<VkPhysicalDevice> graphics_cards;
			
			// 
			VkSurfaceKHR vulkan_surface;

			// 
			VkSwapchainKHR vulkan_swapchain;

			// 
			bool vulkan_device_ready;
				
			// 
			std::vector<VkImageView> image_views;
			
			// 
			uint32_t number_of_images_in_swap_chain;

			// TODO: Setup shaders from JSON file?			
			VkShaderModule vertex_shader_module;
			VkShaderModule fragment_shader_module;

			// 
			VkPipelineLayout vulkan_pipeline_layout;

			// TODO: Check if system supports this image format!
			const VkFormat image_format = VK_FORMAT_B8G8R8A8_UNORM;

			// 
			std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

			// 
			VkRenderPass render_pass;

			// 
			VkPipeline vulkan_pipeline;

			// 
			std::vector<VkFramebuffer> frame_buffers;

			// 
			VkCommandPool command_pool;

			// 
			std::vector<VkCommandBuffer> command_buffers;

			// TODO: Generalise setup of semaphores?

			// 
			VkSemaphore semaphore_image_available;

			// 
			VkSemaphore semaphore_rendering_finished;

			// 			
			VkQueue queue;
			
			// 
			GLFWwindow* window;

			// 
			uint32_t window_width;

			// 
			uint32_t window_height;


		protected:

			// 
			VkResult create_vulkan_instance(const std::string& application_name, const std::string& engine_name, const uint32_t application_version, const uint32_t engine_version, bool enable_validation_layers);

			// 
			VkResult create_physical_device(const VkPhysicalDevice& graphics_card);

			// 
			void enumerate_physical_devices();

			// 
			void create_command_buffers();

			// 
			void create_semaphores();

			// 
			void check_support_of_presentation(const VkPhysicalDevice& graphics_card);

			// 
			void setup_swap_chain();

			// 
			void setup_frame_buffers();
			
			// 
			void setup_pipeline();

			// 
			bool init_vulkan();

			// 
			void shutdown_vulkan();
			
			// Initialises a window using the GLFW library.
			void init_window(const int width, const int height, const std::string& window_name);
			
			// 
			void shutdown_window();


	};
	
};
};
