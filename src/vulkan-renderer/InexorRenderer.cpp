#include "InexorRenderer.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	InexorRenderer::InexorRenderer()
	{
		window = nullptr;
		vulkan_instance = {};
		vulkan_device = {};
		number_of_physical_devices = 0;
		graphics_cards.clear();
		window_width = 0;
		window_height = 0;
		image_views.clear();
		number_of_images_in_swap_chain = 0;
		vulkan_device_ready = false;
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

		// TODO: Why is std::numeric_limits<uint64_t>::max() not working instead of UINT64_MAX?
		vkAcquireNextImageKHR(vulkan_device, vulkan_swapchain, UINT64_MAX, semaphore_image_available, VK_NULL_HANDLE, &image_index);

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
		init_window(800, 600, "Inexor Vulkan Renderer");
		init_vulkan();

		print_instance_layer_properties();
		print_instance_extensions();
		print_device_layers(selected_graphics_card);

		check_support_of_presentation(selected_graphics_card);

		vkGetDeviceQueue(vulkan_device, 0, 0, &queue);

		setup_swap_chain();
		load_shaders();
		setup_pipeline();
		setup_frame_buffers();
		create_command_buffers();
		create_semaphores();
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
