#include "InexorRenderer.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	InexorRenderer::InexorRenderer()
	{
	}


	InexorRenderer::~InexorRenderer()
	{
	}


	VkResult InexorRenderer::load_shaders()
	{
		// TODO: Setup shaders from JSON file.
		
		// Important: Make sure your debug directory contains these shader files!
		VkResult result = create_shader_module_from_file(device, "vertex_shader.spv", &vertex_shader_module);
		if(VK_SUCCESS != result) return result;
		
		result = create_shader_module_from_file(device, "fragment_shader.spv", &fragment_shader_module);
		if(VK_SUCCESS != result) return result;

		// TODO: Add more shaders here..

		return VK_SUCCESS;
	}


	VkResult InexorRenderer::prepare_drawing()
	{
		submit_info = {};
		
		// Bitmask of VkPipelineStageFlagBits.
		VkPipelineStageFlags wait_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		// Every member which is commented out will be filled out during draw_frame().
		submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext                = nullptr;
		submit_info.waitSemaphoreCount   = 1;
		submit_info.pWaitDstStageMask    = &wait_stage_mask;
		submit_info.commandBufferCount   = 1;
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores    = &semaphore_rendering_finished;

		present_info = {};
		
		// Every member which is commented out will be filled out during draw_frame().
		present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext              = nullptr;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = &semaphore_rendering_finished;
		present_info.swapchainCount     = 1;
		present_info.pSwapchains        = &swapchain;
		present_info.pResults           = nullptr;

		return VK_SUCCESS;
	}


	VkResult InexorRenderer::draw_frame()
	{
		uint32_t image_index = 0;

		VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, semaphore_image_available, VK_NULL_HANDLE, &image_index);
		if(VK_SUCCESS != result) return result;

		// Only neccesary parts of the structure need to be updated.
		submit_info.pCommandBuffers = &command_buffers[image_index];
		submit_info.pWaitSemaphores = &semaphore_image_available;

		result = vkQueueSubmit(device_queue, 1, &submit_info, VK_NULL_HANDLE);
		if(VK_SUCCESS != result) return result;

		// Only neccesary parts of the structure need to be updated.
		present_info.pImageIndices = &image_index;

		result = vkQueuePresentKHR(device_queue, &present_info);
		if(VK_SUCCESS != result) return result;

		return VK_SUCCESS;
	}
	
	
	void InexorRenderer::init()
	{
		// Create a window using GLFW library.
		create_window(INEXOR_WINDOW_WIDTH, INEXOR_WINDOW_HEIGHT, INEXOR_APP_WINDOW_TITLE);

		// Create a vulkan instance.
		VkResult result = create_vulkan_instance(INEXOR_APPLICATION_NAME, INEXOR_ENGINE_NAME, INEXOR_APPLICATION_VERSION, INEXOR_ENGINE_VERSION);
		vulkan_error_check(result);

		// TODO: Check if surface is available!

		// Create a window surface using GLFW library.
		result = create_window_surface(instance, window, surface);
		vulkan_error_check(result);
		
		// List up all available graphics cards.
		print_all_physical_devices(instance, surface);

		// Let the user select a graphics card or select the "best" one automatically.
		selected_graphics_card = decide_which_graphics_card_to_use(instance);

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

		// Decide which surface color format is used.
		// The standard format VK_FORMAT_B8G8R8A8_UNORM should be available on every system.
		decide_which_surface_color_format_in_swapchain_to_use(selected_graphics_card, surface, selected_image_format, selected_color_space);
		
		print_driver_vulkan_version();
		print_instance_layers();
		print_instance_extensions();
		print_device_layers(selected_graphics_card);
		print_device_extensions(selected_graphics_card);


		// In this section, we need to check if the setup that we want to make is supported by the system. 

		// Query if presentation is supported.
		if(!check_presentation_availability(selected_graphics_card, surface))
		{
			display_error_message("Error: Presentation is not supported on this system!");
			shutdown_vulkan();
			exit(-1);
		}


		vkGetDeviceQueue(device, selected_queue_family_index, selected_queue_index, &device_queue);

		result = create_swap_chain();
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

		result = create_semaphores();
		vulkan_error_check(result);
		
		result = prepare_drawing();
		vulkan_error_check(result);
	}


	void InexorRenderer::on_window_resized()
	{
		vkDeviceWaitIdle(device);

		// TODO: Implement!
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
