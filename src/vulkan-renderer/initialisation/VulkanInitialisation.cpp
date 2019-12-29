#include "VulkanInitialisation.hpp"


namespace inexor {
namespace vulkan_renderer {


	VulkanInitialisation::VulkanInitialisation()
	{
	}


	VulkanInitialisation::~VulkanInitialisation()
	{
	}


	VkResult VulkanInitialisation::create_vulkan_instance(const std::string& application_name, const std::string& engine_name, const uint32_t application_version, const uint32_t engine_version, bool enable_validation_layers)
	{
		cout << "Initialising Vulkan instance." << endl;
		cout << "Application name: " << application_name.c_str() << endl;
		cout << "Application version: " << VK_VERSION_MAJOR(application_version) << "." << VK_VERSION_MINOR(application_version) << "." << VK_VERSION_PATCH(application_version) << endl;
		cout << "Engine name: " << engine_name.c_str() << endl;
		cout << "Engine version: " << VK_VERSION_MAJOR(engine_version) << "." << VK_VERSION_MINOR(engine_version) << "." << VK_VERSION_PATCH(engine_version) << endl;
		cout << endl;

		// TODO: Check which version of Vulkan is available before trying to create an instance!
		// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkApplicationInfo.html
		// "Because Vulkan 1.0 implementations may fail with VK_ERROR_INCOMPATIBLE_DRIVER,
		// applications should determine the version of Vulkan available before calling vkCreateInstance.
		// If the vkGetInstanceProcAddr returns NULL for vkEnumerateInstanceVersion, it is a Vulkan 1.0
		// implementation. Otherwise, the application can call vkEnumerateInstanceVersion to determine the
		// version of Vulkan."

		// Structure specifying application's Vulkan API info.
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = nullptr;
		app_info.pApplicationName = application_name.c_str();
		app_info.applicationVersion = application_version;
		app_info.pEngineName = engine_name.c_str();
		app_info.engineVersion = engine_version;
		app_info.apiVersion = VK_API_VERSION_1_1;

		// TODO: Check if we need more device or instance extensions!


		// Query which extensions are needed for GLFW.
		uint32_t number_of_GLFW_extensions = 0;
		
		auto glfw_extensions = glfwGetRequiredInstanceExtensions(&number_of_GLFW_extensions);

		cout << "Required GLFW instance extensions: " << endl;

		for(std::size_t i=0; i<number_of_GLFW_extensions; i++)
		{
			cout << glfw_extensions[i] << endl;

			if(!CheckInstanceExtensionAvailability(glfw_extensions[i]))
			{
				std::string error_message = "Error: GLFW required instance extension " + std::string(glfw_extensions[i]) + "not found!";
				display_error_message(error_message);
			}
		}

		cout << endl;

		// A vector of strings which represent the enabled instance layers.
		std::vector<const char*> enabled_instance_layers;

		// The layers that we would like to enable.
		std::vector<const char*> instance_layers_wishlist = {
			//"VK_LAYER_VALVE_steam_overlay",
			//"VK_LAYER_RENDERDOC_Capture"
		};

		// If validation is requested, we need to add the validation layer as instance extension!
		// For more information on Vulkan validation layers see:
		// https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers
		if(enable_validation_layers)
		{
			const char validation_layer_name[] = "VK_LAYER_KHRONOS_validation";
			instance_layers_wishlist.push_back(validation_layer_name);
		}

		// We now have to check which instance layers of our wishlist are really supported on the current system!
		for(auto current_layer : instance_layers_wishlist)
		{
			if(CheckInstanceLayerAvailability(current_layer))
			{
				// This instance layer is available!
				// Add it to the list of enabled instance layers!
				enabled_instance_layers.push_back(current_layer);
			}
			else
			{
				std::string error_message = "Error: instance layer " + std::string(current_layer) + " not available!";
				display_error_message(error_message);
			}
		}

		// Structure specifying parameters of a newly created instance.
		VkInstanceCreateInfo instance_create_info = {};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext = nullptr;
		instance_create_info.flags = 0;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledExtensionCount = number_of_GLFW_extensions;
		instance_create_info.ppEnabledExtensionNames = glfw_extensions;

		// Pass all the enabled layers to Vulkan.
		instance_create_info.ppEnabledLayerNames = enabled_instance_layers.data();
		instance_create_info.enabledLayerCount = static_cast<uint32_t>(enabled_instance_layers.size());

		// Create a new Vulkan instance.
		VkResult result = vkCreateInstance(&instance_create_info, nullptr, &vulkan_instance);
		vulkan_error_check(result);

		// Create a window surface using GLFW library.
		result = glfwCreateWindowSurface(vulkan_instance, window, nullptr, &vulkan_surface);
		vulkan_error_check(result);

		return result;
	}

	
	VkResult VulkanInitialisation::create_physical_device(const VkPhysicalDevice& graphics_card)
	{
		cout << "Creating a physical device" << endl;
		
		const float queue_priorities[] = {1.0f, 1.0f, 1.0f, 1.0f};

		VkDeviceQueueCreateInfo device_queue_create_info = {};
		device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_create_info.pNext = NULL;
		device_queue_create_info.flags = NULL;

		// TODO: Look which queue family fits best for what we want to do.
		// For now we will use index number 0.
		device_queue_create_info.queueFamilyIndex = 0;
		
		// TODO: Check if 4 queues are even supported!
		device_queue_create_info.queueCount = 4;
		device_queue_create_info.pQueuePriorities = queue_priorities;

		// TODO: Fill with required features!
		VkPhysicalDeviceFeatures used_features = {};
		
		VkDeviceCreateInfo device_create_info = {};
		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pNext = NULL;
		device_create_info.flags = NULL;

		// TODO: Maybe create multiple queues at once?
		device_create_info.queueCreateInfoCount = 1;

		// Our wishlist of device extensions that we would like to enable.
		const std::vector<const char*> device_extensions_wishlist = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		// The actual list of enabled device extensions.
		std::vector<const char*> enabled_device_extensions;

		for(auto layer_name : device_extensions_wishlist)
		{
			if(CheckDeviceExtensionAvailability(graphics_card, layer_name))
			{
				// This device layer is supported!
				// Add it to the list of enabled device layers.
				enabled_device_extensions.push_back(layer_name);
			}
			else
			{
				// This device layer is not supported!
				std::string error_message = "Error: Device extension " + std::string(layer_name) + " not supported!";
				display_error_message(error_message);
			}
		}

		device_create_info.pQueueCreateInfos = &device_queue_create_info;
		device_create_info.enabledLayerCount = NULL;
		device_create_info.ppEnabledLayerNames = NULL;
		device_create_info.enabledExtensionCount = static_cast<uint32_t>(enabled_device_extensions.size());
		device_create_info.ppEnabledExtensionNames = enabled_device_extensions.data();
		device_create_info.pEnabledFeatures = &used_features;

		return vkCreateDevice(graphics_card, &device_create_info, NULL, &vulkan_device);
	}


	void VulkanInitialisation::enumerate_physical_devices()
	{
		VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_graphics_cards, NULL);
		vulkan_error_check(result);

		if(number_of_graphics_cards <= 0)
		{
			display_error_message("Error: Could not find any GPU's!");
			exit(-1);
		}

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Number of available graphics cards: " << number_of_graphics_cards << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		// Preallocate memory for the available graphics cards.
		graphics_cards.resize(number_of_graphics_cards);

		// Query information about all the graphics cards available on the system.
		result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_graphics_cards, graphics_cards.data());
		vulkan_error_check(result);

		// Loop through all graphics cards and print information about them.
		for(std::size_t i=0; i<number_of_graphics_cards; i++)
		{
			print_graphics_card_info(graphics_cards[i]);
			print_physical_device_queue_families(graphics_cards[i]);
			print_surface_capabilities(graphics_cards[i], vulkan_surface);
			print_supported_surface_formats(graphics_cards[i], vulkan_surface);
			print_presentation_modes(graphics_cards[i], vulkan_surface);
			cout << endl;
		}
	}


	void VulkanInitialisation::shutdown_vulkan()
	{
		vkDeviceWaitIdle(vulkan_device);

		// It is important to destroy the objects in reversal of the order of creation.
		
		vkDestroySemaphore(vulkan_device, semaphore_image_available, nullptr);
		vkDestroySemaphore(vulkan_device, semaphore_rendering_finished, nullptr);

		// We do not need to reset the command buffers explicitly,
		// since it is covered by vkDestroyCommandPool.
		vkFreeCommandBuffers(vulkan_device, command_pool, number_of_images_in_swap_chain, command_buffers.data());

		vkDestroyCommandPool(vulkan_device, command_pool, nullptr);

		for(std::size_t i=0; i<number_of_images_in_swap_chain; i++)
		{
			vkDestroyFramebuffer(vulkan_device, frame_buffers[i], nullptr);
		}

		vkDestroyPipeline(vulkan_device, vulkan_pipeline, nullptr);
		vkDestroyRenderPass(vulkan_device, render_pass, nullptr);

		for(std::size_t i=0; i<number_of_images_in_swap_chain; i++)
		{
			vkDestroyImageView(vulkan_device, image_views[i], nullptr);
		}

		vkDestroyPipelineLayout(vulkan_device, vulkan_pipeline_layout, nullptr);

		// Destroy shader modules:
		vkDestroyShaderModule(vulkan_device, vertex_shader_module, nullptr);
		vkDestroyShaderModule(vulkan_device, fragment_shader_module, nullptr);

		vkDestroySwapchainKHR(vulkan_device, vulkan_swapchain, nullptr);
		vkDestroySurfaceKHR(vulkan_instance, vulkan_surface, nullptr);
		vkDestroyDevice(vulkan_device, nullptr);		
		vkDestroyInstance(vulkan_instance, nullptr);
	}


	void VulkanInitialisation::create_command_pool()
	{
		VkCommandPoolCreateInfo command_pool_create_info = {};
		command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext = nullptr;
		command_pool_create_info.flags = 0;

		// TODO: Get correct queue with VK_QUEUE_GRAPHICS_BIT!
		command_pool_create_info.queueFamilyIndex = 0;

		VkResult result = vkCreateCommandPool(vulkan_device, &command_pool_create_info, nullptr, &command_pool);
		vulkan_error_check(result);
	}

	
	void VulkanInitialisation::create_command_buffers()
	{
		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
		command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.pNext = nullptr;
		command_buffer_allocate_info.commandPool = command_pool;
		command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = number_of_images_in_swap_chain;

		// Preallocate memory for command buffers.
		command_buffers.resize(number_of_images_in_swap_chain);

		VkResult result = vkAllocateCommandBuffers(vulkan_device, &command_buffer_allocate_info, command_buffers.data());
		vulkan_error_check(result);
	}


	void VulkanInitialisation::record_command_buffers()
	{
		VkCommandBufferBeginInfo command_buffer_begin_info = {};
		command_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext = nullptr;
		command_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		command_buffer_begin_info.pInheritanceInfo = nullptr;

		for(std::size_t i=0; i<number_of_images_in_swap_chain; i++)
		{
			VkResult result = vkBeginCommandBuffer(command_buffers[i], &command_buffer_begin_info);
			vulkan_error_check(result);

			VkRenderPassBeginInfo render_pass_begin_info = {};
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.pNext = nullptr;
			render_pass_begin_info.renderPass = render_pass;
			render_pass_begin_info.framebuffer = frame_buffers[i];
			render_pass_begin_info.renderArea.offset = {0, 0};
			render_pass_begin_info.renderArea.extent = {window_width, window_height};

			VkClearValue clear_value;
			// TODO: Change color if you want another clear color.
			clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};
			render_pass_begin_info.clearValueCount = 1;
			
			render_pass_begin_info.pClearValues = &clear_value;

			vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
			
			vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan_pipeline);

			// TODO: Change number of vertices if neccesary!
			vkCmdDraw(command_buffers[i], 3, 1, 0, 0);

			vkCmdEndRenderPass(command_buffers[i]);

			result = vkEndCommandBuffer(command_buffers[i]);
			vulkan_error_check(result);
		}
	}


	void VulkanInitialisation::create_semaphores()
	{
		VkSemaphoreCreateInfo semaphore_create_info = {};
		semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphore_create_info.pNext = nullptr;
		semaphore_create_info.flags = 0;

		VkResult result = vkCreateSemaphore(vulkan_device, &semaphore_create_info, nullptr, &semaphore_image_available);
		vulkan_error_check(result);

		result = vkCreateSemaphore(vulkan_device, &semaphore_create_info, nullptr, &semaphore_rendering_finished);
		vulkan_error_check(result);
	}


	void VulkanInitialisation::check_support_of_presentation(const VkPhysicalDevice& graphics_card)
	{
		VkBool32 surface_support = false;
		
		VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, 0, vulkan_surface, &surface_support);
		vulkan_error_check(result);

		if(!surface_support)
		{
			display_error_message("Error: Surface not supported!");
		}

		cout << "Presentation is supported." << endl;
	}

	
	void VulkanInitialisation::create_device_queue()
	{
		vkGetDeviceQueue(vulkan_device, 0, 0, &queue);
	}


	void VulkanInitialisation::create_swap_chain()
	{
		cout << "Creating swap chain." << endl;

		// TODO: What else needs to be checked ?
		VkSwapchainCreateInfoKHR swap_chain_create_info = {};

		swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swap_chain_create_info.pNext = nullptr;
		swap_chain_create_info.flags = 0;
		swap_chain_create_info.surface = vulkan_surface;

		// TODO: Check if system supports the number specified here!
		swap_chain_create_info.minImageCount = 3;

		// TODO: Check if system supports this image format!
		swap_chain_create_info.imageFormat = selected_image_format;

		// TODO: Check if system supports this image color space!
		swap_chain_create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

		swap_chain_create_info.imageExtent = VkExtent2D{window_width, window_height};
		swap_chain_create_info.imageArrayLayers = 1;
		swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		// TODO: Check if system supports this image sharing mode!
		swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swap_chain_create_info.queueFamilyIndexCount = 0;
		swap_chain_create_info.pQueueFamilyIndices = nullptr;
		swap_chain_create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		// TODO: Check if there is a better presentation mode available!
		// An alternative would be VK_PRESENT_MODE_MAILBOX_KHR.
		swap_chain_create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;

		swap_chain_create_info.clipped = VK_TRUE;

		// TODO: Make window resizable and recreate swap chain.
		// When recreating the swap chain this is needed.
		swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;

		VkResult result = vkCreateSwapchainKHR(vulkan_device, &swap_chain_create_info, nullptr, &vulkan_swapchain);
		vulkan_error_check(result);
	}

	
	void VulkanInitialisation::create_pipeline()
	{
		VkPipelineShaderStageCreateInfo vertex_shader_stage_create_info = {};
		vertex_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertex_shader_stage_create_info.pNext = nullptr;
		vertex_shader_stage_create_info.flags = 0;
		vertex_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertex_shader_stage_create_info.module = vertex_shader_module;
		
		// Usually shaders have a function called "main" as entry point.
		vertex_shader_stage_create_info.pName = "main";
		vertex_shader_stage_create_info.pSpecializationInfo = nullptr;

		VkPipelineShaderStageCreateInfo fragment_shader_stage_create_info = {};
		fragment_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragment_shader_stage_create_info.pNext = nullptr;
		fragment_shader_stage_create_info.flags = 0;
		fragment_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragment_shader_stage_create_info.module = fragment_shader_module;
		
		// Usually shaders have a function called "main" as entry point.
		fragment_shader_stage_create_info.pName = "main";
		fragment_shader_stage_create_info.pSpecializationInfo = nullptr;
		
		// Put all the required shaders into one array.
		shader_stages.push_back(vertex_shader_stage_create_info);
		shader_stages.push_back(fragment_shader_stage_create_info);

		// Now we set up fixed functions.
		VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {};
		vertex_input_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_create_info.pNext = nullptr;
		vertex_input_create_info.flags = 0;
		vertex_input_create_info.vertexBindingDescriptionCount = 0;
		vertex_input_create_info.pVertexBindingDescriptions = nullptr;
		vertex_input_create_info.vertexAttributeDescriptionCount = 0;
		vertex_input_create_info.pVertexAttributeDescriptions = nullptr;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
		input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_create_info.pNext = nullptr;
		input_assembly_create_info.flags = 0;
		input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

		// Setup viewport.
		VkViewport view_port = {};
		view_port.x = 0.0f;
		view_port.y = 0.0f;
		view_port.width = static_cast<float>(window_width);
		view_port.height = static_cast<float>(window_height);
		view_port.minDepth = 0.0f;
		view_port.maxDepth = 1.0f;

		VkRect2D scissor;
		scissor.offset = {0, 0};
		scissor.extent = {window_width, window_height};

		VkPipelineViewportStateCreateInfo pipeline_viewport_viewport_state_info = {};
		pipeline_viewport_viewport_state_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipeline_viewport_viewport_state_info.pNext = nullptr;
		pipeline_viewport_viewport_state_info.flags = 0;
		pipeline_viewport_viewport_state_info.viewportCount = 1;
		pipeline_viewport_viewport_state_info.pViewports = &view_port;
		pipeline_viewport_viewport_state_info.scissorCount = 1;
		pipeline_viewport_viewport_state_info.pScissors = &scissor;


		// Setup rasterizer.
		VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info;
		pipeline_rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline_rasterization_state_create_info.pNext = nullptr;
		pipeline_rasterization_state_create_info.flags = 0;
		pipeline_rasterization_state_create_info.depthClampEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
		pipeline_rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
		pipeline_rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
		pipeline_rasterization_state_create_info.depthBiasEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
		pipeline_rasterization_state_create_info.depthBiasClamp = 0.0f;
		pipeline_rasterization_state_create_info.depthBiasSlopeFactor = 0.0f;
		pipeline_rasterization_state_create_info.lineWidth = 1.0f;

		// Multisampling.
		VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
		multisample_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_create_info.pNext = nullptr;
		multisample_create_info.flags = 0;
		multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_create_info.sampleShadingEnable = VK_FALSE;
		multisample_create_info.minSampleShading = 1.0f;
		multisample_create_info.pSampleMask = nullptr;
		multisample_create_info.alphaToCoverageEnable = VK_FALSE;
		multisample_create_info.alphaToOneEnable = VK_FALSE;

		
		VkPipelineColorBlendAttachmentState color_blend_attachment = {};
		color_blend_attachment.blendEnable = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


		VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
		color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_create_info.pNext = nullptr;
		color_blend_state_create_info.flags = 0;
		color_blend_state_create_info.logicOpEnable = VK_FALSE;
		color_blend_state_create_info.logicOp = VK_LOGIC_OP_NO_OP;
		color_blend_state_create_info.attachmentCount = 1;
		color_blend_state_create_info.pAttachments = &color_blend_attachment;
		color_blend_state_create_info.blendConstants[0] = 0.0f;
		color_blend_state_create_info.blendConstants[1] = 0.0f;
		color_blend_state_create_info.blendConstants[2] = 0.0f;
		color_blend_state_create_info.blendConstants[3] = 0.0f;


		// TODO: Use uniform buffers.
		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext = nullptr;
		pipeline_layout_create_info.flags = 0;
		pipeline_layout_create_info.setLayoutCount = 0;
		pipeline_layout_create_info.pSetLayouts = nullptr;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges = nullptr;


		VkResult result = vkCreatePipelineLayout(vulkan_device, &pipeline_layout_create_info, nullptr, &vulkan_pipeline_layout);
		vulkan_error_check(result);
		
		VkAttachmentDescription attachment_description = {};
		attachment_description.flags = 0;
		attachment_description.format = selected_image_format;
		attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; 
		attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE ;
		attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference attachment_reference = {};
		attachment_reference.attachment = 0;
		attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass_description = {};
		subpass_description.flags = 0;
		subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.inputAttachmentCount = 0;
		subpass_description.pInputAttachments = nullptr;
		subpass_description.colorAttachmentCount = 1;
		subpass_description.pColorAttachments = &attachment_reference;
		subpass_description.pResolveAttachments = nullptr;
		subpass_description.pDepthStencilAttachment = nullptr;
		subpass_description.preserveAttachmentCount = 0;
		subpass_description.pPreserveAttachments = nullptr;

		VkSubpassDependency subpass_dependency = {};
		subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass = 0;
		subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.srcAccessMask = 0;
		subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpass_dependency.dependencyFlags = 0;

		VkRenderPassCreateInfo render_pass_create_info = {};
		render_pass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.pNext = nullptr;
		render_pass_create_info.flags = 0;
		render_pass_create_info.attachmentCount = 1;
		render_pass_create_info.pAttachments = &attachment_description;
		render_pass_create_info.subpassCount = 1;
		render_pass_create_info.pSubpasses = &subpass_description;
		render_pass_create_info.dependencyCount = 1;
		render_pass_create_info.pDependencies = &subpass_dependency;

		result = vkCreateRenderPass(vulkan_device, &render_pass_create_info, nullptr, &render_pass);
		vulkan_error_check(result);

		VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};
		graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphics_pipeline_create_info.pNext = nullptr;
		graphics_pipeline_create_info.flags = 0;
		graphics_pipeline_create_info.stageCount = 2;
		graphics_pipeline_create_info.pStages = shader_stages.data();
		graphics_pipeline_create_info.pVertexInputState = &vertex_input_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
		graphics_pipeline_create_info.pTessellationState = nullptr;
		graphics_pipeline_create_info.pViewportState = &pipeline_viewport_viewport_state_info;
		graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
		graphics_pipeline_create_info.pMultisampleState = &multisample_create_info;
		graphics_pipeline_create_info.pDepthStencilState = nullptr;
		graphics_pipeline_create_info.pColorBlendState = &color_blend_state_create_info;
		graphics_pipeline_create_info.pDynamicState = nullptr;
		graphics_pipeline_create_info.layout = vulkan_pipeline_layout;
		graphics_pipeline_create_info.renderPass = render_pass;
		graphics_pipeline_create_info.subpass = 0;
		graphics_pipeline_create_info.basePipelineHandle = VK_NULL_HANDLE;
		graphics_pipeline_create_info.basePipelineIndex = -1;

		result = vkCreateGraphicsPipelines(vulkan_device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &vulkan_pipeline);
		vulkan_error_check(result);

	}


	void VulkanInitialisation::create_frame_buffers()
	{
		// Preallocate memory for frame buffers.
		frame_buffers.resize(number_of_images_in_swap_chain);

		for(std::size_t i=0; i<number_of_images_in_swap_chain; i++)
		{
			VkFramebufferCreateInfo frame_buffer_create_info = {};
			frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_create_info.pNext = nullptr;
			frame_buffer_create_info.flags = 0;
			frame_buffer_create_info.renderPass = render_pass;
			frame_buffer_create_info.attachmentCount = 1;
			frame_buffer_create_info.pAttachments = &image_views[i];
			frame_buffer_create_info.width = window_width;
			frame_buffer_create_info.height = window_height;
			frame_buffer_create_info.layers = 1;

			VkResult result = vkCreateFramebuffer(vulkan_device, &frame_buffer_create_info, nullptr, &frame_buffers[i]);
			vulkan_error_check(result);
		}
	}


	void VulkanInitialisation::create_image_views()
	{
		vkGetSwapchainImagesKHR(vulkan_device, vulkan_swapchain, &number_of_images_in_swap_chain, nullptr);

		cout << "Images in swap chain: " << number_of_images_in_swap_chain << endl;

		std::vector<VkImage> swapchain_images(number_of_images_in_swap_chain);

		VkResult result = vkGetSwapchainImagesKHR(vulkan_device, vulkan_swapchain, &number_of_images_in_swap_chain, swapchain_images.data());
		vulkan_error_check(result);

		// Preallocate memory for the image views.
		image_views.resize(number_of_images_in_swap_chain);
	
		VkImageViewCreateInfo image_view_create_info = {};
		image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_create_info.pNext = nullptr;
		image_view_create_info.flags = 0;
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		
		// TODO: Check if system supports this image format!
		image_view_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;

		image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		// TODO: Implement mip-mapping?
		image_view_create_info.subresourceRange.baseMipLevel = 0;
		image_view_create_info.subresourceRange.levelCount = 1;

		image_view_create_info.subresourceRange.baseArrayLayer = 0;

		// TODO: Implement awesome stereographic VR textures?
		image_view_create_info.subresourceRange.layerCount = 1;

		for(std::size_t i=0; i<number_of_images_in_swap_chain; i++)
		{
			image_view_create_info.image = swapchain_images[i];

			result = vkCreateImageView(vulkan_device, &image_view_create_info, nullptr, &image_views[i]);
			vulkan_error_check(result);
		}
	}


	VkPhysicalDevice VulkanInitialisation::decide_which_graphics_card_to_use()
	{
		// List up all graphics cards that are available on this system.
		enumerate_physical_devices();

		// Let's just use the first graphics card in the array for now.
		return graphics_cards[0];
		
		// TODO: Implement a mechanism to select the "best" graphics card automatically.
		// TODO: In case multiple graphics cards are available let the user select one.
		// TODO: Select graphic card by command line parameter "-gpu" <index>
	}


	VkFormat VulkanInitialisation::decide_which_image_format_to_use()
	{
		// A list of image formats that we can accept.
		const std::vector<VkFormat> image_format_wishlist =
		{
			// This is the default format which should be available everywhere.
			VK_FORMAT_B8G8R8A8_UNORM,

			// TODO: Add more formats to the wishlist.
			// The priority is decreasing from top to bottom
		};
		
		// We will enumerate all available image formats and compare it with our wishlist.

		uint32_t number_of_supported_formats = 0;
		
		// First check how many formats are supported.
		vkGetPhysicalDeviceSurfaceFormatsKHR(selected_graphics_card, vulkan_surface, &number_of_supported_formats, nullptr);

		// Query information about all the supported surface formats.
		std::vector<VkSurfaceFormatKHR> surface_formats(number_of_supported_formats);
		vkGetPhysicalDeviceSurfaceFormatsKHR(selected_graphics_card, vulkan_surface, &number_of_supported_formats, surface_formats.data());

		for(std::size_t i=0; i<number_of_supported_formats; i++)
		{
			for(std::size_t j=0; i<image_format_wishlist.size(); j++)
			{
				// Is one of our selected formats supported?
				if(image_format_wishlist[j] == surface_formats[i].format)
				{
					return surface_formats[i].format;
				}
			}
		}

		// This is the default format which should be available on every system.
		return VK_FORMAT_B8G8R8A8_UNORM;
	}


};
};
