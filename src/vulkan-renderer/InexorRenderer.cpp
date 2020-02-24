#include "InexorRenderer.hpp"
#include "debug-callback/VulkanDebugCallback.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	/// @brief Static callback for window resize events.
	/// @note Because GLFW is a C-style API, we can't pass a poiner to a class method, so we have to do it this way!
	/// @param window The GLFW window.
	/// @param height The width of the window.
	/// @param height The height of the window.
	static void frame_buffer_resize_callback(GLFWwindow* window, int width, int height)
	{
		spdlog::debug("Frame buffer resize callback called. window width: {}, height: {}", width, height);

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
		assert(device);

		spdlog::debug("Loading shader files.");

		// It is important to make sure that you debugging folder contains the required shader files!
		struct InexorShaderSetup
		{
			VkShaderStageFlagBits shader_type;
			std::string shader_file_name;
		};

		// TODO: VulkanPipelineManager!
		
		// The actual file list of shaders that we want to load.
		// TODO: Setup shaders JSON or TOML list file.
		const std::vector<InexorShaderSetup> shader_list = 
		{
			{VK_SHADER_STAGE_VERTEX_BIT, "vertexshader.spv"},
			{VK_SHADER_STAGE_FRAGMENT_BIT, "fragmentshader.spv"}
			// Add more shaders here..
		};

		for(const auto& shader : shader_list)
		{
			spdlog::debug("Loading shader file {}.", shader.shader_file_name);
			
			VkResult result = create_shader_from_file(device, shader.shader_type, shader.shader_file_name);
			if(VK_SUCCESS != result)
			{
				vulkan_error_check(result);
				std::string error_message = "Error: Could not initialise shader " +  shader.shader_file_name;
				display_error_message(error_message);
				exit(-1);
			}
		}

		spdlog::debug("Finished loading shaders.");

		return VK_SUCCESS;
	}


	// TODO: Refactor rendering!
	VkResult InexorRenderer::draw_frame()
	{
		assert(device);
		assert(graphics_queue);
		assert(present_queue);

		vkWaitForFences(device, 1, &in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

		uint32_t image_index = 0;
		VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
		
		if(VK_NULL_HANDLE != images_in_flight[image_index])
		{
			vkWaitForFences(device, 1, &images_in_flight[image_index], VK_TRUE, UINT64_MAX);
		}
		
		update_uniform_buffer(current_frame);

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
		spdlog::debug("Initialising vulkan-renderer.");
		spdlog::debug("Creating window.");

		// Create a resizable window using GLFW library.
		create_window(INEXOR_WINDOW_WIDTH, INEXOR_WINDOW_HEIGHT, INEXOR_WINDOW_TITLE, true);

		spdlog::debug("Storing GLFW window user pointer.");

		// Store the current InexorRenderer instance in the GLFW window user pointer.
		// Since GLFW is a C-style API, we can't use a class method as callback for window resize!
		glfwSetWindowUserPointer(window, this);

		spdlog::debug("Setting up framebuffer resize callback.");

		// Setup callback for window resize.
		// Since GLFW is a C-style API, we can't use a class method as callback for window resize!
		glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);

		spdlog::debug("Checking for -renderdoc command line argument.");

		bool enable_renderdoc_instance_layer = false;
		
		// If the user specified command line argument "-renderdoc", the RenderDoc instance layer will be enabled.
		std::optional<bool> enable_renderdoc = is_command_line_argument_specified("-renderdoc");
		
		if(enable_renderdoc.has_value())
		{
			if(enable_renderdoc.value())
			{
				spdlog::debug("RenderDoc command line argument specified.");
				enable_renderdoc_instance_layer = true;
			}
		}
		
		spdlog::debug("Checking for -novalidation command line argument.");
		
		bool enable_khronos_validation_instance_layer = true;

		// If the user specified command line argument "-novalidation", the Khronos validation instance layer will be disabled.
		// For development builds, this is not advisable! Always use validation layers during development!
		std::optional<bool> disable_validation = is_command_line_argument_specified("-novalidation");

		if(disable_validation.has_value())
		{
			if(disable_validation.value())
			{
				spdlog::debug("No Vulkan validation layers command line argument specified.");
				enable_khronos_validation_instance_layer = false;
			}
		}

		spdlog::debug("Creating Vulkan instance.");

		// Create a Vulkan instance.
		VkResult result = create_vulkan_instance(INEXOR_APPLICATION_NAME, INEXOR_ENGINE_NAME, INEXOR_APPLICATION_VERSION, INEXOR_ENGINE_VERSION, enable_khronos_validation_instance_layer, enable_renderdoc_instance_layer);
		vulkan_error_check(result);
		

		// Create a debug callback using VK_EXT_debug_utils
		// Check if validation is enabled check for availabiliy of VK_EXT_debug_utils.
		if(enable_khronos_validation_instance_layer)
		{
			spdlog::debug("Khronos validation layer is enabled.");

			if(VulkanAvailabilityChecks::is_instance_extension_available(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
			{
				VkDebugReportCallbackCreateInfoEXT debug_report_create_info = {};
			
				debug_report_create_info.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
				debug_report_create_info.flags       = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT;
				debug_report_create_info.pfnCallback = (PFN_vkDebugReportCallbackEXT)&VulkanDebugMessageCallback;

				// We have to explicitly load this function.
				PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));

				if(nullptr != vkCreateDebugReportCallbackEXT)
				{
					// Create the debug report callback.
					VkResult result = vkCreateDebugReportCallbackEXT(instance, &debug_report_create_info, nullptr, &debug_report_callback);
					if(VK_SUCCESS == result)
					{
						spdlog::debug("Creating Vulkan debug callback.");
						debug_report_callback_initialised = true;
					}
					else
					{
						vulkan_error_check(result);
					}
				}
				else
				{
					spdlog::error("vkCreateDebugReportCallbackEXT is a null-pointer! Function not available.");
				}
			}
			else
			{
				spdlog::warn("Khronos validation layer is not available!");
			}
		}
		else
		{
			spdlog::warn("Khronos validation layer is DISABLED.");
		}
		
		spdlog::debug("Creating window surface.");

		// Create a window surface using GLFW library.
		// @note The window surface needs to be created right after the instance creation,
		// because it can actually influence the physical device selection.
		result = create_window_surface(instance, window, surface);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}
		
		spdlog::debug("Checking for -gpu command line argument.");

		// The user can specify with "-gpu <number" which graphics card to prefer.
		std::optional<uint32_t> prefered_graphics_card = get_command_line_argument_uint32_t("-gpu");

		if(prefered_graphics_card.has_value())
		{
			spdlog::debug("Prefered graphics card index {} specified.", prefered_graphics_card.value());
		}

		// Let's see if there is a graphics card that is suitable for us.
		std::optional<VkPhysicalDevice> graphics_card_candidate = VulkanSettingsDecisionMaker::which_graphics_card_to_use(instance, surface, prefered_graphics_card);

		// Check if we found a graphics card candidate.
		if(graphics_card_candidate.has_value())
		{
			selected_graphics_card = graphics_card_candidate.value();
		}
		else
		{
			// No graphics card suitable!
			std::string error_message = "Error: Could not find any suitable GPU!";
			display_fatal_error_message(error_message);

			return VK_ERROR_INITIALIZATION_FAILED;
		}

		bool display_graphics_card_info = true;
		
		spdlog::debug("Checking for -nostats command line argument.");

		// If the user specified command line argument "-nostats", no information will be 
		// displayed about all the graphics cards which are available on the system.
		std::optional<bool> hide_gpu_stats = is_command_line_argument_specified("-nostats");
		
		if(hide_gpu_stats.has_value())
		{
			if(hide_gpu_stats.value())
			{
				spdlog::debug("No extended information about graphics cards will be shown.");
				display_graphics_card_info = false;
			}
		}

		if(display_graphics_card_info)
		{
			spdlog::debug("Displaying extended information about graphics cards.");
			
			// Print general information about Vulkan.
			print_driver_vulkan_version();
			print_instance_layers();
			print_instance_extensions();

			// Print all information that we can find about all graphics card available.
			print_all_physical_devices(instance, surface);
		}
		
		spdlog::debug("Checking for -no_separate_data_queue command line argument.");

		// Ignore distinct data transfer queue.
		std::optional<bool> forbid_distinct_data_transfer_queue = is_command_line_argument_specified("-no_separate_data_queue");

		if(forbid_distinct_data_transfer_queue.has_value())
		{
			if(forbid_distinct_data_transfer_queue.value())
			{
				spdlog::warn("Command line argument -no_separate_data_queue specified.");
				spdlog::warn("This will force the application to avoid using a distinct queue for data transfer to GPU.");
				spdlog::warn("Performance loss might be a result of this!");
				use_distinct_data_transfer_queue = false;
			}
		}
		
		result = create_device_queues(use_distinct_data_transfer_queue);
		vulkan_error_check(result);

		spdlog::debug("Checking for -no_vk_debug_markers command line argument.");

		bool enable_debug_marker_device_extension = true;

		if(!enable_renderdoc_instance_layer)
		{
			// Debug markers are only available if RenderDoc is enabled.
			enable_debug_marker_device_extension = false;
		}

		// Check if vulkan debug markers should be disabled.
		// Those are only available if RenderDoc instance layer is enabled!
		std::optional<bool> no_vulkan_debug_markers = is_command_line_argument_specified("-no_vk_debug_markers");
		
		if(no_vulkan_debug_markers.has_value())
		{
			if(no_vulkan_debug_markers.value())
			{
				spdlog::warn("Vulkan debug markers are disabled because -no_vk_debug_markers was specified.");
				enable_debug_marker_device_extension = false;
			}
		}


		result = create_physical_device(selected_graphics_card, enable_debug_marker_device_extension);
		vulkan_error_check(result);

		// Initialise Vulkan debug markers.
		// Those debug markes will be very useful when debugging with RenderDoc!
		result = initialise_debug_marker_manager(enable_debug_marker_device_extension);
		vulkan_error_check(result);

		// Initialise shader manager.
		VulkanShaderManager::initialise(debug_marker_manager);

		// Initialise Vulkan Memory Allocator.
		result = create_vma_allocator();
		vulkan_error_check(result);

		result = initialise_queues();
		vulkan_error_check(result);

		if(!use_distinct_data_transfer_queue)
		{
			spdlog::warn("The application is forces to avoid distinct data transfer queues.");
			spdlog::warn("Because of this, the graphics queue will be used for data transfer.");
			
			// In case -no_separate_data_queue is specified as command line argument,
			// we will use the graphics queue for data transfer.
			data_transfer_queue = graphics_queue;
			data_transfer_queue_family_index = graphics_queue_family_index;
		}

		result = create_swapchain();
		vulkan_error_check(result);
		
		result = create_image_views();
		vulkan_error_check(result);
		
		result = load_shaders();
		vulkan_error_check(result);

		result = create_descriptor_set_layout();
		vulkan_error_check(result);

		result = create_pipeline();
		vulkan_error_check(result);
		
		result = create_frame_buffers();
		vulkan_error_check(result);

		// Create a second command pool for data transfer commands.
		VulkanMeshBufferManager::initialise(device, debug_marker_manager, vma_allocator, data_transfer_queue_family_index.value(), data_transfer_queue);
		
		result = create_command_pool();
		vulkan_error_check(result);

		result = create_uniform_buffers();
		vulkan_error_check(result);

		result = create_descriptor_pool();
		vulkan_error_check(result);

		result = create_descriptor_sets();
		vulkan_error_check(result);

		result = create_command_buffers();
		vulkan_error_check(result);

		result = create_vertex_buffers();
		vulkan_error_check(result);

		result = record_command_buffers();
		vulkan_error_check(result);

		result = create_synchronisation_objects();
		vulkan_error_check(result);

		spdlog::debug("Vulkan initialisation finished.");
		spdlog::debug("Showing window.");
		
		// Show window after Vulkan has been initialised.
		glfwShowWindow(window);

		return VK_SUCCESS;
	}


	void InexorRenderer::run()
	{
		spdlog::debug("Running InexorRenderer application.");

		// TODO: Run this in a separated thread?
		while(!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			draw_frame();
		}
	}


	void InexorRenderer::cleanup()
	{
		spdlog::debug("Cleaning up InexorRenderer.");

		shutdown_vulkan();
		destroy_window();
	}


};
};
