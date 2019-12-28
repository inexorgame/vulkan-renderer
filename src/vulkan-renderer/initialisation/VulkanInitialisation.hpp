#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>
#include <vector>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include "../error-handling/VulkanErrorHandling.hpp"
#include "../graphics-card-info/VulkanGraphicsCardInfoViewer.hpp"
#include "../window-manager/VulkanWindowManager.hpp"

#include <string>
#include <vector>
#include <iostream>
using namespace std;

#include <vulkan/vulkan.h>


namespace inexor {
namespace vulkan_renderer {


	// 
	class VulkanInitialisation : public VulkanErrorHandling,
	                             public VulkanGraphicsCardInfoViewer,
								 public VulkanWindowManager
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
			
			// The number of graphics cards available on the machine.
			uint32_t number_of_graphics_cards;
			
			// The graphics cards available on the machine.
			std::vector<VkPhysicalDevice> graphics_cards;
			
			// The selected graphics card.
			// The graphics card could either be selected by the user
			// or it could be determined automatically by the engine.
			VkPhysicalDevice selected_graphics_card;

			// 
			VkSurfaceKHR vulkan_surface;

			// 
			VkSwapchainKHR vulkan_swapchain;
							
			// 
			uint32_t number_of_images_in_swap_chain;
			
			// 
			std::vector<VkImageView> image_views;

			// TODO: Setup shaders from JSON file?			
			VkShaderModule vertex_shader_module;
			VkShaderModule fragment_shader_module;

			// 
			VkPipelineLayout vulkan_pipeline_layout;

			// The image format which is used.
			VkFormat selected_image_format;

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


		protected:

			// Option A: The graphics card can be chosen by the user manually.
			// Option B: The "best" graphics card will be determined by the engine automatically.
			VkPhysicalDevice decide_which_graphics_card_to_use();

			// Select an image format which will be used.
			// The selected image format must be supported by the system!
			// https://vulkan.lunarg.com/doc/view/latest/windows/vkspec.html#formats-compatibility
			VkFormat decide_which_image_format_to_use();

			// Vulkan validation layers should be enabled during development!
			VkResult create_vulkan_instance(const std::string& application_name, const std::string& engine_name, const uint32_t application_version, const uint32_t engine_version, bool enable_validation_layers = true);

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
			void create_swap_chain();
			
			// 
			void create_command_pool();

			// 
			void create_frame_buffers();
			
			// 
			void create_pipeline();

			// 
			void create_image_views();

			// 
			void shutdown_vulkan();

	};
	
};
};
