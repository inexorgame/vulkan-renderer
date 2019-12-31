#include "InexorRenderer.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	InexorRenderer::InexorRenderer()
	{
		vulkan_instance = {};
		vulkan_device = {};
		image_views.clear();
		number_of_images_in_swap_chain = 0;
		frame_buffers.clear();
	}


	InexorRenderer::~InexorRenderer()
	{
	}


	void InexorRenderer::load_shaders()
	{
		// TODO: Setup shaders from JSON file.
		
		// Important: Make sure your debug directory contains these files!
		create_shader_module_from_file(vulkan_device, "vertex_shader.spv", &vertex_shader_module);
		create_shader_module_from_file(vulkan_device, "fragment_shader.spv", &fragment_shader_module);
	}


	void InexorRenderer::draw_frame()
	{
		uint32_t image_index = 0;

		VkResult result = vkAcquireNextImageKHR(vulkan_device, vulkan_swapchain, UINT64_MAX, semaphore_image_available, VK_NULL_HANDLE, &image_index);
		vulkan_error_check(result);

		// TODO: Do not fill these structures every call!

		VkSubmitInfo submit_info = {};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.pNext = nullptr;
		submit_info.waitSemaphoreCount = 1;
		submit_info.pWaitSemaphores = &semaphore_image_available;

		VkPipelineStageFlags wait_stage_mask[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
		submit_info.pWaitDstStageMask = wait_stage_mask;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffers[image_index];
		submit_info.signalSemaphoreCount = 1;
		submit_info.pSignalSemaphores = &semaphore_rendering_finished;

		result = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
		vulkan_error_check(result);

		VkPresentInfoKHR present_info = {};
		present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext = nullptr;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &semaphore_rendering_finished;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &vulkan_swapchain;
		present_info.pImageIndices = &image_index;
		present_info.pResults = nullptr;

		// TODO: Implement a base class VulkanPresentationEngine?
		result = vkQueuePresentKHR(queue, &present_info);
		vulkan_error_check(result);
	}
	
	
	void InexorRenderer::init()
	{
		// Create a window using GLFW library.
		init_window(INEXOR_WINDOW_WIDTH, INEXOR_WINDOW_HEIGHT, INEXOR_APP_WINDOW_TITLE);

		// Create a vulkan instance.
		VkResult result = create_vulkan_instance(INEXOR_APPLICATION_NAME, INEXOR_ENGINE_NAME, INEXOR_APPLICATION_VERSION, INEXOR_ENGINE_VERSION);
		vulkan_error_check(result);

		// TODO: Check if surface is available!

		// Create a window surface using GLFW.
		create_window_surface(vulkan_instance, window, vulkan_surface);

		print_driver_vulkan_version();
		print_instance_layers();
		print_instance_extensions();
		
		// List up all available graphics cards.
		print_all_physical_devices(vulkan_instance, vulkan_surface);

		// Let the user select a graphics card or select the "best" one automatically.
		selected_graphics_card = decide_which_graphics_card_to_use(vulkan_instance);

		if(!check_swapchain_availability(selected_graphics_card))
		{
			display_error_message("Error: Swapchain device extension is not supported on this system!");
			// TODO: Shutdown Vulkan and application!
		}

		// TODO: Design consistent error handling strategy!
		// TODO: Use nullptr instead of NULL consistently!

		// Create a physical device with the selected graphics card.
		result = create_physical_device(selected_graphics_card);
		vulkan_error_check(result);

		// The standard format VK_FORMAT_B8G8R8A8_UNORM should be available on every system.
		selected_image_format = decide_which_surface_color_format_for_swap_chain_images_to_use(selected_graphics_card, vulkan_surface);
		
		print_device_layers(selected_graphics_card);
		print_device_extensions(selected_graphics_card);


		// In this section, we need to check if the setup that we want to make is supported by the system. 

		// Query if presentation is supported.
		// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
		if(!check_presentation_availability(selected_graphics_card, vulkan_surface))
		{
			display_error_message("Error: Presentation is not supported on this system!");
			// TODO: Shutdown Vulkan and application!
		}


		create_device_queue();

		create_swap_chain();
		
		create_image_views();

		load_shaders();
		
		create_pipeline();
		
		create_frame_buffers();
		
		create_command_pool();

		create_command_buffers();

		record_command_buffers();

		create_semaphores();
	}


	void InexorRenderer::on_window_resized()
	{
		vkDeviceWaitIdle(vulkan_device);

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
		shutdown_window();
	}


};
};
