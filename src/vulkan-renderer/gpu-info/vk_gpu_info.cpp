#include "vk_gpu_info.hpp"


namespace inexor {
namespace vulkan_renderer {


	VulkanGraphicsCardInfoViewer::VulkanGraphicsCardInfoViewer()
	{
	}

	
	VulkanGraphicsCardInfoViewer::~VulkanGraphicsCardInfoViewer()
	{
	}

	
	void VulkanGraphicsCardInfoViewer::print_driver_vulkan_version()
	{
		// The version of the available Vulkan API is encoded as a 32 bit integer.
		// https://vulkan.lunarg.com/doc/view/latest/windows/vkspec.html#extendingvulkan-coreversions-versionnumbers
		uint32_t api_version = 0;
		
		// The Vulkan version number comprises three parts indicating the major, minor and patch version of the Vulkan API Specification.
		// The major version indicates a significant change in the API, which will encompass a wholly new version of the specification.
		// The minor version indicates the incorporation of new functionality into the core specification.
		// The patch version indicates bug fixes, clarifications, and language improvements have been incorporated into the specification.
		VkResult result = vkEnumerateInstanceVersion(&api_version);
		vulkan_error_check(result);

		// Extract major, minor and patch version of the Vulkan API available.
		uint16_t api_major_version = VK_VERSION_MAJOR(api_version);
		uint16_t api_minor_version = VK_VERSION_MINOR(api_version);
		uint16_t api_version_patch = VK_VERSION_PATCH(api_version);

		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Supported Vulkan API version: {}.{}.{}", api_major_version, api_minor_version, api_version_patch);
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
	}


	void VulkanGraphicsCardInfoViewer::print_physical_device_queue_families(const VkPhysicalDevice& graphics_card)
	{
		assert(graphics_card);
		
		// The number of available queue families.
		uint32_t number_of_queue_families = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_queue_families, nullptr);

		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Number of queue families: {}", number_of_queue_families);
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		
		if(number_of_queue_families <= 0)
		{
			display_error_message("Error: Could not find any queue families!");
		}
		else
		{
			// Preallocate memory for the family queues.
			std::vector<VkQueueFamilyProperties> queue_family_properties(number_of_queue_families);

			// Get information about physical device queue family properties.
			vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_queue_families, queue_family_properties.data());

			// Loop through all available queue families.
			for(std::size_t i=0; i< number_of_queue_families; i++)
			{
				spdlog::debug("Queue family: {}", i);
				spdlog::debug("------------------------------------------------------------------------------------------------------------");
				spdlog::debug("Queue Count: {}", queue_family_properties[i].queueCount);
				spdlog::debug("Timestamp Valid Bits: {}", queue_family_properties[i].timestampValidBits);

				if(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)       spdlog::debug("VK_QUEUE_GRAPHICS_BIT");
				if(queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)        spdlog::debug("VK_QUEUE_COMPUTE_BIT");
				if(queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)       spdlog::debug("VK_QUEUE_TRANSFER_BIT");
				if(queue_family_properties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) spdlog::debug("VK_QUEUE_SPARSE_BINDING_BIT");
				if(queue_family_properties[i].queueFlags & VK_QUEUE_PROTECTED_BIT)      spdlog::debug("VK_QUEUE_PROTECTED_BIT");

				uint32_t width  = queue_family_properties[i].minImageTransferGranularity.width;
				uint32_t height = queue_family_properties[i].minImageTransferGranularity.width;
				uint32_t depth  = queue_family_properties[i].minImageTransferGranularity.depth;
			
				spdlog::debug("Min Image Timestamp Granularity: {}, {}, {}", width, height, depth);
			}
		}
	}


	void VulkanGraphicsCardInfoViewer::print_instance_layers()
	{
		uint32_t number_of_instance_layers = 0;

		// First check how many instance layers are available.
		VkResult result = vkEnumerateInstanceLayerProperties(&number_of_instance_layers, nullptr);
		vulkan_error_check(result);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Number of instance layers: {}", number_of_instance_layers);
		spdlog::debug("------------------------------------------------------------------------------------------------------------");

		if(number_of_instance_layers <= 0)
		{
			display_error_message("Error: Could not find any instance layers!");
		}
		else
		{
			// Preallocate memory for instance layers.
			std::vector<VkLayerProperties> instance_layers(number_of_instance_layers);
		
			// Get information about the available instance layers.
			result = vkEnumerateInstanceLayerProperties(&number_of_instance_layers, instance_layers.data());
			vulkan_error_check(result);

			// Loop through all available instance layers and print information about them.
			for(auto instance_layer : instance_layers)
			{
				uint32_t spec_major = VK_VERSION_MAJOR(instance_layer.specVersion);
				uint32_t spec_minor = VK_VERSION_MINOR(instance_layer.specVersion);
				uint32_t spec_patch = VK_VERSION_PATCH(instance_layer.specVersion);

				spdlog::debug("Name: {}", instance_layer.layerName);
				spdlog::debug("Spec Version: {}", spec_major, spec_minor, spec_patch);
				spdlog::debug("Impl Version: {}", instance_layer.implementationVersion);
				spdlog::debug("Description: {}", instance_layer.description);
			}
		}
	}


	void VulkanGraphicsCardInfoViewer::print_instance_extensions()
	{
		uint32_t number_of_instance_extensions = 0;

		// First check how many instance extensions are available.
		VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &number_of_instance_extensions, nullptr);
		vulkan_error_check(result);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Number of instance extensions: {} ",number_of_instance_extensions);
		spdlog::debug("------------------------------------------------------------------------------------------------------------");

		if(number_of_instance_extensions <= 0)
		{
			display_error_message("Error: Could not find any instance extensions!");
		}
		else
		{
			// Preallocate memory for instance extensions.
			std::vector<VkExtensionProperties> extensions(number_of_instance_extensions);

			// Get information about the available instance extensions.
			result = vkEnumerateInstanceExtensionProperties(nullptr, &number_of_instance_extensions, extensions.data());
			vulkan_error_check(result);

			// Loop through all available instance extensions and print information about them.
			for(auto extension : extensions)
			{
				uint32_t spec_major = VK_VERSION_MAJOR(extension.specVersion);
				uint32_t spec_minor = VK_VERSION_MINOR(extension.specVersion);
				uint32_t spec_patch = VK_VERSION_PATCH(extension.specVersion);

				spdlog::debug("Spec version: {}.{}.{}\t Name: {}", spec_major, spec_minor, spec_patch, extension.extensionName);
			}
		}
	}


	void VulkanGraphicsCardInfoViewer::print_device_layers(const VkPhysicalDevice& graphics_card)
	{
		assert(graphics_card);

		uint32_t number_of_device_layers = 0;

		// First check how many device layers are available.
		VkResult result = vkEnumerateDeviceLayerProperties(graphics_card, &number_of_device_layers, nullptr);
		vulkan_error_check(result);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Number of device layers: {}", number_of_device_layers);
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		
		if(number_of_device_layers <= 0)
		{
			display_error_message("Error: Could not find any device layers!");
		}
		else
		{
			// Preallocate memory for device layers.
			std::vector<VkLayerProperties> device_layers(number_of_device_layers);
		
			// Get information about the available device layers.
			result = vkEnumerateDeviceLayerProperties(graphics_card, &number_of_device_layers, device_layers.data());
			vulkan_error_check(result);

			// Loop through all available device layers and print information about them.
			for(auto device_layer : device_layers)
			{
				uint32_t spec_major = VK_VERSION_MAJOR(device_layer.specVersion);
				uint32_t spec_minor = VK_VERSION_MINOR(device_layer.specVersion);
				uint32_t spec_patch = VK_VERSION_PATCH(device_layer.specVersion);

				spdlog::debug("Name: {}", device_layer.layerName);
				spdlog::debug("Spec Version: {}", spec_major, spec_minor, spec_patch);
				spdlog::debug("Impl Version: {}", device_layer.implementationVersion);
				spdlog::debug("Description: {}", device_layer.description);
			}
		}
	}


	void VulkanGraphicsCardInfoViewer::print_device_extensions(const VkPhysicalDevice& graphics_card)
	{
		assert(graphics_card);

		uint32_t number_of_device_extensions = 0;
		
		// First check how many device extensions are available.
		VkResult result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_device_extensions, nullptr);
		vulkan_error_check(result);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Number of device extensions: ", number_of_device_extensions);
		spdlog::debug("------------------------------------------------------------------------------------------------------------");

		if(number_of_device_extensions <= 0)
		{
			display_error_message("Error: Could not find any device extensions!");
		}
		else
		{
			// Preallocate memory for device extensions.
			std::vector<VkExtensionProperties> device_extensions(number_of_device_extensions);
		
			// Get information about the available device extensions.
			result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_device_extensions, device_extensions.data());
			vulkan_error_check(result);

			// Loop through all available device extensions and print information about them.
			for(auto device_extension : device_extensions)
			{
				uint32_t spec_major = VK_VERSION_MAJOR(device_extension.specVersion);
				uint32_t spec_minor = VK_VERSION_MINOR(device_extension.specVersion);
				uint32_t spec_patch = VK_VERSION_PATCH(device_extension.specVersion);

				spdlog::debug("Spec version: {}.{}.{}\t Name: {}", spec_major, spec_minor, spec_patch, device_extension.extensionName);
			}
		}
	}


	void VulkanGraphicsCardInfoViewer::print_surface_capabilities(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
	{
		assert(graphics_card);
		assert(vulkan_surface);

		spdlog::debug("Printing surface capabilities.");
		
		VkSurfaceCapabilitiesKHR surface_capabilities;

		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, vulkan_surface, &surface_capabilities);
		vulkan_error_check(result);

		spdlog::debug("minImageCount: {}",           surface_capabilities.minImageCount);
		spdlog::debug("maxImageCount: {}",           surface_capabilities.maxImageCount);
		spdlog::debug("currentExtent.width: {}",     surface_capabilities.currentExtent.width);
		spdlog::debug("currentExtent.height: {}",    surface_capabilities.currentExtent.height);
		spdlog::debug("minImageExtent.width: {}",    surface_capabilities.minImageExtent.width);
		spdlog::debug("minImageExtent.height: {}",   surface_capabilities.minImageExtent.height);
		spdlog::debug("maxImageExtent.width: {}",    surface_capabilities.maxImageExtent.width);
		spdlog::debug("maxImageExtent.height: {}",   surface_capabilities.maxImageExtent.height);
		spdlog::debug("maxImageArrayLayers: {}",     surface_capabilities.maxImageArrayLayers);
		spdlog::debug("supportedTransforms: {}",     surface_capabilities.supportedTransforms);
		spdlog::debug("currentTransform: {}",        surface_capabilities.currentTransform);
		spdlog::debug("supportedCompositeAlpha: {}", surface_capabilities.supportedCompositeAlpha);
		spdlog::debug("supportedUsageFlags: {}",     surface_capabilities.supportedUsageFlags);
	}


	void VulkanGraphicsCardInfoViewer::print_supported_surface_formats(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
	{
		assert(graphics_card);
		assert(vulkan_surface);

		uint32_t number_of_supported_formats = 0;
		
		// First check how many formats are supported.
		VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, nullptr);
		vulkan_error_check(result);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Supported surface formats: {}", number_of_supported_formats);
		spdlog::debug("------------------------------------------------------------------------------------------------------------");

		if(number_of_supported_formats <= 0)
		{
			display_error_message("Error: Could not find any supported formats!");
		}
		else
		{
			std::vector<VkSurfaceFormatKHR> surface_formats(number_of_supported_formats);

			result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, surface_formats.data());
			vulkan_error_check(result);

			for(std::size_t i=0; i<number_of_supported_formats; i++)
			{
				std::unordered_map<int, std::string>::const_iterator surface_format_lookup = surface_format_names.find(surface_formats[i].format);
			
				if(surface_format_lookup == surface_format_names.end())
				{
					// Name not found, print number instead.
					spdlog::debug("Surface format: {}", surface_formats[i].format);
				}
				else
				{
					// We found a text description for this in surface_format_names.
					spdlog::debug("surface format: {}", surface_format_names.at(surface_formats[i].format));
				}
			}
		}
	}


	void VulkanGraphicsCardInfoViewer::print_presentation_modes(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
	{
		assert(graphics_card);
		assert(vulkan_surface);

		uint32_t number_of_present_modes = 0;
		
		// First check how many presentation modes are available.
		VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, vulkan_surface, &number_of_present_modes, nullptr);
		vulkan_error_check(result);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Available present modes: ", number_of_present_modes);
		spdlog::debug("------------------------------------------------------------------------------------------------------------");


		if(number_of_present_modes <= 0)
		{
			display_error_message("Error: Could not find any presentation modes!");
		}
		else
		{
			// Preallocate memory for the presentation modes.
			std::vector<VkPresentModeKHR> present_modes(number_of_present_modes);

			result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, vulkan_surface, &number_of_present_modes, present_modes.data());
			vulkan_error_check(result);

			const std::unordered_map<int, std::string> present_mode_names = {
				{0,          "VK_PRESENT_MODE_IMMEDIATE_KHR"},
				{1,          "VK_PRESENT_MODE_MAILBOX_KHR"},
				{2,          "VK_PRESENT_MODE_FIFO_KHR"},
				{3,          "VK_PRESENT_MODE_FIFO_RELAXED_KHR"},
				{1000111000, "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR"},
				{1000111001, "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR"}
			};

			for(std::size_t i=0; i<number_of_present_modes; i++)
			{
				std::unordered_map<int, std::string>::const_iterator present_mode_lookup = present_mode_names.find(present_modes[i]);
			
				if(present_mode_lookup == present_mode_names.end())
				{
					// Name not found, print number instead.
					spdlog::debug(present_modes[i]);
				}
				else
				{
					// We found a text description for this in surface_format_names.
					spdlog::debug(present_mode_names.at(present_modes[i]));
				}
			}
		}
	}


	void VulkanGraphicsCardInfoViewer::print_graphics_card_info(const VkPhysicalDevice& graphics_card)
	{
		assert(graphics_card);

		// The properties of the graphics card.
		VkPhysicalDeviceProperties graphics_card_properties;

		// Get the information about that graphics card.
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);

		// Print the name of the graphics card.
		spdlog::debug("Graphics card: graphics_card_properties.deviceName");

		// Get the major, minor and patch version of the Vulkan API version.
		uint32_t VulkanAPIversion     = graphics_card_properties.apiVersion;
		uint32_t vulkan_version_major = VK_VERSION_MAJOR(VulkanAPIversion);
		uint32_t vulkan_version_minor = VK_VERSION_MINOR(VulkanAPIversion);
		uint32_t vulkan_version_patch = VK_VERSION_MAJOR(VulkanAPIversion);

		// The Vulkan version which is supported by the graphics card.
		spdlog::debug("Vulkan API supported version: {}.{}.{}", vulkan_version_major, vulkan_version_minor, vulkan_version_patch);

		// Get the major, minor and patch version of the driver version.
		uint32_t driver_version_major = VK_VERSION_MAJOR(graphics_card_properties.driverVersion);
		uint32_t driver_version_minor = VK_VERSION_MINOR(graphics_card_properties.driverVersion);
		uint32_t driver_version_patch = VK_VERSION_PATCH(graphics_card_properties.driverVersion);

		// The driver version.
		// Always keep your graphics drivers up to date!
		// Note: The driver version format is NOT standardised!
		spdlog::debug("Vulkan API supported version: {}.{}.{}", vulkan_version_major, vulkan_version_minor, vulkan_version_patch);
		spdlog::debug("Vendor ID: {}", graphics_card_properties.vendorID);
		spdlog::debug("Device ID: {}", graphics_card_properties.deviceID);

		// Graphics card types.
		// TODO: Is there any other way to get the graphics card type name by id?
		const std::string graphics_card_types[] = {
			"VK_PHYSICAL_DEVICE_TYPE_OTHER",
			"VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU",
			"VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU",
			"VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU",
			"VK_PHYSICAL_DEVICE_TYPE_CPU",
		};

		// Check if array index is in bounds.
		if(graphics_card_properties.deviceType <= 4)
		{
			spdlog::debug("Device type: {}", graphics_card_types[graphics_card_properties.deviceType]);
		}
	}


	void VulkanGraphicsCardInfoViewer::print_graphics_card_memory_properties(const VkPhysicalDevice& graphics_card)
	{
		assert(graphics_card);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Graphics card's memory properties:");
		spdlog::debug("------------------------------------------------------------------------------------------------------------");

		// Check memory properties of this graphics card.
		VkPhysicalDeviceMemoryProperties graphics_card_memory_properties;

		vkGetPhysicalDeviceMemoryProperties(graphics_card, &graphics_card_memory_properties);

		spdlog::debug("Number of memory types: {}", graphics_card_memory_properties.memoryTypeCount);
		spdlog::debug("Number of heap types: {}", graphics_card_memory_properties.memoryHeapCount);


		// Loop through all memory types and list their features.
		for(std::size_t i=0; i<graphics_card_memory_properties.memoryTypeCount; i++)
		{
			spdlog::debug("[{}] Heap index: ", i, graphics_card_memory_properties.memoryTypes[i].heapIndex);

			auto &propertyFlag = graphics_card_memory_properties.memoryTypes[i].propertyFlags;

			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)        spdlog::debug("VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT");
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)        spdlog::debug("VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT");
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)       spdlog::debug("VK_MEMORY_PROPERTY_HOST_COHERENT_BIT");
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)         spdlog::debug("VK_MEMORY_PROPERTY_HOST_CACHED_BIT");
			if(propertyFlag & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)    spdlog::debug("VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT");
			if(propertyFlag & VK_MEMORY_PROPERTY_PROTECTED_BIT)           spdlog::debug("VK_MEMORY_PROPERTY_PROTECTED_BIT");
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) spdlog::debug("VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD");
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) spdlog::debug("VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD");
		}

		// Loop through all memory heaps.
		for(std::size_t i=0; i<graphics_card_memory_properties.memoryHeapCount; i++)
		{
			spdlog::debug("Heap [{}], memory size: ", i, graphics_card_memory_properties.memoryHeaps[i].size/(1000*1000));

			auto &propertyFlag = graphics_card_memory_properties.memoryHeaps[i].flags;

			if(propertyFlag & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)   spdlog::debug("VK_MEMORY_HEAP_DEVICE_LOCAL_BIT ");
			if(propertyFlag & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) spdlog::debug("VK_MEMORY_HEAP_MULTI_INSTANCE_BIT");
		}
	}


	void VulkanGraphicsCardInfoViewer::print_graphics_card_features(const VkPhysicalDevice& graphics_card)
	{
		assert(graphics_card);

		VkPhysicalDeviceFeatures graphics_card_features;

		// Check which features are supported by this graphics card.
		vkGetPhysicalDeviceFeatures(graphics_card, &graphics_card_features);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Graphics card's features:");
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		
		// This little helper macro prints the regarded information about graphics card features to the console.
		#define PRINT_GRAPHICS_CARD_FEATURE(x) spdlog::debug("{}: {}", #x, (graphics_card_features.x) ? "yes" : "no")

		PRINT_GRAPHICS_CARD_FEATURE(robustBufferAccess);
		PRINT_GRAPHICS_CARD_FEATURE(fullDrawIndexUint32);
		PRINT_GRAPHICS_CARD_FEATURE(imageCubeArray);
		PRINT_GRAPHICS_CARD_FEATURE(independentBlend);
		PRINT_GRAPHICS_CARD_FEATURE(geometryShader);
		PRINT_GRAPHICS_CARD_FEATURE(tessellationShader);
		PRINT_GRAPHICS_CARD_FEATURE(sampleRateShading);
		PRINT_GRAPHICS_CARD_FEATURE(dualSrcBlend);
		PRINT_GRAPHICS_CARD_FEATURE(logicOp);
		PRINT_GRAPHICS_CARD_FEATURE(multiDrawIndirect);
		PRINT_GRAPHICS_CARD_FEATURE(drawIndirectFirstInstance);
		PRINT_GRAPHICS_CARD_FEATURE(depthClamp);
		PRINT_GRAPHICS_CARD_FEATURE(depthBiasClamp);
		PRINT_GRAPHICS_CARD_FEATURE(fillModeNonSolid);
		PRINT_GRAPHICS_CARD_FEATURE(depthBounds);
		PRINT_GRAPHICS_CARD_FEATURE(wideLines);
		PRINT_GRAPHICS_CARD_FEATURE(largePoints);
		PRINT_GRAPHICS_CARD_FEATURE(alphaToOne);
		PRINT_GRAPHICS_CARD_FEATURE(multiViewport);
		PRINT_GRAPHICS_CARD_FEATURE(samplerAnisotropy);
		PRINT_GRAPHICS_CARD_FEATURE(textureCompressionETC2);
		PRINT_GRAPHICS_CARD_FEATURE(textureCompressionASTC_LDR);
		PRINT_GRAPHICS_CARD_FEATURE(textureCompressionBC);
		PRINT_GRAPHICS_CARD_FEATURE(occlusionQueryPrecise);
		PRINT_GRAPHICS_CARD_FEATURE(pipelineStatisticsQuery);
		PRINT_GRAPHICS_CARD_FEATURE(vertexPipelineStoresAndAtomics);
		PRINT_GRAPHICS_CARD_FEATURE(fragmentStoresAndAtomics);
		PRINT_GRAPHICS_CARD_FEATURE(shaderTessellationAndGeometryPointSize);
		PRINT_GRAPHICS_CARD_FEATURE(shaderImageGatherExtended);
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageExtendedFormats);
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageMultisample);
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageReadWithoutFormat);
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageWriteWithoutFormat);
		PRINT_GRAPHICS_CARD_FEATURE(shaderUniformBufferArrayDynamicIndexing);
		PRINT_GRAPHICS_CARD_FEATURE(shaderSampledImageArrayDynamicIndexing);
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageBufferArrayDynamicIndexing);
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageArrayDynamicIndexing);
		PRINT_GRAPHICS_CARD_FEATURE(shaderClipDistance);
		PRINT_GRAPHICS_CARD_FEATURE(shaderCullDistance);
		PRINT_GRAPHICS_CARD_FEATURE(shaderFloat64);
		PRINT_GRAPHICS_CARD_FEATURE(shaderInt64);
		PRINT_GRAPHICS_CARD_FEATURE(shaderInt16);
		PRINT_GRAPHICS_CARD_FEATURE(shaderResourceResidency);
		PRINT_GRAPHICS_CARD_FEATURE(shaderResourceMinLod);
		PRINT_GRAPHICS_CARD_FEATURE(sparseBinding);
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidencyBuffer);
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidencyImage2D);
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidencyImage3D);
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidency2Samples);
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidency4Samples);
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidency8Samples);
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidency16Samples);
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidencyAliased);
		PRINT_GRAPHICS_CARD_FEATURE(variableMultisampleRate);
		PRINT_GRAPHICS_CARD_FEATURE(inheritedQueries);
	}


	void VulkanGraphicsCardInfoViewer::print_graphics_cards_sparse_properties(const VkPhysicalDevice& graphics_card)
	{
		assert(graphics_card);

		VkPhysicalDeviceProperties graphics_card_properties;

		// Get the information about that graphics card.
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Graphics card's sparse properties:");
		spdlog::debug("------------------------------------------------------------------------------------------------------------");

		#define PRINT_SPARSE_PROPERTIES(x) spdlog::debug("{}: {}", #x, (graphics_card_properties.sparseProperties.x))

		PRINT_SPARSE_PROPERTIES(residencyStandard2DBlockShape);
		PRINT_SPARSE_PROPERTIES(residencyStandard2DMultisampleBlockShape);
		PRINT_SPARSE_PROPERTIES(residencyStandard3DBlockShape);
		PRINT_SPARSE_PROPERTIES(residencyAlignedMipSize);
		PRINT_SPARSE_PROPERTIES(residencyNonResidentStrict);
	}

	
	void VulkanGraphicsCardInfoViewer::print_graphics_card_limits(const VkPhysicalDevice& graphics_card)
	{
		assert(graphics_card);

		VkPhysicalDeviceProperties graphics_card_properties;

		// Get the information about that graphics card.
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);
		
		spdlog::debug("------------------------------------------------------------------------------------------------------------");
		spdlog::debug("Graphics card's limits:");
		spdlog::debug("------------------------------------------------------------------------------------------------------------");

		// This little helper macro prints the regarded information about graphics card limits to the console.
		#define PRINT_GRAPHICS_CARD_LIMITS(x) spdlog::debug("{}: {}", #x, (graphics_card_properties.limits.x))

		PRINT_GRAPHICS_CARD_LIMITS(maxImageDimension1D);
		PRINT_GRAPHICS_CARD_LIMITS(maxImageDimension2D);
		PRINT_GRAPHICS_CARD_LIMITS(maxImageDimension3D);
		PRINT_GRAPHICS_CARD_LIMITS(maxImageDimensionCube);
		PRINT_GRAPHICS_CARD_LIMITS(maxImageArrayLayers);
		PRINT_GRAPHICS_CARD_LIMITS(maxTexelBufferElements);
		PRINT_GRAPHICS_CARD_LIMITS(maxUniformBufferRange);
		PRINT_GRAPHICS_CARD_LIMITS(maxStorageBufferRange);
		PRINT_GRAPHICS_CARD_LIMITS(maxPushConstantsSize);
		PRINT_GRAPHICS_CARD_LIMITS(maxMemoryAllocationCount);
		PRINT_GRAPHICS_CARD_LIMITS(maxSamplerAllocationCount);
		PRINT_GRAPHICS_CARD_LIMITS(bufferImageGranularity);
		PRINT_GRAPHICS_CARD_LIMITS(sparseAddressSpaceSize);
		PRINT_GRAPHICS_CARD_LIMITS(maxBoundDescriptorSets);
		PRINT_GRAPHICS_CARD_LIMITS(maxPerStageDescriptorSamplers);
		PRINT_GRAPHICS_CARD_LIMITS(maxPerStageDescriptorUniformBuffers);
		PRINT_GRAPHICS_CARD_LIMITS(maxPerStageDescriptorStorageBuffers);
		PRINT_GRAPHICS_CARD_LIMITS(maxPerStageDescriptorSampledImages);
		PRINT_GRAPHICS_CARD_LIMITS(maxPerStageDescriptorStorageImages);
		PRINT_GRAPHICS_CARD_LIMITS(maxPerStageDescriptorInputAttachments);
		PRINT_GRAPHICS_CARD_LIMITS(maxPerStageResources);
		PRINT_GRAPHICS_CARD_LIMITS(maxDescriptorSetSamplers);
		PRINT_GRAPHICS_CARD_LIMITS(maxDescriptorSetUniformBuffers);
		PRINT_GRAPHICS_CARD_LIMITS(maxDescriptorSetUniformBuffersDynamic);
		PRINT_GRAPHICS_CARD_LIMITS(maxDescriptorSetStorageBuffers);
		PRINT_GRAPHICS_CARD_LIMITS(maxDescriptorSetStorageBuffersDynamic);
		PRINT_GRAPHICS_CARD_LIMITS(maxDescriptorSetSampledImages);
		PRINT_GRAPHICS_CARD_LIMITS(maxDescriptorSetStorageImages);
		PRINT_GRAPHICS_CARD_LIMITS(maxDescriptorSetInputAttachments);
		PRINT_GRAPHICS_CARD_LIMITS(maxVertexInputAttributes);
		PRINT_GRAPHICS_CARD_LIMITS(maxVertexInputBindings);
		PRINT_GRAPHICS_CARD_LIMITS(maxVertexInputAttributeOffset);
		PRINT_GRAPHICS_CARD_LIMITS(maxVertexInputBindingStride);
		PRINT_GRAPHICS_CARD_LIMITS(maxVertexOutputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxTessellationGenerationLevel);
		PRINT_GRAPHICS_CARD_LIMITS(maxTessellationPatchSize);
		PRINT_GRAPHICS_CARD_LIMITS(maxTessellationControlPerVertexInputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxTessellationControlPerVertexOutputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxTessellationControlPerPatchOutputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxTessellationControlTotalOutputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxTessellationEvaluationInputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxTessellationEvaluationOutputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxGeometryShaderInvocations);
		PRINT_GRAPHICS_CARD_LIMITS(maxGeometryInputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxGeometryOutputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxGeometryOutputVertices);
		PRINT_GRAPHICS_CARD_LIMITS(maxGeometryTotalOutputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxFragmentInputComponents);
		PRINT_GRAPHICS_CARD_LIMITS(maxFragmentOutputAttachments);
		PRINT_GRAPHICS_CARD_LIMITS(maxFragmentDualSrcAttachments);
		PRINT_GRAPHICS_CARD_LIMITS(maxFragmentCombinedOutputResources);
		PRINT_GRAPHICS_CARD_LIMITS(maxComputeSharedMemorySize);
		PRINT_GRAPHICS_CARD_LIMITS(maxComputeWorkGroupCount[0]);
		PRINT_GRAPHICS_CARD_LIMITS(maxComputeWorkGroupCount[1]);
		PRINT_GRAPHICS_CARD_LIMITS(maxComputeWorkGroupCount[2]);
		PRINT_GRAPHICS_CARD_LIMITS(maxComputeWorkGroupInvocations);
		PRINT_GRAPHICS_CARD_LIMITS(maxComputeWorkGroupSize[0]);
		PRINT_GRAPHICS_CARD_LIMITS(maxComputeWorkGroupSize[1]);
		PRINT_GRAPHICS_CARD_LIMITS(maxComputeWorkGroupSize[2]);
		PRINT_GRAPHICS_CARD_LIMITS(subPixelPrecisionBits);
		PRINT_GRAPHICS_CARD_LIMITS(subTexelPrecisionBits);
		PRINT_GRAPHICS_CARD_LIMITS(mipmapPrecisionBits);
		PRINT_GRAPHICS_CARD_LIMITS(maxDrawIndexedIndexValue);
		PRINT_GRAPHICS_CARD_LIMITS(maxDrawIndirectCount);
		PRINT_GRAPHICS_CARD_LIMITS(maxSamplerLodBias);
		PRINT_GRAPHICS_CARD_LIMITS(maxSamplerAnisotropy);
		PRINT_GRAPHICS_CARD_LIMITS(maxViewports);
		PRINT_GRAPHICS_CARD_LIMITS(maxViewportDimensions[0]);
		PRINT_GRAPHICS_CARD_LIMITS(maxViewportDimensions[1]);
		PRINT_GRAPHICS_CARD_LIMITS(viewportBoundsRange[0]);
		PRINT_GRAPHICS_CARD_LIMITS(viewportBoundsRange[1]);
		PRINT_GRAPHICS_CARD_LIMITS(viewportSubPixelBits);
		PRINT_GRAPHICS_CARD_LIMITS(minMemoryMapAlignment);
		PRINT_GRAPHICS_CARD_LIMITS(minTexelBufferOffsetAlignment);
		PRINT_GRAPHICS_CARD_LIMITS(minUniformBufferOffsetAlignment);
		PRINT_GRAPHICS_CARD_LIMITS(minStorageBufferOffsetAlignment);
		PRINT_GRAPHICS_CARD_LIMITS(minTexelOffset);
		PRINT_GRAPHICS_CARD_LIMITS(maxTexelOffset);
		PRINT_GRAPHICS_CARD_LIMITS(minTexelGatherOffset);
		PRINT_GRAPHICS_CARD_LIMITS(maxTexelGatherOffset);
		PRINT_GRAPHICS_CARD_LIMITS(minInterpolationOffset);
		PRINT_GRAPHICS_CARD_LIMITS(maxInterpolationOffset);
		PRINT_GRAPHICS_CARD_LIMITS(subPixelInterpolationOffsetBits);
		PRINT_GRAPHICS_CARD_LIMITS(maxFramebufferWidth);
		PRINT_GRAPHICS_CARD_LIMITS(maxFramebufferHeight);
		PRINT_GRAPHICS_CARD_LIMITS(maxFramebufferLayers);
		PRINT_GRAPHICS_CARD_LIMITS(framebufferColorSampleCounts);
		PRINT_GRAPHICS_CARD_LIMITS(framebufferDepthSampleCounts);
		PRINT_GRAPHICS_CARD_LIMITS(framebufferStencilSampleCounts);
		PRINT_GRAPHICS_CARD_LIMITS(framebufferNoAttachmentsSampleCounts);
		PRINT_GRAPHICS_CARD_LIMITS(maxColorAttachments);
		PRINT_GRAPHICS_CARD_LIMITS(sampledImageColorSampleCounts);
		PRINT_GRAPHICS_CARD_LIMITS(sampledImageIntegerSampleCounts);
		PRINT_GRAPHICS_CARD_LIMITS(sampledImageDepthSampleCounts);
		PRINT_GRAPHICS_CARD_LIMITS(sampledImageStencilSampleCounts);
		PRINT_GRAPHICS_CARD_LIMITS(storageImageSampleCounts);
		PRINT_GRAPHICS_CARD_LIMITS(maxSampleMaskWords);
		PRINT_GRAPHICS_CARD_LIMITS(timestampComputeAndGraphics);
		PRINT_GRAPHICS_CARD_LIMITS(timestampPeriod);
		PRINT_GRAPHICS_CARD_LIMITS(maxClipDistances);
		PRINT_GRAPHICS_CARD_LIMITS(maxCullDistances);
		PRINT_GRAPHICS_CARD_LIMITS(maxCombinedClipAndCullDistances);
		PRINT_GRAPHICS_CARD_LIMITS(discreteQueuePriorities);
		PRINT_GRAPHICS_CARD_LIMITS(pointSizeRange[0]);
		PRINT_GRAPHICS_CARD_LIMITS(pointSizeRange[1]);
		PRINT_GRAPHICS_CARD_LIMITS(lineWidthRange[0]);
		PRINT_GRAPHICS_CARD_LIMITS(lineWidthRange[1]);
		PRINT_GRAPHICS_CARD_LIMITS(pointSizeGranularity);
		PRINT_GRAPHICS_CARD_LIMITS(lineWidthGranularity);
		PRINT_GRAPHICS_CARD_LIMITS(strictLines);
		PRINT_GRAPHICS_CARD_LIMITS(standardSampleLocations);
		PRINT_GRAPHICS_CARD_LIMITS(optimalBufferCopyOffsetAlignment);
		PRINT_GRAPHICS_CARD_LIMITS(optimalBufferCopyRowPitchAlignment);
		PRINT_GRAPHICS_CARD_LIMITS(nonCoherentAtomSize);
	}


	void VulkanGraphicsCardInfoViewer::print_all_physical_devices(const VkInstance& vulkan_instance, const VkSurfaceKHR& vulkan_surface)
	{
		assert(vulkan_surface);
		assert(vulkan_instance);

		uint32_t number_of_graphics_cards = 0;

		VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_graphics_cards, nullptr);
		vulkan_error_check(result);

		if(number_of_graphics_cards <= 0)
		{
			display_error_message("Error: Could not find any GPU's!");
		}
		else
		{
			spdlog::debug("------------------------------------------------------------------------------------------------------------");
			spdlog::debug("Number of available graphics cards: {}", number_of_graphics_cards);
			spdlog::debug("------------------------------------------------------------------------------------------------------------");

			// Preallocate memory for the available graphics cards.
			std::vector<VkPhysicalDevice> available_graphics_cards(number_of_graphics_cards);

			// Query information about all the graphics cards available on the system.
			result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_graphics_cards, available_graphics_cards.data());
			vulkan_error_check(result);

			// Loop through all graphics cards and print information about them.
			for(auto graphics_card : available_graphics_cards)
			{
				print_device_layers(graphics_card);
				print_device_extensions(graphics_card);
				print_graphics_card_info(graphics_card);
				print_physical_device_queue_families(graphics_card);
				print_surface_capabilities(graphics_card, vulkan_surface);
				print_supported_surface_formats(graphics_card, vulkan_surface);
				print_presentation_modes(graphics_card, vulkan_surface);
				print_graphics_card_memory_properties(graphics_card);
				print_graphics_card_features(graphics_card);
				print_graphics_cards_sparse_properties(graphics_card);
				print_graphics_card_limits(graphics_card);
			}
		}
	}


};
};
