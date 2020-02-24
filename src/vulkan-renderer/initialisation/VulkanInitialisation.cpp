#include "VulkanInitialisation.hpp"

// Vulkan Memory Allocator (VMA) library.
#define VMA_IMPLEMENTATION
#include "../../vma/vk_mem_alloc.h"


#include <glm/glm.hpp>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>


namespace inexor {
namespace vulkan_renderer {


	VulkanInitialisation::VulkanInitialisation()
	{
	}


	VulkanInitialisation::~VulkanInitialisation()
	{
	}


	VkResult VulkanInitialisation::create_vulkan_instance(const std::string& application_name, const std::string& engine_name, const uint32_t application_version, const uint32_t engine_version, bool enable_validation_instance_layers, bool enable_renderdoc_instance_layer)
	{
		assert(application_name.length()>0);
		assert(engine_name.length()>0);

		// Get the major, minor and patch version of the application.
		uint32_t app_major = VK_VERSION_MAJOR(application_version);
		uint32_t app_minor = VK_VERSION_MINOR(application_version);
		uint32_t app_patch = VK_VERSION_PATCH(application_version);

		// Get the major, minor and patch version of the engine.
		uint32_t engine_major = VK_VERSION_MAJOR(engine_version);
		uint32_t engine_minor = VK_VERSION_MINOR(engine_version);
		uint32_t engine_patch = VK_VERSION_PATCH(engine_version);

		spdlog::info("Initialising Vulkan instance.");
		spdlog::info("Application name: {}", application_name.c_str());
		spdlog::info("Application version: {}.{}.{}", app_major, app_minor, app_patch);
		spdlog::info("Engine name: {}", engine_name.c_str());
		spdlog::info("Engine version: {}.{}.{}", engine_major, engine_minor, engine_patch);

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


		// A vector of strings which represent the enabled instance extensions.
		std::vector<const char*> enabled_instance_extensions;

		// The extensions that we would like to enable.
		std::vector<const char*> instance_extension_wishlist =
		{
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
			// TODO: Add more instance extensions here.
		};


		// Query which extensions are needed for GLFW.
		uint32_t number_of_GLFW_extensions = 0;

		auto glfw_extensions = glfwGetRequiredInstanceExtensions(&number_of_GLFW_extensions);

		spdlog::debug("Required GLFW instance extensions:");
		
		for(std::size_t i=0; i<number_of_GLFW_extensions; i++)
		{
			spdlog::debug(glfw_extensions[i]);
			
			// Add instance extensions required by GLFW to our wishlist.
			instance_extension_wishlist.push_back(glfw_extensions[i]);
		}

		for(const auto& instance_extension : instance_extension_wishlist)
		{
			if(VulkanAvailabilityChecks::is_instance_extension_available(instance_extension))
			{
				spdlog::debug("Adding {} to instance extension wishlist.", instance_extension);
				enabled_instance_extensions.push_back(instance_extension);
			}
			else
			{
				std::string error_message = "Error: Required instance extension " + std::string(instance_extension) + " not available!";
				display_warning_message(error_message);
			}
		}


		// A vector of strings which represent the enabled instance layers.
		std::vector<const char*> enabled_instance_layers;

		// The layers that we would like to enable.
		std::vector<const char*> instance_layers_wishlist =
		{
			// RenderDoc instance layer can be specified using -renderdoc command line argument.
			// Add instance layers if neccesary..
		};

		/// RenderDoc is a modern graphics debugger written by Baldur Karlsson.
		/// It allows many useful debugging functions!
		/// https://renderdoc.org/
		/// https://github.com/baldurk/renderdoc
		if(enable_renderdoc_instance_layer)
		{
			const char renderdoc_layer_name[] = "VK_LAYER_RENDERDOC_Capture";
			
			spdlog::debug("Adding {} to instance extension wishlist.", renderdoc_layer_name);
			instance_layers_wishlist.push_back(renderdoc_layer_name);
		}


		// If validation is requested, we need to add the validation layer as instance extension!
		// For more information on Vulkan validation layers see:
		// https://vulkan.lunarg.com/doc/view/1.0.39.0/windows/layers.html
		if(enable_validation_instance_layers)
		{
			const char validation_layer_name[] = "VK_LAYER_KHRONOS_validation";
			
			spdlog::debug("Adding {} to instance extension wishlist.", validation_layer_name);
			instance_layers_wishlist.push_back(validation_layer_name);
		}

		// We now have to check which instance layers of our wishlist are really supported on the current system!
		// Loop through the wishlist and check for availabiliy.
		for(auto current_layer : instance_layers_wishlist)
		{
			if(VulkanAvailabilityChecks::is_instance_layer_available(current_layer))
			{
				spdlog::debug("Instance layer {} is supported.", current_layer);
				
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

		instance_create_info.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext                   = nullptr;
		instance_create_info.flags                   = 0;
		instance_create_info.pApplicationInfo        = &app_info;
		instance_create_info.ppEnabledExtensionNames = enabled_instance_extensions.data();
		instance_create_info.enabledExtensionCount   = static_cast<uint32_t>(enabled_instance_extensions.size());
		instance_create_info.ppEnabledLayerNames     = enabled_instance_layers.data();
		instance_create_info.enabledLayerCount       = static_cast<uint32_t>(enabled_instance_layers.size());

		// Create a new Vulkan instance.
		return vkCreateInstance(&instance_create_info, nullptr, &instance);
	}


	VkResult VulkanInitialisation::create_window_surface(const VkInstance& instance, GLFWwindow* window, VkSurfaceKHR& surface)
	{
		assert(window);
		assert(surface);
		assert(instance);

		// Create a window surface using GLFW library.
		return glfwCreateWindowSurface(instance, window, nullptr, &surface);
	}


	VkResult VulkanInitialisation::initialise_queues()
	{
		assert(present_queue_family_index.has_value());
		assert(graphics_queue_family_index.has_value());
		assert(data_transfer_queue_family_index.has_value());
		
		spdlog::debug("Initialising GPU queues.");

		spdlog::info("Graphics queue family index: {}.", graphics_queue_family_index.value());
		spdlog::info("Presentation queue family index: {}.", present_queue_family_index.value());
		spdlog::info("Data transfer queue family index: {}.", data_transfer_queue_family_index.value());

		// Setup the queues for presentation and graphics.
		// Since we only have one queue per queue family, we acquire index 0.
		vkGetDeviceQueue(device, present_queue_family_index.value(), 0, &present_queue);
		vkGetDeviceQueue(device, graphics_queue_family_index.value(), 0, &graphics_queue);

		// The use of data transfer queues can be forbidden by using -no_separate_data_queue.
		if(use_distinct_data_transfer_queue && data_transfer_queue_family_index.has_value())
		{
			// Use a separate queue for data transfer to GPU.
			vkGetDeviceQueue(device, data_transfer_queue_family_index.value(), 0, &data_transfer_queue);
		}

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_device_queues(bool use_distinct_data_transfer_queue_if_available)
	{
		spdlog::debug("Creating Vulkan device queues.");
		
		if(use_distinct_data_transfer_queue_if_available)
		{
			spdlog::debug("The application will try to use a distinct data transfer queue if it is available.");
		}
		else
		{
			spdlog::warn("The application is forced not to use a distinct data transfer queue!");
		}

		// This is neccesary since device queues might be recreated as swapchain becomes invalid.
		device_queues.clear();

		// Check if there is one queue family which can be used for both graphics and presentation.
		std::optional<uint32_t> queue_family_index_for_both_graphics_and_presentation = find_queue_family_for_both_graphics_and_presentation(selected_graphics_card, surface);
		
		// TODO: Implement command line argument for separate queues!
		if(queue_family_index_for_both_graphics_and_presentation.has_value())
		{
			spdlog::debug("One queue for both graphics and presentation will be used.");
			
			graphics_queue_family_index = queue_family_index_for_both_graphics_and_presentation.value();
			
			present_queue_family_index = graphics_queue_family_index;

			use_one_queue_family_for_graphics_and_presentation = true;

			// In this case, there is one queue family which can be used for both graphics and presentation.
			VkDeviceQueueCreateInfo device_queue_create_info = {};

			// For now, we only need one queue family.
			uint32_t number_of_combined_queues_to_use = 1;

			device_queue_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_create_info.pNext            = nullptr;
			device_queue_create_info.flags            = 0;
			device_queue_create_info.queueFamilyIndex = queue_family_index_for_both_graphics_and_presentation.value();
			device_queue_create_info.queueCount       = number_of_combined_queues_to_use;
			device_queue_create_info.pQueuePriorities = &global_queue_priority;

			device_queues.push_back(device_queue_create_info);
		}
		else
		{
			spdlog::debug("No queue found which supports both graphics and presentation.");
			spdlog::debug("The application will try to use 2 separate queues.");

			// We have to use 2 different queue families.
			// One for graphics and another one for presentation.
			
			// Check which queue family index can be used for graphics.
			graphics_queue_family_index = find_graphics_queue_family(selected_graphics_card);
			
			if(!graphics_queue_family_index.has_value())
			{
				std::string error_message = "Error: Could not find suitable queue family indices for graphics!";
				display_error_message(error_message);
				return VK_ERROR_INITIALIZATION_FAILED;
			}

			// Check which queue family index can be used for presentation.
			present_queue_family_index = find_presentation_queue_family(selected_graphics_card, surface);

			if(!present_queue_family_index.has_value())
			{
				std::string error_message = "Error: Could not find suitable queue family indices for presentation!";
				display_error_message(error_message);
				return VK_ERROR_INITIALIZATION_FAILED;
			}
			
			spdlog::debug("Graphics queue family index: {}.", graphics_queue_family_index.value());
			spdlog::debug("Presentation queue family index: {}.", present_queue_family_index.value());

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


		// Add another device queue just for data transfer.
		data_transfer_queue_family_index = find_distinct_data_transfer_queue_family(selected_graphics_card);

		if(data_transfer_queue_family_index.has_value() && use_distinct_data_transfer_queue_if_available)
		{
			spdlog::debug("A separate queue will be used for data transfer.");
			spdlog::debug("Data transfer queue family index: {}.", data_transfer_queue_family_index.value());

			// We have the opportunity to use a separated queue for data transfer!
			use_distinct_data_transfer_queue = true;

			VkDeviceQueueCreateInfo device_queue_for_data_transfer_create_info = {};

			// For now, we only need one queue family.
			uint32_t number_of_queues_to_use = 1;

			device_queue_for_data_transfer_create_info.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			device_queue_for_data_transfer_create_info.pNext            = nullptr;
			device_queue_for_data_transfer_create_info.flags            = 0;
			device_queue_for_data_transfer_create_info.queueFamilyIndex = data_transfer_queue_family_index.value();
			device_queue_for_data_transfer_create_info.queueCount       = number_of_queues_to_use;
			device_queue_for_data_transfer_create_info.pQueuePriorities = &global_queue_priority;
			
			device_queues.push_back(device_queue_for_data_transfer_create_info);
		}
		else
		{
			// We don't have the opportunity to use a separated queue for data transfer!
			// Do not create a new queue, use the graphics queue instead.
			use_distinct_data_transfer_queue = false;
		}

		return VK_SUCCESS;
	}

	
	VkResult VulkanInitialisation::create_physical_device(const VkPhysicalDevice& graphics_card, bool enable_debug_markers)
	{
		assert(device);
		assert(graphics_card);
		assert(device_queues.size()>0);
		
		spdlog::debug("Creating physical device.");
		
		// Currently, we don't need any special features at all.
		// Fill this with required features if neccesary.
		VkPhysicalDeviceFeatures used_features = {};

		// Our wishlist of device extensions that we would like to enable.
		std::vector<const char*> device_extensions_wishlist =
		{
			// Since we actually want a window to draw on, we need this swapchain extension.
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
			
			// Add more device extensions here if neccesary.
		};

		if(enable_debug_markers)
		{
			// Debug markers are only present if RenderDoc is enabled.
			device_extensions_wishlist.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		}


		// The actual list of enabled device extensions.
		std::vector<const char*> enabled_device_extensions;

		for(auto device_extension_name : device_extensions_wishlist)
		{
			if(VulkanAvailabilityChecks::is_device_extension_available(graphics_card, device_extension_name))
			{
				spdlog::debug("Device extension {} is supported!", device_extension_name);

				// This device layer is supported!
				// Add it to the list of enabled device layers.
				enabled_device_extensions.push_back(device_extension_name);
			}
			else
			{
				// This device layer is not supported!
				std::string error_message = "Error: Device extension " + std::string(device_extension_name) + " not supported!";
				display_error_message(error_message);
			}
		}

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

		return vkCreateDevice(graphics_card, &device_create_info, nullptr, &device);
	}


	VkResult VulkanInitialisation::initialise_debug_marker_manager(const bool enable_debug_markers)
	{
		spdlog::debug("Initialising Vulkan debug marker manager.");

		// Create an instance of VulkanDebugMarkerManager.
		debug_marker_manager = std::make_shared<VulkanDebugMarkerManager>(device, selected_graphics_card, enable_debug_markers);
		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_command_pool()
	{
		assert(device);
		assert(command_pool);
		assert(graphics_queue_family_index.has_value());
		assert(debug_marker_manager);
		
		spdlog::debug("Creating command pool for rendering.");

		VkCommandPoolCreateInfo command_pool_create_info = {};

		command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext            = nullptr;
		command_pool_create_info.flags            = 0;
		command_pool_create_info.queueFamilyIndex = graphics_queue_family_index.value();

		return vkCreateCommandPool(device, &command_pool_create_info, nullptr, &command_pool);
	}

	
	VkResult VulkanInitialisation::create_command_buffers()
	{
		assert(device);
		assert(debug_marker_manager);

		spdlog::debug("Creating command buffers.");
		spdlog::debug("Number of images in swapchain: {}.", number_of_images_in_swapchain);

		command_buffers.clear();

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


	VkResult VulkanInitialisation::create_vma_allocator()
	{
		assert(device);
		assert(vma_allocator);
		assert(selected_graphics_card);
		assert(debug_marker_manager);

		spdlog::debug("Initialising Vulkan memory allocator.");

		VmaAllocatorCreateInfo allocatorInfo = {};

		allocatorInfo.physicalDevice = selected_graphics_card;
		allocatorInfo.device = device;

		return vmaCreateAllocator(&allocatorInfo, &vma_allocator);
	}
	

	VkResult VulkanInitialisation::create_vertex_buffers()
	{
		assert(debug_marker_manager);
		
		spdlog::debug("Creating vertex buffers.");
		
		const std::vector<InexorVertex> vertices =
		{
			{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
		};

		const std::vector<uint32_t> indices =
		{
			0, 1, 2, 2, 3, 0
		};


		std::vector<InexorVertex> vertices2;

		std::size_t number_of_vertices = std::rand() % 90 + 9;

		vertices2.resize(number_of_vertices);

		// Create <number_of_vertices> vertices.
		for(std::size_t i=0; i<number_of_vertices; i++)
		{
			vertices2[i].color.r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			vertices2[i].color.g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			vertices2[i].color.b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

			vertices2[i].pos.x = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			vertices2[i].pos.y = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			//vertices2[i].pos.z = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		}

		VkResult result = create_vertex_buffer_with_index_buffer(vertices, indices, example_vertex_buffer);
		
		//VkResult result = create_vertex_buffer(vertices2, example_vertex_buffer);
		
		return result;
	}


	VkResult VulkanInitialisation::record_command_buffers()
	{
		assert(debug_marker_manager);
		
		spdlog::debug("Recording command buffers.");

		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			spdlog::debug("Recording command buffer #{}.", i);

			debug_marker_manager->bind_region(command_buffers[i], "Beginning of rendering", INEXOR_DEBUG_MARKER_GREEN);

			VkCommandBufferBeginInfo command_buffer_begin_info = {};

			command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			command_buffer_begin_info.pNext            = nullptr;
			command_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			command_buffer_begin_info.pInheritanceInfo = nullptr;
			
			// Begin recording of the command buffer.
			VkResult result = vkBeginCommandBuffer(command_buffers[i], &command_buffer_begin_info);
			if(VK_SUCCESS != result) return result;

			// Change color if you want another clear color.
			// Format: rgba (red, green, blue, alpha).
			// TODO: Setup clear color by configuration.
			VkClearValue clear_value;
			clear_value.color = {0.0f, 0.0f, 0.0f, 1.0f};

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

			VkDeviceSize offsets[] = {0};

			vkCmdBindVertexBuffers(command_buffers[i], 0, 1, &example_vertex_buffer.vertex_buffer.buffer, offsets);

			
			if(example_vertex_buffer.index_buffer_available)
			{	
				spdlog::debug("Recording indexed drawing of (name?).");

				debug_marker_manager->bind_region(command_buffers[i], "Render vertices using vertex buffer + index buffer", INEXOR_DEBUG_MARKER_GREEN);
				
				// Use the index buffer as well!
				vkCmdBindIndexBuffer(command_buffers[i], example_vertex_buffer.index_buffer.buffer, 0, VK_INDEX_TYPE_UINT32);
				
				vkCmdBindDescriptorSets(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[i], 0, nullptr);

				// Draw using index buffer + vertex buffer.
				vkCmdDrawIndexed(command_buffers[i], example_vertex_buffer.number_of_indices, 1, 0, 0, 0);
				
				debug_marker_manager->end_region(command_buffers[i]);
			}
			else
			{
				spdlog::debug("Recording drawing of (name?). (No index buffer!)");

				debug_marker_manager->bind_region(command_buffers[i], "Render vertices using vertex buffer ONLY", INEXOR_DEBUG_MARKER_GREEN);

				vkCmdBindDescriptorSets(command_buffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_sets[i], 0, nullptr);

				// Draw using vertex buffer only. No index buffer specified.
				vkCmdDraw(command_buffers[i], example_vertex_buffer.number_of_vertices, 1, 0, 0);
				
				debug_marker_manager->end_region(command_buffers[i]);
			}

			vkCmdEndRenderPass(command_buffers[i]);

			// End recording of the command buffer.
			result = vkEndCommandBuffer(command_buffers[i]);
			if(VK_SUCCESS != result) return result;
			
			debug_marker_manager->end_region(command_buffers[i]);
		}

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_synchronisation_objects()
	{
		assert(number_of_images_in_swapchain>0);
		assert(debug_marker_manager);

		spdlog::debug("Creating synchronisation objects (semaphores and fences).");
		spdlog::debug("Number of images in swapchain: {}", number_of_images_in_swapchain);
		
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
		assert(device);
		assert(surface);
		assert(selected_graphics_card);
		assert(debug_marker_manager);

		spdlog::debug("Creating swapchain.");

		// TODO: Check if system supports this image sharing mode!
		
		// Decide which surface color format is used.
		// The standard format VK_FORMAT_B8G8R8A8_UNORM should be available on every system.
		std::optional<VkSurfaceFormatKHR> selected_surface_format = VulkanSettingsDecisionMaker::which_surface_color_format_in_swapchain_to_use(selected_graphics_card, surface);
		
		if(selected_surface_format.has_value())
		{
			selected_color_space  = selected_surface_format.value().colorSpace;
			selected_image_format = selected_surface_format.value().format;
		}
		else
		{
			std::string error_message = "Error: Could not find a acceptable surface format!";
			display_error_message(error_message);
			exit(-1);
		}

		VulkanSettingsDecisionMaker::which_width_and_height_of_swapchain_extent(selected_graphics_card, surface, window_width, window_height, selected_swapchain_image_extent);

		std::optional<VkPresentModeKHR> selected_present_mode = VulkanSettingsDecisionMaker::which_presentation_mode_to_use(selected_graphics_card, surface);

		if(!selected_present_mode.has_value())
		{
			std::string error_message = "Error: Could not select a presentation mode for the presentation engine. This is strange, since VK_PRESENT_MODE_FIFO_KHR should be available on all systems!";
			display_error_message(error_message);
			exit(-1);
		}

		number_of_images_in_swapchain = VulkanSettingsDecisionMaker::how_many_images_in_swapchain_to_use(selected_graphics_card, surface);

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

		spdlog::info("Images in swap chain: {}.", number_of_images_in_swapchain);

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


	VkResult VulkanInitialisation::cleanup_swapchain()
	{
		spdlog::debug("Cleaning up swapchain.");
		
		spdlog::debug("Waiting for device to be idle.");
		
		vkDeviceWaitIdle(device);

		spdlog::debug("Device is idle.");
		spdlog::debug("Destroying frame buffer.");
		
		if(frame_buffers.size() > 0)
		{
			for(auto frame_buffer : frame_buffers)
			{
				if(VK_NULL_HANDLE != frame_buffer)
				{
					vkDestroyFramebuffer(device, frame_buffer, nullptr);
					frame_buffer = VK_NULL_HANDLE;
				}
			}

			frame_buffers.clear();
		}
		
		spdlog::debug("Destroying command buffers.");

		// We do not need to reset the command buffers explicitly, since it is covered by vkDestroyCommandPool.
		if(command_buffers.size() > 0)
		{
			// The size of the command buffer is equal to the number of image in swapchain.
			vkFreeCommandBuffers(device, command_pool, static_cast<uint32_t>(command_buffers.size()), command_buffers.data());

			// Don't forget to free the memory.
			command_buffers.clear();
		}

		spdlog::debug("Destroying pipeline.");

		if(VK_NULL_HANDLE != pipeline)
		{
			vkDestroyPipeline(device, pipeline, nullptr);
			pipeline = VK_NULL_HANDLE;
		}

		spdlog::debug("Destroying pipeline layout.");
		
		if(VK_NULL_HANDLE != pipeline_layout)
		{
			vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
			pipeline_layout = VK_NULL_HANDLE;
		}
		
		spdlog::debug("Destroying render pass.");

		if(VK_NULL_HANDLE != render_pass)
		{
			vkDestroyRenderPass(device, render_pass, nullptr);
			render_pass = VK_NULL_HANDLE;
		}

		spdlog::debug("Destroying image views.");
		
		if(swapchain_image_views.size() > 0)
		{
			for(auto image_view : swapchain_image_views)
			{
				if(VK_NULL_HANDLE != image_view)
				{
					vkDestroyImageView(device, image_view, nullptr);
					image_view = VK_NULL_HANDLE;
				}
			}

			swapchain_image_views.clear();
		}
		
		swapchain_images.clear();

		spdlog::debug("Destroying swapchain.");

		if(VK_NULL_HANDLE != swapchain)
		{
			vkDestroySwapchainKHR(device, swapchain, nullptr);
			swapchain = VK_NULL_HANDLE;
		}
		
		spdlog::debug("Destroying uniform buffers.");
		
		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			vkDestroyBuffer(device, uniform_buffers[i].buffer, nullptr);
			vmaFreeMemory(vma_allocator, uniform_buffers[i].allocation);
		}

		uniform_buffers.clear();

		spdlog::debug("Destroying descriptor pool.");

		vkDestroyDescriptorPool(device, descriptor_pool, nullptr);

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::recreate_swapchain()
	{
		assert(device);

		int current_window_width = 0;
		int current_window_height = 0;

		// If window is minimized, wait until it is visible again.
		while(current_window_width == 0 || current_window_height == 0)
		{
			glfwGetFramebufferSize(window, &current_window_width, &current_window_height);
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(device);
		
		spdlog::debug("Recreating the swapchain.");

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
		
		result = create_uniform_buffers();
		if(VK_SUCCESS != result) return result;

		result = create_descriptor_pool();
		if(VK_SUCCESS != result) return result;
		
		result = create_descriptor_sets();
		if(VK_SUCCESS != result) return result;
		
		result = create_command_buffers();
		if(VK_SUCCESS != result) return result;

		result = record_command_buffers();
		if(VK_SUCCESS != result) return result;

		return VK_SUCCESS;
	}

	
	VkResult VulkanInitialisation::create_descriptor_set_layout()
	{
		assert(device);
		assert(debug_marker_manager);
		
		spdlog::debug("Creating descriptor set layout.");

		VkDescriptorSetLayoutBinding uboLayoutBinding = {};

		uboLayoutBinding.binding            = 0;
		uboLayoutBinding.descriptorType     = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount    = 1;
		uboLayoutBinding.stageFlags         = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};

		layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings    = &uboLayoutBinding;

		VkResult result = vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptor_set_layout);
		vulkan_error_check(result);

		return VK_SUCCESS;
	}

	
	VkResult VulkanInitialisation::create_descriptor_pool()
	{
		assert(device);
		assert(debug_marker_manager);
		
		spdlog::debug("Creating descriptor pool.");

		VkDescriptorPoolSize pool_size = {};

		pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		pool_size.descriptorCount = number_of_images_in_swapchain;

		VkDescriptorPoolCreateInfo pool_info = {};

		pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = 1;
		pool_info.pPoolSizes    = &pool_size;
		pool_info.maxSets       = number_of_images_in_swapchain;

		VkResult result = vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);
		vulkan_error_check(result);

		return VK_SUCCESS;
	}

	
	VkResult VulkanInitialisation::create_descriptor_sets()
	{
		assert(device);
		assert(number_of_images_in_swapchain>0);
		assert(debug_marker_manager);

		spdlog::debug("Creating descriptor set layout.");
		spdlog::debug("Number of images in swapchain: {}", number_of_images_in_swapchain);

        std::vector<VkDescriptorSetLayout> layouts(number_of_images_in_swapchain, descriptor_set_layout);
        
		VkDescriptorSetAllocateInfo alloc_info = {};

        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool = descriptor_pool;
        alloc_info.descriptorSetCount = number_of_images_in_swapchain;
        alloc_info.pSetLayouts = layouts.data();

		descriptor_sets.clear();
        descriptor_sets.resize(number_of_images_in_swapchain);

		VkResult result = vkAllocateDescriptorSets(device, &alloc_info, descriptor_sets.data());
		vulkan_error_check(result);
		

        for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			spdlog::debug("Updating descriptor set #{}", i);

            VkDescriptorBufferInfo bufferInfo = {};

            bufferInfo.buffer = uniform_buffers[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range  = sizeof(UniformBufferObject);

            VkWriteDescriptorSet descriptorWrite = {};

            descriptorWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet          = descriptor_sets[i];
            descriptorWrite.dstBinding      = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo     = &bufferInfo;

            vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
        }

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::update_uniform_buffer(std::size_t current_image)
	{
		assert(vma_allocator);
		assert(debug_marker_manager);

        float time = InexorTimeStep::get_program_start_time_step();

        UniformBufferObject ubo = {};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), window_width / (float) window_height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;

        void* data;

		vmaMapMemory(vma_allocator, uniform_buffers[current_image].allocation, &data);

		std::memcpy(uniform_buffers[current_image].allocation_info.pMappedData, &ubo, sizeof(ubo));

		vmaUnmapMemory(vma_allocator, uniform_buffers[current_image].allocation);

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_uniform_buffers()
	{
		assert(device);
		assert(debug_marker_manager);
		
		VkDeviceSize buffer_size = sizeof(UniformBufferObject);
		
		spdlog::debug("Creating uniform buffers of size {}.", buffer_size);

		uniform_buffers.clear();
		uniform_buffers.resize(number_of_images_in_swapchain, buffer_size);

		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			spdlog::debug("Creating uniform buffer {}.", i);

			// It is important to use VMA_MEMORY_USAGE_CPU_TO_GPU for uniform buffers!
			VkResult result = create_buffer(uniform_buffers[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);
			vulkan_error_check(result);
		}

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_pipeline()
	{
		assert(device);
		assert(debug_marker_manager);

		spdlog::debug("Creating graphics pipeline.");

		shader_stages.clear();
		
		// TODO: Load list of shaders from JSON or TOML file.
		// TODO: Initialise Vulkan pipeline by loading JSON or TOML profiles.

		// Loop through all shaders in Vulkan shader manager's list and add them to the setup.
		auto list_of_shaders = VulkanShaderManager::get_shaders();

		spdlog::debug("Setting up shader stages.");

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
		pipeline_rasterization_state_create_info.cullMode                = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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
		pipeline_layout_create_info.setLayoutCount         = 1;
		pipeline_layout_create_info.pSetLayouts            = &descriptor_set_layout;
		pipeline_layout_create_info.pushConstantRangeCount = 0;
		pipeline_layout_create_info.pPushConstantRanges    = nullptr;


		spdlog::debug("Setting up pipeline layout.");

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

		spdlog::debug("Setting up render pass.");

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

		spdlog::debug("Finalizing graphics pipeline.");

		result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &pipeline);
		if(VK_SUCCESS != result) return result;

		return VK_SUCCESS;
	}


	VkResult VulkanInitialisation::create_frame_buffers()
	{
		assert(device);
		assert(number_of_images_in_swapchain>0);
		assert(debug_marker_manager);
		
		spdlog::debug("Creating frame buffers.");
		spdlog::debug("Number of images in swapchain: {}.", number_of_images_in_swapchain);

		// Preallocate memory for frame buffers.
		frame_buffers.resize(number_of_images_in_swapchain);

		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			spdlog::debug("Creating frameuffer #{}.", i);

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
		assert(device);
		assert(number_of_images_in_swapchain>0);
		assert(debug_marker_manager);

		spdlog::debug("Creating image views.");
		spdlog::debug("Number of images in swapchain: {}.", number_of_images_in_swapchain);
		
		// Preallocate memory for the image views.
		swapchain_image_views.clear();
		swapchain_image_views.resize(number_of_images_in_swapchain);
	
		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			spdlog::debug("Creating image #{}.", i);
			
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

	
	VkResult VulkanInitialisation::shutdown_vulkan()
	{
		// It is important to destroy the objects in reversal of the order of creation.
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Shutting down Vulkan API.");
		
		cleanup_swapchain();

		vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);

		spdlog::debug("Destroying vertex buffers.");
		VulkanMeshBufferManager::shutdown_vertex_buffers();

		// Destroy allocator of Vulkan Memory Allocator (VMA) library.
		vmaDestroyAllocator(vma_allocator);

		spdlog::debug("Destroying semaphores.");
		VulkanSynchronisationManager::shutdown_semaphores(device);

		spdlog::debug("Destroying fences.");
		VulkanSynchronisationManager::shutdown_fences(device);

		spdlog::debug("Destroying command pool.");

		if(VK_NULL_HANDLE != command_pool)
		{
			vkDestroyCommandPool(device, command_pool, nullptr);
			command_pool = VK_NULL_HANDLE;
		}

		spdlog::debug("Destroying shader objects.");
		VulkanShaderManager::shutdown_shaders(device);
		
		spdlog::debug("Destroying surface.");
		if(VK_NULL_HANDLE != surface)
		{
			vkDestroySurfaceKHR(instance, surface, nullptr);
			surface = VK_NULL_HANDLE;
		}
		
		// Device queues are implicitly cleaned up when the device is destroyed,
		// so we don’t need to do anything in cleanup.
		spdlog::debug("Destroying Vulkan device.");
		if(VK_NULL_HANDLE != device)
		{
			vkDestroyDevice(device, nullptr);
			device = VK_NULL_HANDLE;
		}
		
		// Destroy Vulkan debug callback.
		if(debug_report_callback_initialised)
		{
			// We have to explicitly load this function.
			PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));

			if(nullptr != vkDestroyDebugReportCallbackEXT)
			{
				vkDestroyDebugReportCallbackEXT(instance, debug_report_callback, nullptr);
				debug_report_callback_initialised = false;
			}
		}
		
		spdlog::debug("Destroying Vulkan instance.");
		if(VK_NULL_HANDLE != instance)
		{
			vkDestroyInstance(instance, nullptr);
			instance = VK_NULL_HANDLE;
		}
		
		spdlog::debug("Shutdown finished.");
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		
		return VK_SUCCESS;
	}


};
};
