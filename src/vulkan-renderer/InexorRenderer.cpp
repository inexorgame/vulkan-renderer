#include "InexorRenderer.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	InexorRenderer::InexorRenderer()
	{
		vulkan_instance = {};
		vulkan_device = {};
		number_of_graphics_cards = 0;
		graphics_cards.clear();
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

		vkAcquireNextImageKHR(vulkan_device, vulkan_swapchain, UINT64_MAX, semaphore_image_available, VK_NULL_HANDLE, &image_index);
		// TODO: Error checking?

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

		VkResult result = vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE);
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

		result = vkQueuePresentKHR(queue, &present_info);
		vulkan_error_check(result);
	}
	
	
	void InexorRenderer::init()
	{
		// Check which version of the Vulkan API is available.
		print_driver_vulkan_version();
		print_instance_layer_properties();
		print_instance_extensions();
		
		// Create a window using GLFW library.
		init_window(INEXOR_WINDOW_WIDTH, INEXOR_WINDOW_HEIGHT, "Inexor Vulkan Renderer");

		VkResult result = create_vulkan_instance(INEXOR_APPLICATION_NAME, INEXOR_ENGINE_NAME, INEXOR_APPLICATION_VERSION, INEXOR_ENGINE_VERSION);
		vulkan_error_check(result);

		// Let the user select a graphics card or select the "best" one automatically.
		selected_graphics_card = decide_which_graphics_card_to_use();

		// Create a physical device with the selected graphics card.
		result = create_physical_device(selected_graphics_card);
		vulkan_error_check(result);

		// The standard format VK_FORMAT_B8G8R8A8_UNORM should be available on every system.
		selected_image_format = decide_which_image_format_to_use();

		print_device_layers(selected_graphics_card);

		// In this section, we need to check if the setup that we want to make is supported by the system. 

		// TODO: class VulkanBestPractices
		//decide_how_many_images_in_swap_chain_to_use();
		//decide_which_image_color_space_to_use();
		//decide_which_image_sharing_mode_to_use();
		//decide_which_present_mode_to_use();

		check_support_of_presentation(selected_graphics_card);


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
