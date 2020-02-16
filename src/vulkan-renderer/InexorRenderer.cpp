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
		
		create_shader_from_file(device, VK_SHADER_STAGE_VERTEX_BIT, "vertex_shader.spv");
		create_shader_from_file(device, VK_SHADER_STAGE_FRAGMENT_BIT, "fragment_shader.spv");

		// TODO: Setup shaders JSON or TOML list file.
		return VK_SUCCESS;
	}


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
	

	void InexorRenderer::init()
	{
		// Create a window using GLFW library.
		create_window(INEXOR_WINDOW_WIDTH, INEXOR_WINDOW_HEIGHT, INEXOR_WINDOW_TITLE, true);

		// Store the current InexorRenderer instance in the window user pointer.
		// Since GLFW is a C API, we can't pass pointers to member functions and have to do it this way!
		glfwSetWindowUserPointer(window, this);

		// Setup callback for window resize
		glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);

		// Create a Vulkan instance.
		VkResult result = create_vulkan_instance(INEXOR_APPLICATION_NAME, INEXOR_ENGINE_NAME, INEXOR_APPLICATION_VERSION, INEXOR_ENGINE_VERSION);
		vulkan_error_check(result);
		
		// The window surface needs to be created right after the instance creation,
		// because it can actually influence the physical device selection.

		// Create a window surface using GLFW library.
		result = create_window_surface(instance, window, surface);
		vulkan_error_check(result);

		// List up all available graphics cards.
		print_all_physical_devices(instance, surface);

		// TODO: Implement command line argument for preferred graphics card!
		// Let the user select a graphics card or select the "best" one automatically.
		std::optional<VkPhysicalDevice> graphics_card = decide_which_graphics_card_to_use(instance);
		
		if(graphics_card.has_value())
		{
			selected_graphics_card = graphics_card.value();
		}
		else
		{
			std::string error_message = "Error: Could not find a suitable graphics card!";
			display_error_message(error_message);
			exit(-1);
		}

		// TODO: Move this to device selection mechanism!
		if(!check_swapchain_availability(selected_graphics_card))
		{
			display_error_message("Error: Swapchain device extension is not supported on this system!");
			shutdown_vulkan();
			exit(-1);
		}

		// Create the device queues.
		result = create_device_queues();
		vulkan_error_check(result);

		// Create a physical device handle of the selected graphics card.
		result = create_physical_device(selected_graphics_card);
		vulkan_error_check(result);

		// Print some detailed information about the available Vulkan driver.
		print_driver_vulkan_version();
		print_instance_layers();
		print_instance_extensions();

		// TODO: Loop through all available graphics cards and print information about them.
		// TODO: Implement print_limits_of_every_graphics_card_available() ?

		print_device_layers(selected_graphics_card);
		print_device_extensions(selected_graphics_card);
		print_graphics_card_limits(selected_graphics_card);
		print_graphics_cards_sparse_properties(selected_graphics_card);
		print_graphics_card_features(selected_graphics_card);
		print_graphics_card_memory_properties(selected_graphics_card);

		// In this section, we need to check if the setup that we want to make is supported by the system. 
		
		// TODO: Move this to device selection mechanism!

		// Query if presentation is supported.
		if(!check_presentation_availability(selected_graphics_card, surface))
		{
			display_error_message("Error: Presentation is not supported on this system!");
			shutdown_vulkan();
			exit(-1);
		}

		// TODO: Move this to create_queues method?
		// Check if suitable queues were found.
		if(!present_queue_family_index.has_value() || (!graphics_queue_family_index.has_value()))
		{
			display_error_message("Error: Could not find suitable queues!");
			shutdown_vulkan();
			exit(-1);
		}

		// Setup the queues for presentation and graphics.
		// Since we only have one queue per queue family, we acquire index 0.
		vkGetDeviceQueue(device, present_queue_family_index.value(), 0, &present_queue);
		vkGetDeviceQueue(device, graphics_queue_family_index.value(), 0, &graphics_queue);

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

		result = record_command_buffers();
		vulkan_error_check(result);

		result = create_synchronisation_objects();
		vulkan_error_check(result);
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
