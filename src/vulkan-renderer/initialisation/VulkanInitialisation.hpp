#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// TODO: Add support for other operation systems here.

#include "../error-handling/VulkanErrorHandling.hpp"
#include "../graphics-card-info/VulkanGraphicsCardInfoViewer.hpp"
#include "../window-manager/VulkanWindowManager.hpp"
#include "../availability-checks/VulkanAvailabilityChecks.hpp"
#include "../settings-decision-maker/VulkanSettingsDecisionMaker.hpp"

#include <vector>
#include <string>
#include <vector>
#include <iostream>


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanInitialisation
	/// @brief A class for initialisation of the Vulkan API.
	class VulkanInitialisation : public VulkanGraphicsCardInfoViewer,
								 public VulkanWindowManager,
								 public VulkanAvailabilityChecks,
								 public VulkanSettingsDecisionMaker
								 // TODO: VulkanTextureManager
								 // TODO: VulkanConfigurationFileManager
	{
		public:

			VulkanInitialisation();

			~VulkanInitialisation();


		protected:
		
			// The Vulkan instance handle.
			VkInstance instance;

			// The device handle.
			VkDevice device;

			// Opaque handle to a surface object.
			VkSurfaceKHR surface;
			
			// The graphics card which was selected either automatically or manually by the user.
			VkPhysicalDevice selected_graphics_card;

			// Presentation mode supported for a surface.
			VkPresentModeKHR selected_present_mode;

			// Opaque handle to a swapchain object.
			VkSwapchainKHR swapchain;

			// The number of images in the swapchain.
			uint32_t number_of_images_in_swapchain;

			// Structure specifying a queue submit operation.
			VkSubmitInfo submit_info;
			
			// Structure describing parameters of a queue presentation.
			VkPresentInfoKHR present_info;

			// The images in the swap chain.
			std::vector<VkImageView> image_views;

			// Opaque handle to a pipeline layout object.
			VkPipelineLayout pipeline_layout;

			// The image format which is used.
			VkFormat selected_image_format;

			// Supported color space of the presentation engine. 
			VkColorSpaceKHR selected_color_space;

			// 
			std::vector<VkPipelineShaderStageCreateInfo> shader_stages;

			// 
			VkRenderPass render_pass;

			// 
			VkPipeline pipeline;

			// 
			std::vector<VkFramebuffer> frame_buffers;

			// 
			VkCommandPool command_pool;

			// 
			std::vector<VkCommandBuffer> command_buffers;

			// TODO: Generalise setup of semaphores?

			// Opaque handle to a semaphore object.
			VkSemaphore semaphore_image_available;

			// Opaque handle to a semaphore object.
			VkSemaphore semaphore_rendering_finished;

			// Try to use one queue family for both graphics and presentation.
			bool use_one_queue_family_for_graphics_and_presentation = false;

			// 
			std::optional<uint32_t> graphics_queue_family_index;
			VkQueue graphics_queue;
			
			// 
			std::optional<uint32_t> present_queue_family_index;
			VkQueue present_queue;

			// 
			std::vector<VkDeviceQueueCreateInfo> device_queues;



			// TODO: Setup shaders from JSON or TOML file?			
			VkShaderModule vertex_shader_module;

			VkShaderModule fragment_shader_module;


		protected:


			/// @brief Creates a Vulkan instance.
			/// @param application_name The name of the application
			/// @param engine_name The name of the engine.
			/// @param application_version The version of the application encoded as an unsigned 32 bit integer.
			/// @param engine_version The version of the engine encoded as an unsigned 32 bit integer.
			/// @param enable_validation_layers True if validation is enabled.
			VkResult create_vulkan_instance(const std::string& application_name, const std::string& engine_name, const uint32_t application_version, const uint32_t engine_version, bool enable_validation_layers = true);


			/// @brief Create a window surface.
			/// @param vulkan_instance The instance of Vulkan.
			/// @param window The GLFW window.
			/// @param vulkan_surface The Vulkan (window) surface.
			VkResult create_window_surface(const VkInstance& vulkan_instance, GLFWwindow* window, VkSurfaceKHR& vulkan_surface);


			/// @brief Create a physical device handle.
			/// @param graphics_card The regarded graphics card.
			VkResult create_physical_device(const VkPhysicalDevice& graphics_card);


			/// @brief Creates the command buffers.
			VkResult create_command_buffers();


			/// @brief Records the command buffers.
			VkResult record_command_buffers();


			/// @brief Creates the semaphores neccesary for synchronisation.
			VkResult create_semaphores();


			/// @brief Creates the swapchain.
			VkResult create_swapchain();
			

			/// @brief Creates the command pool.
			VkResult create_command_pool();


			/// @brief Creates the frame buffers.
			VkResult create_frame_buffers();
			

			/// @brief Creates the rendering pipeline.
			VkResult create_pipeline();


			/// @brief Creates the image views.
			VkResult create_image_views();


			/// @brief Creates the device queues.
			VkResult create_device_queues();


			/// @brief Destroys all Vulkan objects.
			void shutdown_vulkan();

	};
	
};
};
