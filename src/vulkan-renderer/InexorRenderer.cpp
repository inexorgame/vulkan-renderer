#include "InexorRenderer.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	/// @brief Static callback for window resize events.
	/// @note Because GLFW is a C-style API, we can't pass a poiner to a class method, so we have to do it this way!
	/// @param window The GLFW window.
	/// @param height The width of the window.
	/// @param height The height of the window.
	static void frame_buffer_resize_callback(GLFWwindow* window, int width, int height)
	{
		// This is actually the way it is handled by the official Vulkan samples.
		auto app = reinterpret_cast<VulkanInitialisation*>(glfwGetWindowUserPointer(window));
		app->frame_buffer_resized = true;
	}


	InexorRenderer::InexorRenderer()
	{
	}


	InexorRenderer::~InexorRenderer()
	{
	}


	VkResult InexorRenderer::load_shaders()
	{
		// It is important to make sure that you debugging folder contains the required shader files!
		struct InexorShaderSetup
		{
			VkShaderStageFlagBits shader_type;
			std::string shader_file_name;
		};
		
		// The actual file list of shaders that we want to load.
		// TODO: Setup shaders JSON or TOML list file.
		const std::vector<InexorShaderSetup> shader_list = 
		{
			{VK_SHADER_STAGE_VERTEX_BIT, "vertexshader.spv"},
			{VK_SHADER_STAGE_FRAGMENT_BIT, "fragmentshader.spv"}
		};

		for(const auto& shader : shader_list)
		{
			VkResult result = create_shader_from_file(device, shader.shader_type, shader.shader_file_name);
			if(VK_SUCCESS != result)
			{
				vulkan_error_check(result);
				std::string error_message = "Error: Could not initialise shader " +  shader.shader_file_name;
				display_error_message(error_message);
			}
		}

		return VK_SUCCESS;
	}


	// TODO: Refactor rendering!
	VkResult InexorRenderer::draw_frame()
	{
		vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

		uint32_t image_index = 0;
		VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
		
		if(VK_NULL_HANDLE != images_in_flight[image_index])
		{
			vkWaitForFences(device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
		}
		
		// Mark the image as now being in use by this frame
		images_in_flight[image_index] = in_flight_fences[current_frame];

		// Is it time to regenerate the swapchain because window has been resized or minimized?
		if(VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			// VK_ERROR_OUT_OF_DATE_KHR: The swap chain has become incompatible with the surface
			// and can no longer be used for rendering. Usually happens after a window resize.
			return recreate_swapchain();
		}

		// Did something else fail?
		// VK_SUBOPTIMAL_KHR: The swap chain can still be used to successfully present
		// to the surface, but the surface properties are no longer matched exactly.
		if(VK_SUCCESS != result && VK_SUBOPTIMAL_KHR != result)
		{
			std::string error_message = "Error: Failed to acquire swapchain image!";
			display_error_message(error_message);
			exit(-1);
		}

		const VkPipelineStageFlags wait_stage_mask[] =
		{
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext                = nullptr;
		submit_info.waitSemaphoreCount   = 1;
		submit_info.pWaitDstStageMask    = wait_stage_mask;
		submit_info.commandBufferCount   = 1;
		submit_info.pCommandBuffers      = &command_buffers[image_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pWaitSemaphores      = &image_available_semaphores[current_frame];
		submit_info.pSignalSemaphores    = &rendering_finished_semaphores[current_frame];

		vkResetFences(device, 1, &in_flight_fences[current_frame]);
		
		result = vkQueueSubmit(graphics_queue, 1, &submit_info, in_flight_fences[current_frame]);
		if(VK_SUCCESS != result) return result;

		present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext              = nullptr;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = &rendering_finished_semaphores[current_frame];
		present_info.swapchainCount     = 1;
		present_info.pSwapchains        = &swapchain;
		present_info.pImageIndices      = &image_index;
		present_info.pResults           = nullptr;

		result = vkQueuePresentKHR(present_queue, &present_info);

		// Some notes on frame_buffer_resized:
		// It is important to do this after vkQueuePresentKHR to ensure that the semaphores are
		// in a consistent state, otherwise a signalled semaphore may never be properly waited upon.
		if(VK_ERROR_OUT_OF_DATE_KHR == result || VK_SUBOPTIMAL_KHR == result || frame_buffer_resized)
		{
			frame_buffer_resized = false;
			recreate_swapchain();
		}
		
        current_frame = (current_frame + 1) % INEXOR_MAX_FRAMES_IN_FLIGHT;

		return VK_SUCCESS;
	}
	

	VkResult InexorRenderer::init()
	{
		// Create a resizable window using GLFW library.
		create_window(INEXOR_WINDOW_WIDTH, INEXOR_WINDOW_HEIGHT, INEXOR_WINDOW_TITLE, true);

		// Store the current InexorRenderer instance in the GLFW window user pointer.
		// Since GLFW is a C-style API, we can't use a class method as callback for window resize!
		glfwSetWindowUserPointer(window, this);

		// Setup callback for window resize.
		// Since GLFW is a C-style API, we can't use a class method as callback for window resize!
		glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);

		// Create a Vulkan instance.
		VkResult result = create_vulkan_instance(INEXOR_APPLICATION_NAME, INEXOR_ENGINE_NAME, INEXOR_APPLICATION_VERSION, INEXOR_ENGINE_VERSION);
		vulkan_error_check(result);
			
		// Create a window surface using GLFW library.
		// @note The window surface needs to be created right after the instance creation,
		// because it can actually influence the physical device selection.
		result = create_window_surface(instance, window, surface);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// TODO: Implement command line argument for preferred graphics card!

		// Let's see if there is a graphics card that is suitable for us.
		std::optional<VkPhysicalDevice> graphics_card_candidate = decide_which_graphics_card_to_use(instance, surface);

		// Check if we found a graphics card candidate.
		if(graphics_card_candidate.has_value())
		{
			selected_graphics_card = graphics_card_candidate.value();
		}
		else
		{
			// No graphics card suitable!
			std::string error_message = "Error: Could not find any suitable GPU!";
			display_error_message(error_message);

			return VK_ERROR_INITIALIZATION_FAILED;
		}

		result = create_device_queues();
		vulkan_error_check(result);
		
		result = create_physical_device(selected_graphics_card);
		vulkan_error_check(result);

		// Initialise allocator of Vulkan Memory Allocator library.
		result = create_vma_allocator();
		vulkan_error_check(result);

		result = initialise_queues();
		vulkan_error_check(result);

		result = create_swapchain();
		vulkan_error_check(result);
		
		result = create_image_views();
		vulkan_error_check(result);
		
		result = load_shaders();
		vulkan_error_check(result);

		result = create_pipeline();
		vulkan_error_check(result);
		
		result = create_frame_buffers();
		vulkan_error_check(result);
		
		result = create_command_pool();
		vulkan_error_check(result);

		result = create_command_buffers();
		vulkan_error_check(result);

		result = create_vertex_buffers();
		vulkan_error_check(result);

		result = record_command_buffers();
		vulkan_error_check(result);

		result = create_synchronisation_objects();
		vulkan_error_check(result);

		return VK_SUCCESS;
	}


	void InexorRenderer::run()
	{
		// TODO: Run this in a separated thread?
		while(!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			draw_frame();
		}
	}


	void InexorRenderer::cleanup()
	{
		shutdown_vulkan();
		destroy_window();
	}


};
};
