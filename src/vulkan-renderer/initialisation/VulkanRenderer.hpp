#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#ifdef _WIN32
  #ifndef VK_USE_PLATFORM_WIN32_KHR
    #define VK_USE_PLATFORM_WIN32_KHR
  #endif
#endif

// TODO: Add support for other operation systems here.

#include "../error-handling/VulkanErrorHandling.hpp"
#include "../graphics-card-info/VulkanGraphicsCardInfoViewer.hpp"
#include "../window-manager/VulkanWindowManager.hpp"
#include "../availability-checks/VulkanAvailabilityChecks.hpp"
#include "../settings-decision-maker/VulkanSettingsDecisionMaker.hpp"
#include "../shader-manager/VulkanShaderManager.hpp"
#include "../synchronisation-manager/VulkanSynchronisationManager.hpp"
#include "../vertex-structure/InexorVertex.hpp"
#include "../vertex-buffer-manager/VulkanMeshBufferManager.hpp"
#include "../ubo-manager/VulkanUniformBufferManager.hpp"
#include "../debug-marker/VulkanDebugMarkerManager.hpp"
#include "../queue-manager/VulkanQueueManager.hpp"
#include "../time-step/InexorTimeStep.hpp"
#include "../texture-manager/VulkanTextureManager.hpp"
#include "../vertex-buffer-manager/InexorMeshBuffer.hpp"

// Vulkan Memory Allocator.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#include "../../vma/vk_mem_alloc.h"

#include <spdlog/spdlog.h>


#include <vector>
#include <string>
#include <vector>
#include <iostream>
using namespace std;

// The maximum number of images to process simultaneously.
// TODO: Refactoring! That is triple buffering essentially!
#define INEXOR_MAX_FRAMES_IN_FLIGHT 3


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanInitialisation
	/// @brief A class for initialisation of the Vulkan API.
	class VulkanRenderer :  public VulkanGraphicsCardInfoViewer,
							public VulkanWindowManager,
							public VulkanAvailabilityChecks,
							public VulkanShaderManager,
							public VulkanSynchronisationManager,
							public VulkanMeshBufferManager,
							public VulkanQueueManager,
							public VulkanTextureManager,
							public InexorTimeStep
							// TODO: VulkanSwapchainManager, VulkanPipelineManager, VulkanRenderPassManager?
	{
		public:

			VulkanRenderer();

			~VulkanRenderer();


		public:
			
			// Although many drivers and platforms trigger VK_ERROR_OUT_OF_DATE_KHR automatically after a window resize,
			// it is not guaranteed to happen. That’s why we’ll add some extra code to also handle resizes explicitly.
			bool frame_buffer_resized = false;

		protected:

			// Vulkan Memory Allocator
			// Vulkan requires you to manage video memory for every type of resource like textures or vertex buffers manually.
			// To avoid having to do the memory management explicitely, we will use the famous Vulkan memory allocator library by AMD.
			VmaAllocator vma_allocator;
			
			// The debug marker manager instance.
			std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

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
			uint32_t number_of_images_in_swapchain = 0;

			// Structure specifying a queue submit operation.
			VkSubmitInfo submit_info;
			
			// Structure describing parameters of a queue presentation.
			VkPresentInfoKHR present_info;

			// 
			std::vector<VkImage> swapchain_images;
			
			// The images in the swapchain.
			std::vector<VkImageView> swapchain_image_views;

			// Opaque handle to a pipeline layout object.
			VkPipelineLayout pipeline_layout;

			// The image format which is used.
			VkFormat selected_image_format;
			
			// 
			VkExtent2D selected_swapchain_image_extent = {};

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
			


			// Neccesary for synchronisation!
			std::vector<VkSemaphore> image_available_semaphores;
			std::vector<VkSemaphore> rendering_finished_semaphores;
			std::vector<VkFence> in_flight_fences;
			std::vector<VkFence> images_in_flight;

			
			VkDescriptorSetLayout descriptor_set_layout;
			
			
			VkDescriptorPool descriptor_pool;

			
			std::vector<VkDescriptorSet> descriptor_sets;


			/// Debug report callback.
			VkDebugReportCallbackEXT debug_report_callback{};

			// Vulkan debug report callback.
			bool debug_report_callback_initialised = false;


			std::vector<InexorBuffer> uniform_buffers;


		protected:


			/// @brief Creates a Vulkan instance.
			/// @param application_name The name of the application
			/// @param engine_name The name of the engine.
			/// @param application_version The version of the application encoded as an unsigned 32 bit integer.
			/// @param engine_version The version of the engine encoded as an unsigned 32 bit integer.
			/// @param enable_validation_layers True if validation is enabled.
			VkResult create_vulkan_instance(const std::string& application_name, const std::string& engine_name, const uint32_t application_version, const uint32_t engine_version, bool enable_validation_instance_layers = true, bool enable_renderdoc_instance_layer = false);


			/// @brief Create a window surface.
			/// @param vulkan_instance The instance of Vulkan.
			/// @param window The GLFW window.
			/// @param vulkan_surface The Vulkan (window) surface.
			VkResult create_window_surface(const VkInstance& vulkan_instance, GLFWwindow* window, VkSurfaceKHR& vulkan_surface);


			/// @brief Create a physical device handle.
			/// @param graphics_card The regarded graphics card.
			VkResult create_physical_device(const VkPhysicalDevice& graphics_card, bool enable_debug_markers = true);

			
			/// @brief Creates an instance of VulkanDebugMarkerManager
			VkResult initialise_debug_marker_manager(const bool enable_debug_markers = true);


			/// @brief Initialise allocator of Vulkan Memory Allocator library.
			VkResult create_vma_allocator();


			/// @brief Creates the command buffers.
			VkResult create_command_buffers();


			/// @brief Records the command buffers.
			VkResult record_command_buffers(const std::vector<InexorMeshBuffer>& buffers);


			/// @brief Creates the semaphores neccesary for synchronisation.
			VkResult create_synchronisation_objects();


			/// @brief Creates the swapchain.
			VkResult create_swapchain();
			
			
			/// @brief Cleans the swapchain.
			VkResult cleanup_swapchain();
			
			
			/// @brief Creates the uniform buffers.
			VkResult create_uniform_buffers();

			
			/// @brief Creates the descriptor set.
			VkResult create_descriptor_sets();


			/// @brief Creates the descriptor pool.
			VkResult create_descriptor_pool();
			

			/// @brief Updates the uniform buffer.
			VkResult update_uniform_buffer(std::size_t current_image);


			/// @brief Recreates the swapchain.
			VkResult recreate_swapchain(std::vector<InexorMeshBuffer>& mesh_buffers);


			/// @brief Creates the command pool.
			VkResult create_command_pool();


			/// @brief Creates the frame buffers.
			VkResult create_frame_buffers();
			

			/// @brief Creates the descriptor set layout.
			VkResult create_descriptor_set_layout();


			/// @brief Creates the rendering pipeline.
			VkResult create_pipeline();


			/// @brief Creates the image views.
			VkResult create_image_views();


			/// @brief Destroys all Vulkan objects.
			VkResult shutdown_vulkan();

	};
	
};
};
