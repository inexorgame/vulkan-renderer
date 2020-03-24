#include "InexorApplication.hpp"
#include "vulkan-renderer/debug-callback/vk_debug_callback.hpp"


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
		auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
		app->frame_buffer_resized = true;
	}


	InexorApplication::InexorApplication()
	{
	}


	InexorApplication::~InexorApplication()
	{
	}

	
	VkResult InexorApplication::load_TOML_configuration_file(const std::string& TOML_file_name)
	{
		spdlog::debug("Loading TOML configuration file: {}", TOML_file_name);

		std::ifstream toml_file;

		// Check if this file exists.
		toml_file.open(TOML_file_name.c_str(), std::ios::in);

		if(!toml_file.is_open())
		{
			spdlog::error("Could not open configuration file: {}!", TOML_file_name);
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		toml_file.close();

		// Load the TOML file using toml11.
		auto renderer_configuration = toml::parse(TOML_file_name);

		// Search for the title of the configuration file and print it to debug output.
		auto configuration_title = toml::find<std::string>(renderer_configuration, "title");
		spdlog::debug("Title: {}", configuration_title);

		window_width  = toml::find<int>(renderer_configuration, "application", "window", "width");
		window_height = toml::find<int>(renderer_configuration, "application", "window", "width");
		window_title  = toml::find<std::string>(renderer_configuration, "application", "window", "name");

		spdlog::debug("window: '{}', {} x {}", window_title, window_width, window_height);

		application_name = toml::find<std::string>(renderer_configuration, "application", "name");
		engine_name = toml::find<std::string>(renderer_configuration, "application", "engine", "name");

		spdlog::debug("application name: {}, engine name: {}", application_name, engine_name);

		int application_version_major = toml::find<int>(renderer_configuration, "application", "version", "major");
		int application_version_minor = toml::find<int>(renderer_configuration, "application", "version", "minor"); 
		int application_version_patch = toml::find<int>(renderer_configuration, "application", "version", "patch");

		spdlog::debug("application version {}.{}.{}", application_version_major, application_version_minor, application_version_patch);

		// Generate an uint32_t value from the major, minor and patch version info.
		application_version = VK_MAKE_VERSION(application_version_major, application_version_minor, application_version_patch);
		
		int engine_version_major = toml::find<int>(renderer_configuration, "application", "engine", "version", "major");
		int engine_version_minor = toml::find<int>(renderer_configuration, "application", "engine", "version", "minor"); 
		int engine_version_patch = toml::find<int>(renderer_configuration, "application", "engine", "version", "patch");

		spdlog::debug("engine version {}.{}.{}", engine_version_major, engine_version_minor, engine_version_patch);

		// Generate an uint32_t value from the major, minor and patch version info.
		engine_version = VK_MAKE_VERSION(engine_version_major, engine_version_minor, engine_version_patch);

		texture_files = toml::find<std::vector<std::string>>(renderer_configuration, "textures", "files");
		
		spdlog::debug("textures:");

		for(const auto& texture_file : texture_files)
		{
			spdlog::debug("{}", texture_file);
		}

		gltf_model_files = toml::find<std::vector<std::string>>(renderer_configuration, "glTFmodels", "files");
		
		spdlog::debug("glTF 2.0 models:");

		for(const auto& gltf_model_file : gltf_model_files)
		{
			spdlog::debug("{}", gltf_model_file);
		}

		vertex_shader_files = toml::find<std::vector<std::string>>(renderer_configuration, "shaders", "vertex", "files");
		
		spdlog::debug("vertex shaders:");

		for(const auto& vertex_shader_file : vertex_shader_files)
		{
			spdlog::debug("{}", vertex_shader_file);
		}
		
		fragment_shader_files = toml::find<std::vector<std::string>>(renderer_configuration, "shaders", "fragment", "files");
				
		spdlog::debug("fragment shaders:");

		for(const auto& fragment_shader_file : fragment_shader_files)
		{
			spdlog::debug("{}", fragment_shader_file);
		}

		return VK_SUCCESS;
	}


	VkResult InexorApplication::load_textures()
	{
		assert(device);
		assert(selected_graphics_card);
		assert(debug_marker_manager);
		assert(vma_allocator);
		
		// Initialise texture manager.
		VkResult result = VulkanTextureManager::initialise(device, selected_graphics_card, debug_marker_manager, vma_allocator, get_graphics_family_index().value(), get_graphics_queue());
		vulkan_error_check(result);
		
		// TODO: Refactor! use key from TOML file as name!
		std::size_t texture_number = 1;


		for(const auto& texture_file : texture_files)
		{
			std::string texture_name = "example_texture_"+ std::to_string(texture_number);
			texture_number++;

			// TODO: Find duplicate loads!
			// TOOD: Specify assets folder!
			result = VulkanTextureManager::create_texture_from_file(texture_name, texture_file);
			vulkan_error_check(result);

			auto new_texture = get_texture(texture_name);

			assert(new_texture.has_value());

			// Store the texture.
			textures.push_back(new_texture.value());
		}
		
		return VK_SUCCESS;
	}


	VkResult InexorApplication::load_shaders()
	{
		assert(device);

		spdlog::debug("Loading vertex shaders.");

		if(0 == vertex_shader_files.size())
		{
			spdlog::error("No vertex shaders to load!");
		}

		// Loop through the list of vertex shaders and initialise all of them.
		for(const auto& vertex_shader : vertex_shader_files)
		{
			spdlog::debug("Loading vertex shader file {}.", vertex_shader);
			
			VkResult result = create_shader_from_file(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader, vertex_shader, "main");
			if(VK_SUCCESS != result)
			{
				vulkan_error_check(result);
				std::string error_message = "Error: Could not initialise vertex shader " +  vertex_shader;
				display_error_message(error_message);
				exit(-1);
			}
		}

		spdlog::debug("Loading fragment shaders.");

		if(0 == fragment_shader_files.size())
		{
			spdlog::error("No fragment shaders to load!");
		}

		// Loop through the list of fragment shaders and initialise all of them.
		for(const auto& fragment_shader : fragment_shader_files)
		{
			spdlog::debug("Loading fragment shader file {}.", fragment_shader);
			
			VkResult result = create_shader_from_file(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader, fragment_shader, "main");
			if(VK_SUCCESS != result)
			{
				vulkan_error_check(result);
				std::string error_message = "Error: Could not initialise fragment shader " +  fragment_shader;
				display_error_message(error_message);
				exit(-1);
			}
		}

		spdlog::debug("Loading shaders finished.");

		return VK_SUCCESS;
	}


	// TODO: Refactor rendering method!
	VkResult InexorApplication::draw_frame()
	{
		assert(device);
		assert(VulkanQueueManager::get_graphics_queue());
		assert(VulkanQueueManager::get_present_queue());

		vkWaitForFences(device, 1, &(*in_flight_fences[current_frame]), VK_TRUE, UINT64_MAX);

		uint32_t image_index = 0;
		VkResult result = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, *image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index);
		
		if(VK_NULL_HANDLE != images_in_flight[image_index])
		{
			vkWaitForFences(device, 1, &*images_in_flight[image_index], VK_TRUE, UINT64_MAX);
		}
		
		// Update the data which changes every frame!
		update_uniform_buffer(current_frame);

		// Mark the image as now being in use by this frame.
		images_in_flight[image_index] = in_flight_fences[current_frame];

		// Is it time to regenerate the swapchain because window has been resized or minimized?
		if(VK_ERROR_OUT_OF_DATE_KHR == result)
		{
			// VK_ERROR_OUT_OF_DATE_KHR: The swap chain has become incompatible with the surface
			// and can no longer be used for rendering. Usually happens after a window resize.
			return recreate_swapchain(mesh_buffers);
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
		submit_info.pWaitSemaphores      = &*image_available_semaphores[current_frame];
		submit_info.pSignalSemaphores    = &*rendering_finished_semaphores[current_frame];

		vkResetFences(device, 1, &*in_flight_fences[current_frame]);


		result = vkQueueSubmit(VulkanQueueManager::get_graphics_queue(), 1, &submit_info, *in_flight_fences[current_frame]);
		if(VK_SUCCESS != result) return result;

		present_info.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		present_info.pNext              = nullptr;
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores    = &*rendering_finished_semaphores[current_frame];
		present_info.swapchainCount     = 1;
		present_info.pSwapchains        = &swapchain;
		present_info.pImageIndices      = &image_index;
		present_info.pResults           = nullptr;

		result = vkQueuePresentKHR(VulkanQueueManager::get_present_queue(), &present_info);

		// Some notes on frame_buffer_resized:
		// It is important to do this after vkQueuePresentKHR to ensure that the semaphores are
		// in a consistent state, otherwise a signalled semaphore may never be properly waited upon.
		if(VK_ERROR_OUT_OF_DATE_KHR == result || VK_SUBOPTIMAL_KHR == result || frame_buffer_resized)
		{
			frame_buffer_resized = false;
			recreate_swapchain(mesh_buffers);
		}
		
        current_frame = (current_frame + 1) % INEXOR_MAX_FRAMES_IN_FLIGHT;

		return VK_SUCCESS;
	}
	

	VkResult InexorApplication::load_models()
	{
		assert(debug_marker_manager);
		
		spdlog::debug("Creating vertex buffers.");
		
		std::vector<InexorVertex> vertices1;
		std::vector<uint32_t> indices1;

		load_model_from_glTF_file("assets/models/monkey/monkey_triangulated.gltf", vertices1/*, indices1*/);

		VkResult result = create_vertex_buffer("Example vertex buffer 1", vertices1, mesh_buffers);
		
		spdlog::debug("Vertex buffer setup finished.");

		return result;
	}


	VkResult InexorApplication::check_application_specific_features()
	{
		// Check if anisotropic filtering is available!
		VkPhysicalDeviceFeatures graphics_card_features;

		// Check which features are supported by this graphics card.
		vkGetPhysicalDeviceFeatures(selected_graphics_card, &graphics_card_features);

		if(!graphics_card_features.samplerAnisotropy)
		{
			spdlog::warn("The selected graphics card does NOT support anisotropic filtering!");
		}

		return VK_SUCCESS;
	}


	VkResult InexorApplication::initialise()
	{
		// Load the configuration from the TOML file.
		VkResult result = load_TOML_configuration_file("configuration/renderer.toml");
		vulkan_error_check(result);

		spdlog::debug("Initialising vulkan-renderer.");
		spdlog::debug("Creating window.");

		// Create a resizable window using GLFW library.
		create_window(window_width, window_height, window_title, true);

		spdlog::debug("Storing GLFW window user pointer.");

		// Store the current InexorApplication instance in the GLFW window user pointer.
		// Since GLFW is a C-style API, we can't use a class method as callback for window resize!
		glfwSetWindowUserPointer(window, this);

		spdlog::debug("Setting up framebuffer resize callback.");

		// Setup callback for window resize.
		// Since GLFW is a C-style API, we can't use a class method as callback for window resize!
		glfwSetFramebufferSizeCallback(window, frame_buffer_resize_callback);

		spdlog::debug("Checking for '-renderdoc' command line argument.");

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
		
		spdlog::debug("Checking for '-novalidation' command line argument.");
		
		bool enable_khronos_validation_instance_layer = true;

		// If the user specified command line argument "-novalidation", the Khronos validation instance layer will be disabled.
		// For development builds, this is not advisable! Always use validation layers during development!
		std::optional<bool> disable_validation = is_command_line_argument_specified("-novalidation");

		if(disable_validation.has_value())
		{
			if(disable_validation.value())
			{
				spdlog::warn("Vulkan validation layers DISABLED by command line argument -novalidation!.");
				enable_khronos_validation_instance_layer = false;
			}
		}

		spdlog::debug("Creating Vulkan instance.");

		// Create a Vulkan instance.
		result = create_vulkan_instance(application_name, engine_name, application_version, engine_version, enable_khronos_validation_instance_layer, enable_renderdoc_instance_layer);
		vulkan_error_check(result);
		

		// Create a debug callback using VK_EXT_debug_utils
		// Check if validation is enabled check for availabiliy of VK_EXT_debug_utils.
		if(enable_khronos_validation_instance_layer)
		{
			spdlog::debug("Khronos validation layer is enabled.");

			if(availability_checks::is_instance_extension_available(VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
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

		bool use_distinct_data_transfer_queue = true;

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
		
		result = prepare_queues(selected_graphics_card, surface, use_distinct_data_transfer_queue);
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

		result = check_application_specific_features();
		vulkan_error_check(result);

		// Vulkan debug markes will be very useful when debugging with RenderDoc!
		result = initialise_debug_marker_manager(enable_debug_marker_device_extension);
		vulkan_error_check(result);

		VulkanShaderManager::initialise(device, debug_marker_manager);

		result = create_vma_allocator();
		vulkan_error_check(result);

		result = setup_queues(device);
		vulkan_error_check(result);

		result = create_swapchain();
		vulkan_error_check(result);

		result = create_depth_buffer();
		vulkan_error_check(result);

		result = load_textures();
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

		InexorMeshBufferManager::initialise(device, debug_marker_manager, vma_allocator, VulkanQueueManager::get_data_transfer_queue_family_index().value(), VulkanQueueManager::get_data_transfer_queue());

		result = create_command_pool();
		vulkan_error_check(result);

		VulkanUniformBufferManager::initialise(device, debug_marker_manager, vma_allocator);

		result = create_uniform_buffers();
		vulkan_error_check(result);

		result = create_descriptor_pool();
		vulkan_error_check(result);

		result = create_descriptor_sets();
		vulkan_error_check(result);

		result = create_command_buffers();
		vulkan_error_check(result);

		result = load_models();
		vulkan_error_check(result);

		result = record_command_buffers(mesh_buffers);
		vulkan_error_check(result);

		VulkanFenceManager::initialise(device, debug_marker_manager);

		VulkanSemaphoreManager::initialise(device, debug_marker_manager);

		result = create_synchronisation_objects();
		vulkan_error_check(result);

		spdlog::debug("Vulkan initialisation finished.");

		spdlog::debug("Showing window.");
		
		// Show window after Vulkan has been initialised.
		glfwShowWindow(window);

		return VK_SUCCESS;
	}


	void InexorApplication::run()
	{
		spdlog::debug("Running InexorApplication.");

		// TODO: Run this in a separated thread?
		while(!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
			draw_frame();
		}
	}


	void InexorApplication::cleanup()
	{
		spdlog::debug("Cleaning up InexorRenderer.");

		shutdown_vulkan();
		destroy_window();

		mesh_buffers.clear();
		vertex_shader_files.clear();
		fragment_shader_files.clear();
		texture_files.clear();
		shader_files.clear();
		gltf_model_files.clear();
	}


};
};
