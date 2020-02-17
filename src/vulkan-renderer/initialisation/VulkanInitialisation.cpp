#include "VulkanInitialisation.hpp"
using namespace std;


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
		// Get the major, minor and patch version of the application.
		uint32_t app_major = VK_VERSION_MAJOR(application_version);
		uint32_t app_minor = VK_VERSION_MINOR(application_version);
		uint32_t app_patch = VK_VERSION_PATCH(application_version);

		// Get the major, minor and patch version of the engine.
		uint32_t engine_major = VK_VERSION_MAJOR(engine_version);
		uint32_t engine_minor = VK_VERSION_MINOR(engine_version);
		uint32_t engine_patch = VK_VERSION_PATCH(engine_version);

		cout << "Initialising Vulkan instance." << endl;
		cout << "Application name: "            << application_name.c_str() << endl;
		cout << "Application version: "         << app_major << "." << app_minor << "." << app_patch << endl;
		cout << "Engine name: "                 << engine_name.c_str() << endl;
		cout << "Engine version: "              << engine_major << "." << engine_minor << "." << engine_patch << endl;
		cout << endl;

		// TODO: Check which version of Vulkan is available before trying to create an instance!
		// TODO: Switch to VOLK one day? This would allow for dynamic initialisation during runtime without linking vulkan libraries.
		
		// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkApplicationInfo.html
		// "Because Vulkan 1.0 implementations may fail with VK_ERROR_INCOMPATIBLE_DRIVER,
		// applications should determine the version of Vulkan available before calling vkCreateInstance.
		// If the vkGetInstanceProcAddr returns NULL for vkEnumerateInstanceVersion, it is a Vulkan 1.0 implementation.
		// Otherwise, the application can call vkEnumerateInstanceVersion to determine the version of Vulkan."

		// Structure specifying application's Vulkan API info.
		VkApplicationInfo app_info = {};

		app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext              = nullptr;
		app_info.pApplicationName   = application_name.c_str();
		app_info.applicationVersion = application_version;
		app_info.pEngineName        = engine_name.c_str();
		app_info.engineVersion      = engine_version;
		app_info.apiVersion         = VK_API_VERSION_1_1;


		// Query which extensions are needed for GLFW.
		uint32_t number_of_GLFW_extensions = 0;
		
		auto glfw_extensions = glfwGetRequiredInstanceExtensions(&number_of_GLFW_extensions);

		cout << "Required GLFW instance extensions: " << endl;

		for(std::size_t i=0; i<number_of_GLFW_extensions; i++)
		{
			cout << glfw_extensions[i] << endl;

			if(!check_instance_extension_availability(glfw_extensions[i]))
			{
				std::string error_message = "Error: GLFW required instance extension " + std::string(glfw_extensions[i]) + " not available!";
				display_error_message(error_message);
				exit(-1);
			}
		}

		cout << endl;

		// A vector of strings which represent the enabled instance layers.
		std::vector<const char*> enabled_instance_layers;

		// The layers that we would like to enable.
		std::vector<const char*> instance_layers_wishlist = {
			//"VK_LAYER_RENDERDOC_Capture"
		};

		// If validation is requested, we need to add the validation layer as instance extension!
		// For more information on Vulkan validation layers see:
		// https://vulkan.lunarg.com/doc/view/1.0.39.0/windows/layers.html
		if(enable_validation_layers)
		{
			const char validation_layer_name[] = "VK_LAYER_KHRONOS_validation";
			instance_layers_wishlist.push_back(validation_layer_name);
		}

		// We now have to check which instance layers of our wishlist are really supported on the current system!
		// Loop through the wishlist and check for availabiliy.
		for(auto current_layer : instance_layers_wishlist)
		{
			if(check_instance_layer_availability(current_layer))
			{
				cout << "Instance layer " << current_layer << " is supported!" << endl;
				
				// This instance layer is available!
				// Add it to the list of enabled instance layers!
				enabled_instance_layers.push_back(current_layer);
			}
			else
			{
				cout << "Instance layer " << current_layer << " is NOT supported!" << endl;

				std::string error_message = "Error: instance layer " + std::string(current_layer) + " not available!";
				display_error_message(error_message);
			}
		}

		cout << endl;

		// Structure specifying parameters of a newly created instance.
		VkInstanceCreateInfo instance_create_info = {};

		instance_create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext                   = nullptr;
		instance_create_info.flags                   = 0;
		instance_create_info.pApplicationInfo        = &app_info;
		instance_create_info.enabledExtensionCount   = number_of_GLFW_extensions;
		instance_create_info.ppEnabledExtensionNames = glfw_extensions;
		instance_create_info.ppEnabledLayerNames     = enabled_instance_layers.data();
		instance_create_info.enabledLayerCount       = static_cast<uint32_t>(enabled_instance_layers.size());

		// Create a new Vulkan instance.
		return vkCreateInstance(&instance_create_info, nullptr, &instance);
	}


	VkResult VulkanInitialisation::create_window_surface(const VkInstance& instance, GLFWwindow* window, VkSurfaceKHR& surface)
	{
		// Create a window surface using GLFW library.
		return glfwCreateWindowSurface(instance, window, nullptr, &surface);
	}


	VkResult VulkanInitialisation::create_device_queues()
	{
		cout << "Creating device queues." << endl;

		// This is neccesary since device queues might be recreated as swapchain becomes invalid.
		device_queues.clear();

		// Check if there is one queue family which can be used for both graphics and presentation.
		std::optional<uint32_t> queue_family_index_for_both_graphics_and_presentation = check_existence_of_queue_family_for_both_graphics_and_presentation(selected_graphics_card, surface);
		
		if(queue_family_index_for_both_graphics_and_presentation.has_value())
		{
			graphics_queue_family_index = queue_family_index_for_both_graphics_and_presentation.value();
			present_queue_family_index = graphics_queue_family_index;
			use_one_queue_family_for_graphics_and_presentation = true;

			// In this case, there is one queue family which can be used for both graphics and presentation.
			VkDeviceQueueCreateInfo device_queue_create_info = {};

			// For now, we only need one queue family.
			uint32_t number_of_queues_to_use = 1;

			device_queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_create_info.pNext            = nullptr;
			device_queue_create_info.flags            = 0;
			device_queue_create_info.queueFamilyIndex = queue_family_index_for_both_graphics_and_presentation.value();
			device_queue_create_info.queueCount       = number_of_queues_to_use;
			device_queue_create_info.pQueuePriorities = &global_queue_priority;

			device_queues.push_back(device_queue_create_info);
		}
		else
		{
			// We have to use 2 different queue families.
			// One for graphics and another one for presentation.
			
			// Check which queue family index can be used for graphics.
			graphics_queue_family_index = decide_which_graphics_queue_family_to_use(selected_graphics_card);
			
			if(!graphics_queue_family_index.has_value())
			{
				std::string error_message = "Error: Could not find suitable queue family indices for graphics!";
				display_error_message(error_message);
				return VK_ERROR_INITIALIZATION_FAILED;
			}

			// Check which queue family index can be used for presentation.
			present_queue_family_index = decide_which_presentation_queue_family_to_use(selected_graphics_card, surface);

			if(!present_queue_family_index.has_value())
			{
				std::string error_message = "Error: Could not find suitable queue family indices for presentation!";
				display_error_message(error_message);
				return VK_ERROR_INITIALIZATION_FAILED;
			}


			// Set up one queue for graphics.
			VkDeviceQueueCreateInfo device_queue_create_info_for_graphics_queue = {};
			
			// For now, we only need one queue family.
			uint32_t number_of_graphics_queues_to_use = 1;

			device_queue_create_info_for_graphics_queue.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_create_info_for_graphics_queue.pNext            = nullptr;
			device_queue_create_info_for_graphics_queue.flags            = 0;
			device_queue_create_info_for_graphics_queue.queueFamilyIndex = graphics_queue_family_index.value();
			device_queue_create_info_for_graphics_queue.queueCount       = number_of_graphics_queues_to_use;
			device_queue_create_info_for_graphics_queue.pQueuePriorities = &global_queue_priority;

			device_queues.push_back(device_queue_create_info_for_graphics_queue);


			// Set up one queue for presentation.
			VkDeviceQueueCreateInfo device_queue_create_info_for_presentation_queue = {};
			
			// For now, we only need one queue family.
			uint32_t number_of_present_queues_to_use = 1;

			device_queue_create_info_for_presentation_queue.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_create_info_for_presentation_queue.pNext            = nullptr;
			device_queue_create_info_for_presentation_queue.flags            = 0;
			device_queue_create_info_for_presentation_queue.queueFamilyIndex = present_queue_family_index.value();
			device_queue_create_info_for_presentation_queue.queueCount       = number_of_present_queues_to_use;
			device_queue_create_info_for_presentation_queue.pQueuePriorities = &global_queue_priority;

			device_queues.push_back(device_queue_create_info_for_presentation_queue);
		}

		return VK_SUCCESS;
	}

	
	VkResult VulkanInitialisation::create_physical_device(const VkPhysicalDevice& graphics_card)
	{
		cout << "Creating a physical device." << endl;
		
		// Currently, we don't need any special features at all.
		// Fill this with required features if neccesary.
		VkPhysicalDeviceFeatures used_features = {};

		// Our wishlist of device extensions that we would like to enable.
		const std::vector<const char*> device_extensions_wishlist =
		{
			// Since we actually want a window to draw on, we need this swapchain extension.
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
			
			// Add more device extensions here if neccesary.
		};


		// The actual list of enabled device extensions.
		std::vector<const char*> enabled_device_extensions;

		for(auto device_extension_name : device_extensions_wishlist)
		{
			if(check_device_extension_availability(graphics_card, device_extension_name))
			{
				cout << "Device extension " << device_extension_name << " is supported!" << endl;

				// This device layer is supported!
				// Add it to the list of enabled device layers.
				enabled_device_extensions.push_back(device_extension_name);
			}
			else
			{
				cout << "Device extension " << device_extension_name << " is supported!" << endl;

				// This device layer is not supported!
				std::string error_message = "Error: Device extension " + std::string(device_extension_name) + " not supported!";
				display_error_message(error_message);
			}
		}

		cout << endl;

		VkDeviceCreateInfo device_create_info = {};
		
		device_create_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pNext                   = nullptr;
		device_create_info.flags                   = 0;
		device_create_info.queueCreateInfoCount    = static_cast<uint32_t>(device_queues.size());
		device_create_info.pQueueCreateInfos       = device_queues.data();
		device_create_info.enabledLayerCount       = 0;
		device_create_info.ppEnabledLayerNames     = nullptr;
		device_create_info.enabledExtensionCount   = static_cast<uint32_t>(enabled_device_extensions.size());
		device_create_info.ppEnabledExtensionNames = enabled_device_extensions.data();
		device_create_info.pEnabledFeatures        = &used_features;

		return vkCreateDevice(graphics_card, &device_create_info, NULL, &device);
	}


	VkResult VulkanInitialisation::create_command_pool()
	{
		cout << "Creating command pool." << endl;

		if(!graphics_queue_family_index.has_value())
		{
			std::string error_message = "Error: No graphics queue family index specified!";
			display_error_message(error_message);
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		VkCommandPoolCreateInfo command_pool_create_info = {};

		command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext            = nullptr;
		command_pool_create_info.flags            = 0;
		command_pool_create_info.queueFamilyIndex = graphics_queue_family_index.value();

		return vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool);
	}

	
	VkResult VulkanInitialisation::create_command_buffers()
	{
		cout << "Creating command buffers." << endl;

		command_buffers.clear();

		// TODO: Migrate to Vulkan Memory Allocator (VMA)!
		// Hopefully there will be a conan package for that some day.

		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};
		
		command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.pNext              = nullptr;
		command_buffer_allocate_info.commandPool        = command_pool;
		command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = number_of_images_in_swapchain;

		// Preallocate memory for command buffers.
		command_buffers.resize(number_of_images_in_swapchain);

		return vkAllocateCommandBuffers(device, &command_buffer_allocate_info, command_buffers.data());
	}




	/// @param type_filter parameter will be used to specify the bit field of memory types
	/// that are suitable. That means that we can find the index of a suitable memory
	/// type by simply iterating over them and checking if the corresponding bit is set to 1.
	uint32_t find_suitable_memory_type(const VkPhysicalDevice& device, const uint32_t type_filter, const VkMemoryPropertyFlags& memory_property_flags)
	{
		VkPhysicalDeviceMemoryProperties memory_properties;

		// Query information about the available types of memory
		vkGetPhysicalDeviceMemoryProperties(device, &memory_properties);

		/// Memory heaps are distinct memory resources like dedicated VRAM and swap space in RAM for when VRAM runs out.
		/// The different types of memory exist within these heaps. Right now we’ll only concern ourselves with the type
		/// of memory and not the heap it comes from, but you can imagine that this can affect performance.

		// Let's use uint32_t for iteration so we don't need to static_castt the return value i.
		for(uint32_t i=0; i<memory_properties.memoryTypeCount; i++)
		{
			// 
			if(type_filter & (1 << i))
			{
				/// We may have more than one desirable property, so we should check if the result
				/// of the bitwise AND is not just non-zero, but equal to the desired properties bit field.
				if(memory_properties.memoryTypes[i].propertyFlags & memory_property_flags)
				{
					return i;
				}
			}
		}

		cout << "Error: No matching memory type found!" << endl;

		return 0;
	}

	
	VkResult VulkanInitialisation::create_vertex_buffers()
	{
		VkBufferCreateInfo buffer_create_info = {};

		buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size        = sizeof(vertices[0])*vertices.size();
		buffer_create_info.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VkResult result = vkCreateBuffer(device, &buffer_create_info, nullptr, &vertex_buffer);
		vulkan_error_check(result);

		/// The buffer has been created, but it doesn’t actually have any memory assigned to it yet!
		/// The first step of allocating memory for the buffer is to query its memory requirements.
		VkMemoryRequirements memory_requirements;

		vkGetBufferMemoryRequirements(device, vertex_buffer, &memory_requirements);

		cout << "Vertex buffer size: " << memory_requirements.size << endl;

		/// Graphics cards can offer different types of memory to allocate from. Each type of
		/// memory varies in terms of allowed operations and performance characteristics.
		/// We need to combine the requirements of the buffer and our own application
		/// requirements to find the right type of memory to use.


		VkMemoryAllocateInfo memory_allocate_info = {};

		// TODO: Use std::optional
		//std::optional<uint32_t> memory_type_index = find_suitable_memory_type(selected_graphics_card, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		uint32_t memory_type_index = find_suitable_memory_type(selected_graphics_card, memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		/*
		if(!memory_type_index.has_value())
		{
			std:: string error_message = "Error: Could not find suitable memory index!";
			display_error_message(error_message);
			return VK_ERROR_INITIALIZATION_FAILED;
		}
		*/
		
		memory_allocate_info.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memory_allocate_info.allocationSize  = memory_requirements.size;
		memory_allocate_info.pNext           = nullptr;
		memory_allocate_info.memoryTypeIndex = memory_type_index;

		result = vkAllocateMemory(device, &memory_allocate_info, nullptr, &vertex_buffer_memory);
		vulkan_error_check(result);

		vkBindBufferMemory(device, vertex_buffer, vertex_buffer_memory, 0);

		// Get the memory address.
		void* data;
		vkMapMemory(device, vertex_buffer_memory, 0, buffer_create_info.size, 0, &data);
		
		// TODO: Refactor this process! Use a proper staging buffer!
		//std::memcpy(data, vertices.data(), static_cast<std::size_t>(buffer_create_info.size));
		memcpy(data, vertices.data(), (size_t) buffer_create_info.size);

		vkUnmapMemory(device, vertex_buffer_memory);

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::record_command_buffers()
	{
		cout << "Recording command buffers." << endl;

		VkCommandBufferBeginInfo command_buffer_begin_info = {};

		command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		command_buffer_begin_info.pNext            = nullptr;
		command_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		command_buffer_begin_info.pInheritanceInfo = nullptr;

		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			// Begin recording of the command buffer.
			VkResult result = vkBeginCommandBuffer(command_buffers[i], &command_buffer_begin_info);
			if(VK_SUCCESS != result) return result;

			// Change color if you want another clear color.
			// Format: rgba (red, green, blue, alpha).
			VkClearValue clear_value;
			clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};

			// TODO: Setup clear color by configuration.
			
			VkRenderPassBeginInfo render_pass_begin_info = {};
			
			render_pass_begin_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.pNext             = nullptr;
			render_pass_begin_info.renderPass        = render_pass;
			render_pass_begin_info.framebuffer       = frame_buffers[i];
			render_pass_begin_info.renderArea.offset = {0, 0};
			render_pass_begin_info.renderArea.extent = {window_width, window_height};
			render_pass_begin_info.clearValueCount   = 1;
			render_pass_begin_info.pClearValues      = &clear_value;

			vkCmdBeginRenderPass(command_buffers[i], &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			// TODO: Refactor this!
			VkDeviceSize offsets[] = {0};
			VkBuffer vertex_buffers[] = {vertex_buffer};
			vkCmdBindVertexBuffers(command_buffers[i], 0, 1, vertex_buffers, offsets);

			// TODO: Draw size of buffer!
			vkCmdDraw(command_buffers[i], 3, 1, 0, 0);
			vkCmdEndRenderPass(command_buffers[i]);

			// End recording of the command buffer.
			result = vkEndCommandBuffer(command_buffers[i]);
			if(VK_SUCCESS != result) return result;
		}

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_synchronisation_objects()
	{
		cout << "Creating semaphores and fences." << endl;
		
		in_flight_fences.clear();
		image_available_semaphores.clear();
		rendering_finished_semaphores.clear();

		for(std::size_t i=0; i<INEXOR_MAX_FRAMES_IN_FLIGHT; i++)
		{
			// TODO: Refactor this into something like create_multiple_semaphores()?
			// Here we create the semaphores and fences which are neccesary for synchronisation.
			// Cleanup will be handled by VulkanSynchronisationManager.
			image_available_semaphores.push_back(create_semaphore(device, "image_available_semaphores_"+std::to_string(i)).value());
			rendering_finished_semaphores.push_back(create_semaphore(device, "rendering_finished_semaphores_"+std::to_string(i)).value());
			in_flight_fences.push_back(create_fence(device, "in_flight_fences_"+std::to_string(i)).value());
		}
	
		images_in_flight.clear();
		
		// Note: Images in flight do not need to be initialised!
		images_in_flight.resize(number_of_images_in_swapchain, VK_NULL_HANDLE);

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_swapchain()
	{
		cout << "Creating swap chain." << endl;

		// TODO: Check if system supports this image sharing mode!

		// Decide which surface color format is used.
		// The standard format VK_FORMAT_B8G8R8A8_UNORM should be available on every system.
		std::optional<VkSurfaceFormatKHR> selected_surface_format = decide_which_surface_color_format_in_swapchain_to_use(selected_graphics_card, surface);
		
		if(selected_surface_format.has_value())
		{
			selected_color_space = selected_surface_format.value().colorSpace;
			selected_image_format = selected_surface_format.value().format;
		}
		else
		{
			std::string error_message = "Error: Could not find a acceptable surface format!";
			display_error_message(error_message);
			exit(-1);
		}


		decide_width_and_height_of_swapchain_extent(selected_graphics_card, surface, window_width, window_height, selected_swapchain_image_extent);

		std::optional<VkPresentModeKHR> selected_present_mode = decide_which_presentation_mode_to_use(selected_graphics_card, surface);

		if(!selected_present_mode.has_value())
		{
			std::string error_message = "Error: Could not select a presentation mode for the presentation engine. This is strange, since VK_PRESENT_MODE_FIFO_KHR should be available on all systems!";
			display_error_message(error_message);
			exit(-1);
		}

		number_of_images_in_swapchain = decide_how_many_images_in_swapchain_to_use(selected_graphics_card, surface);

		if(0 == number_of_images_in_swapchain)
		{
			std::string error_message = "Error: Invalid number of images in swapchain!";
			display_error_message(error_message);
			exit(-1);
		}

		VkSwapchainCreateInfoKHR swapchain_create_info = {};
		
		swapchain_create_info.sType                 = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.pNext                 = nullptr;
		swapchain_create_info.flags                 = 0;
		swapchain_create_info.surface               = surface;
		swapchain_create_info.minImageCount         = number_of_images_in_swapchain;
		swapchain_create_info.imageFormat           = selected_image_format;
		swapchain_create_info.imageColorSpace       = selected_color_space;
		swapchain_create_info.imageExtent           = selected_swapchain_image_extent;
		swapchain_create_info.imageArrayLayers      = 1;
		swapchain_create_info.imageUsage            = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		if(use_one_queue_family_for_graphics_and_presentation)
		{
			// In this case, we can use one queue family for both graphics and presentation.
			swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
			swapchain_create_info.queueFamilyIndexCount = 0;
			swapchain_create_info.pQueueFamilyIndices   = nullptr;
		}
		else
		{
			// In this case, we can't use the same queue family for both graphics and presentation.
			// We must use 2 separate queue families!
			const std::vector<uint32_t> queue_family_indices =
			{
				graphics_queue_family_index.value(),
				present_queue_family_index.value()
			};
			
			// It is important to note that we can't use VK_SHARING_MODE_EXCLUSIVE in this case.
			// VK_SHARING_MODE_CONCURRENT may result in lower performance access to the buffer or image than VK_SHARING_MODE_EXCLUSIVE.
			swapchain_create_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
			swapchain_create_info.pQueueFamilyIndices   = queue_family_indices.data();
			swapchain_create_info.queueFamilyIndexCount = static_cast<uint32_t>(queue_family_indices.size());
		}

		swapchain_create_info.preTransform          = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		swapchain_create_info.compositeAlpha        = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode           = selected_present_mode.value();
		swapchain_create_info.clipped               = VK_TRUE;
		swapchain_create_info.oldSwapchain          = VK_NULL_HANDLE;


		VkResult result = vkCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain);
		if(VK_SUCCESS != result) return result;

		swapchain_image_views.clear();

		result = vkGetSwapchainImagesKHR(device, swapchain, &number_of_images_in_swapchain, nullptr);
		if(VK_SUCCESS != result) return result;

		cout << "Images in swap chain: " << number_of_images_in_swapchain << endl;

		if(number_of_images_in_swapchain <= 0)
		{
			display_error_message("Error: Invalid number of images in swapchain!");
		}

		swapchain_images.clear();

		// Preallocate memory for the images in swapchain.
		swapchain_images.resize(number_of_images_in_swapchain);

		result = vkGetSwapchainImagesKHR(device, swapchain, &number_of_images_in_swapchain, swapchain_images.data());
		if(VK_SUCCESS != result) return result;

		return VK_SUCCESS;
	}


	void VulkanInitialisation::cleanup_swapchain()
	{
		cout << "Cleaning up swapchain." << endl;
		cout << "Waiting for device to be idle." << endl;
		
		vkDeviceWaitIdle(device);

		cout << "Device is idle." << endl;
		cout << "Destroying frame buffer." << endl;
		
		for(auto frame_buffer : frame_buffers)
		{
			if(VK_NULL_HANDLE != frame_buffer)
			{
				vkDestroyFramebuffer(device, frame_buffer, nullptr);
			}
		}

		frame_buffers.clear();

		cout << "Destroying command buffers." << endl;

		// We do not need to reset the command buffers explicitly, since it is covered by vkDestroyCommandPool.
		if(command_buffers.size() > 0)
		{
			// The size of the command buffer is equal to the number of image in swapchain.
			vkFreeCommandBuffers(device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());

			// Don't forget to free the memory.
			command_buffers.clear();
		}

		cout << "Destroying pipeline." << endl;

		if(VK_NULL_HANDLE != pipeline)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
		}

		cout << "Destroying pipeline layout." << endl;
		
		if(VK_NULL_HANDLE != pipeline_layout)
		{
			vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
		}
		
		cout << "Destroying render pass." << endl;
		
		if(VK_NULL_HANDLE != render_pass)
		{
			vkDestroyRenderPass(device, render_pass, nullptr);
		}

		cout << "Destroying image views." << endl;
		
		for(auto image_view : swapchain_image_views)
		{
			if(VK_NULL_HANDLE != image_view)
			{
				vkDestroyImageView(device, image_view, nullptr);
			}
		}

		swapchain_image_views.clear();
		swapchain_images.clear();

		cout << "Destroying swapchain." << endl;

		if(VK_NULL_HANDLE != swapchain)
		{
			vkDestroySwapchainKHR(device, swapchain, nullptr);
		}
	}


	VkResult VulkanInitialisation::recreate_swapchain()
	{
		int current_window_width = 0;
		int current_window_height = 0;

		// If window is minimized, wait until it is visible again.
		while(current_window_width == 0 || current_window_height == 0)
		{
			glfwGetFramebufferSize(window, &current_window_width, &current_window_height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);
		
		cout << "Recreating the swapchain." << endl;

		// Cleanup only neccesary parts.
		cleanup_swapchain();

		VkResult result = create_swapchain();
		if(VK_SUCCESS != result) return result;

		result = create_image_views();
		if(VK_SUCCESS != result) return result;
		
		result = create_pipeline();
		if(VK_SUCCESS != result) return result;
		
		result = create_frame_buffers();
		if(VK_SUCCESS != result) return result;
		
		result = create_command_buffers();
		if(VK_SUCCESS != result) return result;

		// TODO: create vertex buffers?
		
		result = record_command_buffers();
		if(VK_SUCCESS != result) return result;

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_pipeline()
	{
		cout << "Creating graphics pipeline." << endl;

		shader_stages.clear();
		
		// TODO: Load list of shaders from JSON or TOML file.
		// TODO: Initialise Vulkan pipeline by loading JSON or TOML profiles.

		// Loop through all shaders in Vulkan shader manager's list and add them to the setup.
		auto list_of_shaders = VulkanShaderManager::get_shaders();

		for(const auto& current_shader : list_of_shaders)
		{
			VkPipelineShaderStageCreateInfo shader_stage_create_info = {};

			shader_stage_create_info.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shader_stage_create_info.pNext               = nullptr;
			shader_stage_create_info.flags               = 0;
			shader_stage_create_info.stage               = current_shader.get_shader_type();
			shader_stage_create_info.module              = current_shader.get_shader_module();
			shader_stage_create_info.pName               = "main"; // TODO: Refactor this to current_shader.get_shader_entry_point().c_str()!
			shader_stage_create_info.pSpecializationInfo = nullptr;
			
			shader_stages.push_back(shader_stage_create_info);
		}

		
		VkPipelineVertexInputStateCreateInfo vertex_input_create_info = {};
		
		auto vertex_binding_description    = InexorVertex::get_vertex_binding_description();
		auto attribute_binding_description = InexorVertex::get_attribute_binding_description();

		vertex_input_create_info.sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_create_info.pNext                           = nullptr;
		vertex_input_create_info.flags                           = 0;
		vertex_input_create_info.vertexBindingDescriptionCount   = 1;
		vertex_input_create_info.pVertexBindingDescriptions      = &vertex_binding_description;
		vertex_input_create_info.vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_binding_description.size());
		vertex_input_create_info.pVertexAttributeDescriptions    = attribute_binding_description.data();


		VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};

		input_assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_create_info.pNext                  = nullptr;
		input_assembly_create_info.flags                  = 0;
		input_assembly_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

		// TODO: Setup viewport by JSON or TOML file.
		
		VkViewport view_port = {};

		view_port.x        = 0.0f;
		view_port.y        = 0.0f;
		view_port.width    = static_cast<float>(window_width);
		view_port.height   = static_cast<float>(window_height);
		view_port.minDepth = 0.0f;
		view_port.maxDepth = 1.0f;
		
		// TODO: Setup scissor by JSON or TOML file.

		VkRect2D scissor = {};

		scissor.offset = {0, 0};
		scissor.extent = {window_width, window_height};


		VkPipelineViewportStateCreateInfo pipeline_viewport_viewport_state_info = {};

		pipeline_viewport_viewport_state_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		pipeline_viewport_viewport_state_info.pNext         = nullptr;
		pipeline_viewport_viewport_state_info.flags         = 0;
		pipeline_viewport_viewport_state_info.viewportCount = 1;
		pipeline_viewport_viewport_state_info.pViewports    = &view_port;
		pipeline_viewport_viewport_state_info.scissorCount  = 1;
		pipeline_viewport_viewport_state_info.pScissors     = &scissor;


		VkPipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info = {};

		pipeline_rasterization_state_create_info.sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		pipeline_rasterization_state_create_info.pNext                   = nullptr;
		pipeline_rasterization_state_create_info.flags                   = 0;
		pipeline_rasterization_state_create_info.depthClampEnable        = VK_FALSE;
		pipeline_rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
		pipeline_rasterization_state_create_info.polygonMode             = VK_POLYGON_MODE_FILL;
		pipeline_rasterization_state_create_info.cullMode                = VK_CULL_MODE_BACK_BIT;
		pipeline_rasterization_state_create_info.frontFace               = VK_FRONT_FACE_CLOCKWISE;
		pipeline_rasterization_state_create_info.depthBiasEnable         = VK_FALSE;
		pipeline_rasterization_state_create_info.depthBiasConstantFactor = 0.0f;
		pipeline_rasterization_state_create_info.depthBiasClamp          = 0.0f;
		pipeline_rasterization_state_create_info.depthBiasSlopeFactor    = 0.0f;
		pipeline_rasterization_state_create_info.lineWidth               = 1.0f;


		VkPipelineMultisampleStateCreateInfo multisample_create_info = {};

		multisample_create_info.sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_create_info.pNext                 = nullptr;
		multisample_create_info.flags                 = 0;
		multisample_create_info.rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT;
		multisample_create_info.sampleShadingEnable   = VK_FALSE;
		multisample_create_info.minSampleShading      = 1.0f;
		multisample_create_info.pSampleMask           = nullptr;
		multisample_create_info.alphaToCoverageEnable = VK_FALSE;
		multisample_create_info.alphaToOneEnable      = VK_FALSE;

		
		VkPipelineColorBlendAttachmentState color_blend_attachment = {};

		color_blend_attachment.blendEnable         = VK_TRUE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		color_blend_attachment.colorBlendOp        = VK_BLEND_OP_ADD;
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		color_blend_attachment.alphaBlendOp        = VK_BLEND_OP_ADD;
		color_blend_attachment.colorWriteMask      = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;


		VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};

		color_blend_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state_create_info.pNext             = nullptr;
		color_blend_state_create_info.flags             = 0;
		color_blend_state_create_info.logicOpEnable     = VK_FALSE;
		color_blend_state_create_info.logicOp           = VK_LOGIC_OP_NO_OP;
		color_blend_state_create_info.attachmentCount   = 1;
		color_blend_state_create_info.pAttachments      = &color_blend_attachment;
		color_blend_state_create_info.blendConstants[0] = 0.0f;
		color_blend_state_create_info.blendConstants[1] = 0.0f;
		color_blend_state_create_info.blendConstants[2] = 0.0f;
		color_blend_state_create_info.blendConstants[3] = 0.0f;


		VkPipelineLayoutCreateInfo pipeline_layout_create_info = {};

		pipeline_layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext                  = nullptr;
		pipeline_layout_create_info.flags                  = 0;
		pipeline_layout_create_info.setLayoutCount         = 0;
		pipeline_layout_create_info.pSetLayouts            = nullptr;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges    = nullptr;


		VkResult result = vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &pipeline_layout);
		if(VK_SUCCESS != result) return result;
		

		// TODO: Generalize renderpass description.

		VkAttachmentDescription attachment_description = {};

		attachment_description.flags          = 0;
		attachment_description.format         = selected_image_format;
		attachment_description.samples        = VK_SAMPLE_COUNT_1_BIT;
		attachment_description.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR; 
		attachment_description.storeOp        = VK_ATTACHMENT_STORE_OP_STORE ;
		attachment_description.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment_description.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
		attachment_description.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;


		VkAttachmentReference attachment_reference = {};

		attachment_reference.attachment = 0;
		attachment_reference.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


		VkSubpassDescription subpass_description = {};

		subpass_description.flags                   = 0;
		subpass_description.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass_description.inputAttachmentCount    = 0;
		subpass_description.pInputAttachments       = nullptr;
		subpass_description.colorAttachmentCount    = 1;
		subpass_description.pColorAttachments       = &attachment_reference;
		subpass_description.pResolveAttachments     = nullptr;
		subpass_description.pDepthStencilAttachment = nullptr;
		subpass_description.preserveAttachmentCount = 0;
		subpass_description.pPreserveAttachments    = nullptr;

		
		VkSubpassDependency subpass_dependency = {};

		subpass_dependency.srcSubpass      = VK_SUBPASS_EXTERNAL;
		subpass_dependency.dstSubpass      = 0;
		subpass_dependency.srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		subpass_dependency.srcAccessMask   = 0;
		subpass_dependency.dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT|VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		subpass_dependency.dependencyFlags = 0;


		VkRenderPassCreateInfo render_pass_create_info = {};
		
		render_pass_create_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		render_pass_create_info.pNext           = nullptr;
		render_pass_create_info.flags           = 0;
		render_pass_create_info.attachmentCount = 1;
		render_pass_create_info.pAttachments    = &attachment_description;
		render_pass_create_info.subpassCount    = 1;
		render_pass_create_info.pSubpasses      = &subpass_description;
		render_pass_create_info.dependencyCount = 1;
		render_pass_create_info.pDependencies   = &subpass_dependency;

		result = vkCreateRenderPass(device, &render_pass_create_info, nullptr, &render_pass);
		if(VK_SUCCESS != result) return result;


		VkGraphicsPipelineCreateInfo graphics_pipeline_create_info = {};

		graphics_pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphics_pipeline_create_info.pNext               = nullptr;
		graphics_pipeline_create_info.flags               = 0;
		graphics_pipeline_create_info.stageCount          = static_cast<uint32_t>(shader_stages.size());
		graphics_pipeline_create_info.pStages             = shader_stages.data();
		graphics_pipeline_create_info.pVertexInputState   = &vertex_input_create_info;
		graphics_pipeline_create_info.pInputAssemblyState = &input_assembly_create_info;
		graphics_pipeline_create_info.pTessellationState  = nullptr;
		graphics_pipeline_create_info.pViewportState      = &pipeline_viewport_viewport_state_info;
		graphics_pipeline_create_info.pRasterizationState = &pipeline_rasterization_state_create_info;
		graphics_pipeline_create_info.pMultisampleState   = &multisample_create_info;
		graphics_pipeline_create_info.pDepthStencilState  = nullptr;
		graphics_pipeline_create_info.pColorBlendState    = &color_blend_state_create_info;
		graphics_pipeline_create_info.pDynamicState       = nullptr;
		graphics_pipeline_create_info.layout              = pipeline_layout;
		graphics_pipeline_create_info.renderPass          = render_pass;
		graphics_pipeline_create_info.subpass             = 0;
		graphics_pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;
		graphics_pipeline_create_info.basePipelineIndex   = -1;

		result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);
		if(VK_SUCCESS != result) return result;

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_frame_buffers()
	{
		cout << "Creating frame buffers." << endl;

		// Preallocate memory for frame buffers.
		frame_buffers.resize(number_of_images_in_swapchain);

		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			VkFramebufferCreateInfo frame_buffer_create_info = {};

			frame_buffer_create_info.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			frame_buffer_create_info.pNext           = nullptr;
			frame_buffer_create_info.flags           = 0;
			frame_buffer_create_info.renderPass      = render_pass;
			frame_buffer_create_info.attachmentCount = 1;
			frame_buffer_create_info.pAttachments    = &swapchain_image_views[i];
			frame_buffer_create_info.width           = window_width;
			frame_buffer_create_info.height          = window_height;
			frame_buffer_create_info.layers          = 1;

			VkResult result = vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &frame_buffers[i]);
			if(VK_SUCCESS != result) return result;
		}

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_image_views()
	{
		cout << "Creating image views." << endl;

		// Preallocate memory for the image views.
		swapchain_image_views.clear();
		swapchain_image_views.resize(number_of_images_in_swapchain);
	
		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			VkImageViewCreateInfo image_view_create_info = {};
		
			image_view_create_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			image_view_create_info.pNext                           = nullptr;
			image_view_create_info.flags                           = 0;
			image_view_create_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
			image_view_create_info.format                          = selected_image_format;
			image_view_create_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
			image_view_create_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
			image_view_create_info.subresourceRange.baseMipLevel   = 0;
			image_view_create_info.subresourceRange.levelCount     = 1;
			image_view_create_info.subresourceRange.baseArrayLayer = 0;
			image_view_create_info.subresourceRange.layerCount     = 1;
			image_view_create_info.image = swapchain_images[i];

			VkResult result = vkCreateImageView(device, &image_view_create_info, nullptr, &swapchain_image_views[i]);
			if(VK_SUCCESS != result) return result;
		}

		return VK_SUCCESS;
	}

	
	void VulkanInitialisation::shutdown_vulkan()
	{
		// It is important to destroy the objects in reversal of the order of creation.
		
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Shutting down Vulkan API." << endl;
		
		// This functions calls vkDeviceWaitIdle for us.
		cleanup_swapchain();

		// TODO: Refactor memory management!
		vkDestroyBuffer(device, vertex_buffer, nullptr);
		vkFreeMemory(device, vertex_buffer_memory, nullptr);

		cout << "Destroying semaphores." << endl;
		VulkanSynchronisationManager::shutdown_semaphores(device);

		cout << "Destroying fences." << endl;
		VulkanSynchronisationManager::shutdown_fences(device);

		cout << "Destroying command pool." << endl;
		if(VK_NULL_HANDLE != command_pool)
		{
			vkDestroyCommandPool(device, command_pool, nullptr);
		}

		cout << "Destroying shader objects." << endl;
		VulkanShaderManager::shutdown_shaders(device);
		
		cout << "Destroying surface." << endl;
		if(VK_NULL_HANDLE != surface)
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
		}
		
		// Device queues are implicitly cleaned up when the device is destroyed, so we don’t need to do anything in cleanup.
		cout << "Destroying device." << endl;
		if(VK_NULL_HANDLE != device)
		{
			vkDestroyDevice(device, nullptr);
		}
		
		cout << "Destroying instance." << endl;
		if(VK_NULL_HANDLE != instance)
		{
			vkDestroyInstance(instance, nullptr);
		}
		
		cout << "Shutdown finished." << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
	}


};
};
