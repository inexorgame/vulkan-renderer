#include "VulkanGraphicsCardInfoViewer.hpp"


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
		uint32_t api_version= 0;
		
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

		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Supported Vulkan API version: " << api_major_version << "." << api_minor_version << "." << api_version_patch << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;

		// Is Vulkan 1.1 available on this system?
		if(api_major_version > 1 || api_minor_version >= 1)
		{
			cout << "Vulkan 1.1 is supported." << endl;
		}

		cout << endl;
	}

	
	void VulkanGraphicsCardInfoViewer::print_physical_device_queue_families(const VkPhysicalDevice& graphics_card)
	{
		// The number of available queue families.
		uint32_t number_of_queue_families = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_queue_families, nullptr);

		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Number of queue families: " << number_of_queue_families << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		
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
				cout << "Queue family " << i << ": " << endl;
				cout << "------------------------------------------------------------------------------------------------------------" << endl;
				cout << "Queue Count: "          << queue_family_properties[i].queueCount << endl;
				cout << "Timestamp Valid Bits: " << queue_family_properties[i].timestampValidBits << endl;

				if(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)       cout << "VK_QUEUE_GRAPHICS_BIT" << endl;
				if(queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)        cout << "VK_QUEUE_COMPUTE_BIT" << endl;
				if(queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)       cout << "VK_QUEUE_TRANSFER_BIT" << endl;
				if(queue_family_properties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) cout << "VK_QUEUE_SPARSE_BINDING_BIT" << endl;
				if(queue_family_properties[i].queueFlags & VK_QUEUE_PROTECTED_BIT)      cout << "VK_QUEUE_PROTECTED_BIT" << endl;

				uint32_t width  = queue_family_properties[i].minImageTransferGranularity.width;
				uint32_t height = queue_family_properties[i].minImageTransferGranularity.width;
				uint32_t depth  = queue_family_properties[i].minImageTransferGranularity.depth;
			
				cout << "Min Image Timestamp Granularity: " << width << ", " << height << ", " << depth << endl;
				cout << endl;
			}
		}
	}

	
	void VulkanGraphicsCardInfoViewer::print_instance_layers()
	{
		uint32_t number_of_instance_layers = 0;

		// First check how many instance layers are available.
		VkResult result = vkEnumerateInstanceLayerProperties(&number_of_instance_layers, nullptr);
		vulkan_error_check(result);

		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Number of instance layers: " << number_of_instance_layers << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;

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

				cout << "Name: "         << instance_layer.layerName << endl;
				cout << "Spec Version: " << spec_major << "." << spec_minor << "." << spec_patch << endl;
				cout << "Impl Version: " << instance_layer.implementationVersion << endl;
				cout << "Description: "  << instance_layer.description << endl;
				cout << endl;
			}
		
			cout << endl;
		}
	}


	void VulkanGraphicsCardInfoViewer::print_instance_extensions()
	{
		uint32_t number_of_instance_extensions = 0;

		// First check how many instance extensions are available.
		VkResult result = vkEnumerateInstanceExtensionProperties(nullptr, &number_of_instance_extensions, nullptr);
		vulkan_error_check(result);

		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Number of instance extensions: " << number_of_instance_extensions << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;

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

				cout << "Spec version: " << spec_major << "." << spec_minor << "." << spec_patch << " \tName: " << extension.extensionName << endl;
			}

			cout << endl;
		}
	}


	void VulkanGraphicsCardInfoViewer::print_device_layers(const VkPhysicalDevice& graphics_card)
	{
		uint32_t number_of_device_layers = 0;

		// First check how many device layers are available.
		VkResult result = vkEnumerateDeviceLayerProperties(graphics_card, &number_of_device_layers, nullptr);
		vulkan_error_check(result);

		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Number of device layers: " << number_of_device_layers << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		
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

				cout << "Name: "          << device_layer.description << endl;
				cout << "Spec version: "  << spec_major << "." << spec_minor << "." << spec_patch << endl;
				cout << "Impl version : " << device_layer.implementationVersion << endl;
				cout << "Description: "   << device_layer.description << endl;
				cout << endl;
			}
			
			cout << endl;
		}
	}


	void VulkanGraphicsCardInfoViewer::print_device_extensions(const VkPhysicalDevice& graphics_card)
	{
		uint32_t number_of_device_extensions = 0;
		
		// First check how many device extensions are available.
		VkResult result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_device_extensions, nullptr);
		vulkan_error_check(result);
		
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Number of device extensions: " << number_of_device_extensions << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		
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

				cout << "Spec version: " << spec_major << "." << spec_minor << "." << spec_patch << " \tName: " << device_extension.extensionName << endl;
			}

			cout << endl;		
		}
	}

	
	void VulkanGraphicsCardInfoViewer::print_surface_capabilities(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
	{
		cout << "Printing surface capabilities" << endl;
		
		VkSurfaceCapabilitiesKHR surface_capabilities;

		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, vulkan_surface, &surface_capabilities);
		vulkan_error_check(result);

		cout << "minImageCount: "           << surface_capabilities.minImageCount << endl;
		cout << "maxImageCount: "           << surface_capabilities.maxImageCount << endl;
		cout << "currentExtent.width: "     << surface_capabilities.currentExtent.width << endl;
		cout << "currentExtent.height: "    << surface_capabilities.currentExtent.height << endl;
		cout << "minImageExtent.width: "    << surface_capabilities.minImageExtent.width << endl;
		cout << "minImageExtent.height: "   << surface_capabilities.minImageExtent.height << endl;
		cout << "maxImageExtent.width: "    << surface_capabilities.maxImageExtent.width << endl;
		cout << "maxImageExtent.height: "   << surface_capabilities.maxImageExtent.height << endl;
		cout << "maxImageArrayLayers: "     << surface_capabilities.maxImageArrayLayers << endl;
		cout << "supportedTransforms: "     << surface_capabilities.supportedTransforms << endl;
		cout << "currentTransform: "        << surface_capabilities.currentTransform << endl;
		cout << "supportedCompositeAlpha: " << surface_capabilities.supportedCompositeAlpha << endl;
		cout << "supportedUsageFlags: "     << surface_capabilities.supportedUsageFlags << endl;
		cout << endl;
	}


	void VulkanGraphicsCardInfoViewer::print_supported_surface_formats(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
	{
		uint32_t number_of_supported_formats = 0;
		
		// First check how many formats are supported.
		VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, nullptr);
		vulkan_error_check(result);

		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Supported surface formats: " << number_of_supported_formats << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		
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
					cout << surface_formats[i].format << endl;
				}
				else
				{
					// We found a text description for this in surface_format_names.
					cout << surface_format_names.at(surface_formats[i].format) << endl;
				}
			}

			cout << endl;
		}
	}


	void VulkanGraphicsCardInfoViewer::print_presentation_modes(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
	{
		uint32_t number_of_present_modes = 0;
		
		// First check how many presentation modes are available.
		VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, vulkan_surface, &number_of_present_modes, nullptr);
		vulkan_error_check(result);

		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Available present modes: " << number_of_present_modes << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;

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
					cout << present_modes[i] << endl;
				}
				else
				{
					// We found a text description for this in surface_format_names.
					cout << present_mode_names.at(present_modes[i]) << endl;
				}
			}

			cout << endl;
		}
	}


	void VulkanGraphicsCardInfoViewer::print_graphics_card_info(const VkPhysicalDevice& graphics_card)
	{
		// The properties of the graphics card.
		VkPhysicalDeviceProperties graphics_card_properties;

		// Get the information about that graphics card.
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);

		// Print the name of the graphics card.
		cout << "Graphics card: " << graphics_card_properties.deviceName << endl;

		// Get the major, minor and patch version of the Vulkan API version.
		uint32_t VulkanAPIversion     = graphics_card_properties.apiVersion;
		uint32_t vulkan_version_major = VK_VERSION_MAJOR(VulkanAPIversion);
		uint32_t vulkan_version_minor = VK_VERSION_MINOR(VulkanAPIversion);
		uint32_t vulkan_version_patch = VK_VERSION_MAJOR(VulkanAPIversion);

		// The Vulkan version which is supported by the graphics card.
		cout << "Vulkan API supported version: " << vulkan_version_major << "." << vulkan_version_minor << "." << vulkan_version_patch << endl;

		// Get the major, minor and patch version of the driver version.
		uint32_t driver_version_major = VK_VERSION_MAJOR(graphics_card_properties.driverVersion);
		uint32_t driver_version_minor = VK_VERSION_MINOR(graphics_card_properties.driverVersion);
		uint32_t driver_version_patch = VK_VERSION_PATCH(graphics_card_properties.driverVersion);

		// The driver version.
		// Always keep your graphics drivers up to date!
		// Note: The driver version format is NOT standardised!
		cout << "Driver version: " << driver_version_major << "." << driver_version_minor << "." << driver_version_patch << endl;
		cout << "Vendor ID: "      << graphics_card_properties.vendorID << endl;
		cout << "Device ID: "      << graphics_card_properties.deviceID << endl;

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
			cout << "Device type: " << graphics_card_types[graphics_card_properties.deviceType] << endl;
		}

		// From the Vulkan documentation:
		// The number of discrete priorities that can be assigned to a queue based on the value of each member of VkDeviceQueueCreateInfo::pQueuePriorities.
		// This must be at least 2, and levels must be spread evenly over the range, with at least one level at 1.0, and another at 0.0.
		//cout << "Discrete queue priorities: " << graphics_card_properties.limits.discreteQueuePriorities << endl;
		
		cout << endl;
	}


	void VulkanGraphicsCardInfoViewer::print_graphics_card_memory_properties(const VkPhysicalDevice& graphics_card)
	{
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Graphics card's memory properties:" << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;

		// Check memory properties of this graphics card.
		VkPhysicalDeviceMemoryProperties graphics_card_memory_properties;

		vkGetPhysicalDeviceMemoryProperties(graphics_card, &graphics_card_memory_properties);

		cout << "Number of memory types: " << graphics_card_memory_properties.memoryTypeCount << endl;
		cout << "Number of heap types: "   << graphics_card_memory_properties.memoryHeapCount << endl;

		cout << endl;


		// Loop through all memory types and list their features.
		for(std::size_t i=0; i<graphics_card_memory_properties.memoryTypeCount; i++)
		{
			cout << "[" << i << "] Heap index: "<< graphics_card_memory_properties.memoryTypes[i].heapIndex << endl;

			auto &propertyFlag = graphics_card_memory_properties.memoryTypes[i].propertyFlags;

			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)        cout << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)        cout << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)       cout << "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)         cout << "VK_MEMORY_PROPERTY_HOST_CACHED_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)    cout << "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_PROTECTED_BIT)           cout << "VK_MEMORY_PROPERTY_PROTECTED_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) cout << "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) cout << "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD" << endl;
		}

		cout << endl;

		// Loop through all memory heaps.
		for(std::size_t i=0; i<graphics_card_memory_properties.memoryHeapCount; i++)
		{
			cout << "Heap memory [" << i << "]" << ", memory size: " << graphics_card_memory_properties.memoryHeaps[i].size << " bytes." << endl;

			auto &propertyFlag = graphics_card_memory_properties.memoryHeaps[i].flags;

			if(propertyFlag & VK_MEMORY_HEAP_DEVICE_LOCAL_BIT)   cout << "VK_MEMORY_HEAP_DEVICE_LOCAL_BIT (GPU MEMORY)" << endl;
			if(propertyFlag & VK_MEMORY_HEAP_MULTI_INSTANCE_BIT) cout << "VK_MEMORY_HEAP_MULTI_INSTANCE_BIT" << endl;
		}
	}


	void VulkanGraphicsCardInfoViewer::print_graphics_card_features(const VkPhysicalDevice& graphics_card)
	{
		VkPhysicalDeviceFeatures graphics_card_features;

		// Check which features are supported by this graphics card.
		vkGetPhysicalDeviceFeatures(graphics_card, &graphics_card_features);
		
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Graphics card's features:" << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		
		// This little helper macro prints the regarded information about graphics card features to the console.
		#define PRINT_GRAPHICS_CARD_FEATURE(x) cout << #x << ": " << ((graphics_card_features.x) ? "yes" : "no") << endl;

		PRINT_GRAPHICS_CARD_FEATURE(robustBufferAccess)
		PRINT_GRAPHICS_CARD_FEATURE(fullDrawIndexUint32)
		PRINT_GRAPHICS_CARD_FEATURE(imageCubeArray)
		PRINT_GRAPHICS_CARD_FEATURE(independentBlend)
		PRINT_GRAPHICS_CARD_FEATURE(geometryShader)
		PRINT_GRAPHICS_CARD_FEATURE(tessellationShader)
		PRINT_GRAPHICS_CARD_FEATURE(sampleRateShading)
		PRINT_GRAPHICS_CARD_FEATURE(dualSrcBlend)
		PRINT_GRAPHICS_CARD_FEATURE(logicOp)
		PRINT_GRAPHICS_CARD_FEATURE(multiDrawIndirect)
		PRINT_GRAPHICS_CARD_FEATURE(drawIndirectFirstInstance)
		PRINT_GRAPHICS_CARD_FEATURE(depthClamp)
		PRINT_GRAPHICS_CARD_FEATURE(depthBiasClamp)
		PRINT_GRAPHICS_CARD_FEATURE(fillModeNonSolid)
		PRINT_GRAPHICS_CARD_FEATURE(depthBounds)
		PRINT_GRAPHICS_CARD_FEATURE(wideLines)
		PRINT_GRAPHICS_CARD_FEATURE(largePoints)
		PRINT_GRAPHICS_CARD_FEATURE(alphaToOne)
		PRINT_GRAPHICS_CARD_FEATURE(multiViewport)
		PRINT_GRAPHICS_CARD_FEATURE(samplerAnisotropy)
		PRINT_GRAPHICS_CARD_FEATURE(textureCompressionETC2)
		PRINT_GRAPHICS_CARD_FEATURE(textureCompressionASTC_LDR)
		PRINT_GRAPHICS_CARD_FEATURE(textureCompressionBC)
		PRINT_GRAPHICS_CARD_FEATURE(occlusionQueryPrecise)
		PRINT_GRAPHICS_CARD_FEATURE(pipelineStatisticsQuery)
		PRINT_GRAPHICS_CARD_FEATURE(vertexPipelineStoresAndAtomics)
		PRINT_GRAPHICS_CARD_FEATURE(fragmentStoresAndAtomics)
		PRINT_GRAPHICS_CARD_FEATURE(shaderTessellationAndGeometryPointSize)
		PRINT_GRAPHICS_CARD_FEATURE(shaderImageGatherExtended)
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageExtendedFormats)
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageMultisample)
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageReadWithoutFormat)
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageWriteWithoutFormat)
		PRINT_GRAPHICS_CARD_FEATURE(shaderUniformBufferArrayDynamicIndexing)
		PRINT_GRAPHICS_CARD_FEATURE(shaderSampledImageArrayDynamicIndexing)
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageBufferArrayDynamicIndexing)
		PRINT_GRAPHICS_CARD_FEATURE(shaderStorageImageArrayDynamicIndexing)
		PRINT_GRAPHICS_CARD_FEATURE(shaderClipDistance)
		PRINT_GRAPHICS_CARD_FEATURE(shaderCullDistance)
		PRINT_GRAPHICS_CARD_FEATURE(shaderFloat64)
		PRINT_GRAPHICS_CARD_FEATURE(shaderInt64)
		PRINT_GRAPHICS_CARD_FEATURE(shaderInt16)
		PRINT_GRAPHICS_CARD_FEATURE(shaderResourceResidency)
		PRINT_GRAPHICS_CARD_FEATURE(shaderResourceMinLod)
		PRINT_GRAPHICS_CARD_FEATURE(sparseBinding)
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidencyBuffer)
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidencyImage2D)
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidencyImage3D)
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidency2Samples)
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidency4Samples)
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidency8Samples)
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidency16Samples)
		PRINT_GRAPHICS_CARD_FEATURE(sparseResidencyAliased)
		PRINT_GRAPHICS_CARD_FEATURE(variableMultisampleRate)
		PRINT_GRAPHICS_CARD_FEATURE(inheritedQueries)

		cout << endl;
	}


	void VulkanGraphicsCardInfoViewer::print_graphics_cards_sparse_properties(const VkPhysicalDevice& graphics_card)
	{
		VkPhysicalDeviceProperties graphics_card_properties;

		// Get the information about that graphics card.
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);
		
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Graphics card's sparse properties:" << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;

		#define PRINT_SPARSE_PROPERTIES(x) cout << #x << ": " << (graphics_card_properties.sparseProperties.x) << endl;

		PRINT_SPARSE_PROPERTIES(residencyStandard2DBlockShape);
		PRINT_SPARSE_PROPERTIES(residencyStandard2DMultisampleBlockShape);
		PRINT_SPARSE_PROPERTIES(residencyStandard3DBlockShape);
		PRINT_SPARSE_PROPERTIES(residencyAlignedMipSize);
		PRINT_SPARSE_PROPERTIES(residencyNonResidentStrict);

		cout << endl;
	}

	
	void VulkanGraphicsCardInfoViewer::print_graphics_card_limits(const VkPhysicalDevice& graphics_card)
	{
		VkPhysicalDeviceProperties graphics_card_properties;

		// Get the information about that graphics card.
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);
		
		cout << "------------------------------------------------------------------------------------------------------------" << endl;
		cout << "Graphics card's limits:" << endl;
		cout << "------------------------------------------------------------------------------------------------------------" << endl;

		// This little helper macro prints the regarded information about graphics card limits to the console.
		#define PRINT_GRAPHICS_CARD_LIMITS(x) cout << #x << ": " << (graphics_card_properties.limits.x) << endl;

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

		cout << endl;
	}


	void VulkanGraphicsCardInfoViewer::print_all_physical_devices(const VkInstance& vulkan_instance, const VkSurfaceKHR& vulkan_surface)
	{
		uint32_t number_of_graphics_cards = 0;

		VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_graphics_cards, nullptr);
		vulkan_error_check(result);

		if(number_of_graphics_cards <= 0)
		{
			display_error_message("Error: Could not find any GPU's!");
		}
		else
		{
			cout << "------------------------------------------------------------------------------------------------------------" << endl;
			cout << "Number of available graphics cards: " << number_of_graphics_cards << endl;
			cout << "------------------------------------------------------------------------------------------------------------" << endl;

			// Preallocate memory for the available graphics cards.
			std::vector<VkPhysicalDevice> available_graphics_cards(number_of_graphics_cards);

			// Query information about all the graphics cards available on the system.
			result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_graphics_cards, available_graphics_cards.data());
			vulkan_error_check(result);

			// Loop through all graphics cards and print information about them.
			for(auto graphics_cards : available_graphics_cards)
			{
				print_graphics_card_info(graphics_cards);
				print_physical_device_queue_families(graphics_cards);
				print_surface_capabilities(graphics_cards, vulkan_surface);
				print_supported_surface_formats(graphics_cards, vulkan_surface);
				print_presentation_modes(graphics_cards, vulkan_surface);
				cout << endl;
			}
		}
	}


};
};
