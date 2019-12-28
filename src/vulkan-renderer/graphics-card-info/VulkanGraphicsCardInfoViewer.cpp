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
		vkEnumerateInstanceVersion(&api_version);
		
		// Extract major, minor and patch version of the Vulkan API available.
		uint16_t api_major_version = VK_VERSION_MAJOR(api_version);
		uint16_t api_minor_version = VK_VERSION_MINOR(api_version);
		uint16_t api_version_patch = VK_VERSION_PATCH(api_version);

		cout << "Supported Vulkan API version: " << api_major_version << "." << api_minor_version << "." << api_version_patch << endl;

		// Is Vulkan 1.1 available on this system?
		if(api_major_version > 1 || api_minor_version >= 1)
		{
			cout << "Vulkan 1.1 is supported." << endl;
		}

		cout << endl;
	}

	
	void VulkanGraphicsCardInfoViewer::print_physical_device_queue_families(const VkPhysicalDevice& graphics_card)
	{
		uint32_t number_of_queue_families = 0;

		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_queue_families, NULL);

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Number of queue families: " << number_of_queue_families << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		// The queue families of the selected graphics card.
		std::vector<VkQueueFamilyProperties> queue_family_properties;

		// Preallocate memory for the family queues.
		queue_family_properties.resize(number_of_queue_families);

		// Get information about physical device queue family properties.
		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_queue_families, queue_family_properties.data());

		// Loop through all available queue families.
		for(std::size_t i=0; i< number_of_queue_families; i++)
		{
			cout << "Queue family " << i << ": " << endl;
			cout << "Queue Count: " << queue_family_properties[i].queueCount << endl;
			cout << "Timestamp Valid Bits: " << queue_family_properties[i].timestampValidBits << endl;

			if(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)       cout << "VK_QUEUE_GRAPHICS_BIT" << endl;
			if(queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)        cout << "VK_QUEUE_COMPUTE_BIT" << endl;
			if(queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT)        cout << "VK_QUEUE_COMPUTE_BIT" << endl;
			if(queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT)       cout << "VK_QUEUE_TRANSFER_BIT" << endl;
			if(queue_family_properties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) cout << "VK_QUEUE_SPARSE_BINDING_BIT" << endl;
			if(queue_family_properties[i].queueFlags & VK_QUEUE_PROTECTED_BIT)      cout << "VK_QUEUE_PROTECTED_BIT" << endl;

			uint32_t width = queue_family_properties[i].minImageTransferGranularity.width;
			uint32_t height = queue_family_properties[i].minImageTransferGranularity.width;
			uint32_t depth = queue_family_properties[i].minImageTransferGranularity.depth;
			
			cout << "Min Image Timestamp Granularity: " << width << ", " << height << ", " << depth << endl;
			cout << endl;
		}
	}

	
	void VulkanGraphicsCardInfoViewer::print_instance_layer_properties()
	{
		// The number of available Vulkan layers.
		uint32_t number_of_layers = 0;

		// Ask for the number of available Vulkan layers.
		vkEnumerateInstanceLayerProperties(&number_of_layers, NULL);

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Number of instance layers: " << number_of_layers << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		std::vector<VkLayerProperties> instance_layer_properties;

		instance_layer_properties.resize(number_of_layers);

		vkEnumerateInstanceLayerProperties(&number_of_layers, instance_layer_properties.data());

		// Loop through all available layers and print information about them.
		for(std::size_t i=0; i< number_of_layers; i++)
		{
			// Extract major, minor and patch version of spec.
			uint32_t spec_major = VK_VERSION_MAJOR(instance_layer_properties[i].specVersion);
			uint32_t spec_minor = VK_VERSION_MINOR(instance_layer_properties[i].specVersion);
			uint32_t spec_patch = VK_VERSION_PATCH(instance_layer_properties[i].specVersion);

			cout << "Name: "         << instance_layer_properties[i].layerName << endl;
			cout << "Spec Version: " << spec_major << "." << spec_minor << "." << spec_patch << endl;
			cout << "Impl Version: " << instance_layer_properties[i].implementationVersion << endl;
			cout << "Description: "  << instance_layer_properties[i].description << endl;
			cout << endl;
		}
		
		cout << endl;
	}


	void VulkanGraphicsCardInfoViewer::print_instance_extensions()
	{
		uint32_t number_of_extensions = 0;

		vkEnumerateInstanceExtensionProperties(NULL, &number_of_extensions, NULL);

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Number of extensions: " << number_of_extensions << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		std::vector<VkExtensionProperties> extensions;

		// Preallocate memory for extension properties.
		extensions.resize(number_of_extensions);

		vkEnumerateInstanceExtensionProperties(NULL, &number_of_extensions, extensions.data());

		for(std::size_t i=0; i<number_of_extensions; i++)
		{
			cout << "Name: " << extensions[i].extensionName << endl;
			cout << "Spec: " << extensions[i].specVersion << endl;
			cout << endl;
		}

		cout << endl;
	}


	void VulkanGraphicsCardInfoViewer::print_device_layers(const VkPhysicalDevice& graphics_card)
	{
		uint32_t number_of_device_layers = 0;
		vkEnumerateDeviceLayerProperties(graphics_card, &number_of_device_layers, NULL);

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Number of device layers: " << number_of_device_layers << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		std::vector<VkLayerProperties> device_layer_properties;

		device_layer_properties.resize(number_of_device_layers);

		vkEnumerateDeviceLayerProperties(graphics_card, &number_of_device_layers, device_layer_properties.data());

		for(std::size_t i=0; i<number_of_device_layers; i++)
		{
			uint32_t spec_major = VK_VERSION_MAJOR(device_layer_properties[i].specVersion);
			uint32_t spec_minor = VK_VERSION_MINOR(device_layer_properties[i].specVersion);
			uint32_t spec_patch = VK_VERSION_PATCH(device_layer_properties[i].specVersion);

			cout << "Name: "          << device_layer_properties[i].description << endl;
			cout << "Spec Version: "  << spec_major << "." << spec_minor << "." << spec_patch << endl;
			cout << "Impl Version : " << device_layer_properties[i].implementationVersion << endl;
			cout << "Description: "   << device_layer_properties[i].description << endl;
			cout << endl;
		}
		
		cout << endl;
	}

	
	void VulkanGraphicsCardInfoViewer::print_surface_capabilities(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
	{
		VkSurfaceCapabilitiesKHR surface_capabilities;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, vulkan_surface, &surface_capabilities);
		
		cout << "Printing surface capabilities" << endl;

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
		vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, nullptr);

		cout << "Supported surface formats: " << number_of_supported_formats << endl;

		std::vector<VkSurfaceFormatKHR> surface_formats(number_of_supported_formats);

		vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, surface_formats.data());

		for(std::size_t i=0; i<number_of_supported_formats; i++)
		{
			// We will not print the text which corresponds to the format.
			// You can look it up in the official Vulkan documentation.
			cout << surface_formats[i].format << endl;
		}
	}


	void VulkanGraphicsCardInfoViewer::print_presentation_modes(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
	{
		uint32_t number_of_present_modes = 0;
		
		// First check how many presentation modes are available.
		vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, vulkan_surface, &number_of_present_modes, nullptr);
	
		cout << "Available present modes: " << number_of_present_modes << endl;

		std::vector<VkPresentModeKHR> present_modes(number_of_present_modes);

		vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, vulkan_surface, &number_of_present_modes, present_modes.data());

		for(std::size_t i=0; i<number_of_present_modes; i++)
		{
			cout << present_modes[i] << endl;
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
		uint32_t VulkanAPIversion = graphics_card_properties.apiVersion;
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
		// The number of discrete priorities that can be assigned to a queue based on the value of each member
		// of VkDeviceQueueCreateInfo::pQueuePriorities.This must be at least 2, and levels must be spread evenly
		// over the range, with at least one level at 1.0, and another at 0.0.
		cout << "Discrete queue priorities: " << graphics_card_properties.limits.discreteQueuePriorities << endl;

		VkPhysicalDeviceFeatures graphics_card_features;

		// Check which features are supported by this graphics card.
		vkGetPhysicalDeviceFeatures(graphics_card, &graphics_card_features);

		// We will only print some of the features in the structure.
		// For more information check the Vulkan documentation.
		// Check if geometry shader is supported.
		cout << "Geometry shader supported: " << ((graphics_card_features.geometryShader) ? "yes" : "no") << endl;

		// TODO: Check for more features if neccesary.

		cout << endl;
		cout << "Checking memory properties." << endl;

		// Check memory properties of this graphics card.
		VkPhysicalDeviceMemoryProperties graphics_card_memory_properties;

		vkGetPhysicalDeviceMemoryProperties(graphics_card, &graphics_card_memory_properties);

		cout << "Number of memory types: " << graphics_card_memory_properties.memoryTypeCount << endl;
		cout << "Number of heap types: "   << graphics_card_memory_properties.memoryHeapCount << endl;

		// Loop through all memory types and list their features.
		for(std::size_t i=0; i<graphics_card_memory_properties.memoryTypeCount; i++)
		{
			cout << "Heap index: "<< graphics_card_memory_properties.memoryTypes[i].heapIndex << endl;
			
			auto &propertyFlag = graphics_card_memory_properties.memoryTypes[i].propertyFlags;

			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)        cout << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)        cout << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)       cout << "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_CACHED_BIT)         cout << "VK_MEMORY_PROPERTY_HOST_CACHED_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT)    cout << "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_PROTECTED_BIT)           cout << "VK_MEMORY_PROPERTY_PROTECTED_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) cout << "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) cout << "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)        cout << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" << endl;

			cout << endl;
		}
	}


};
};
