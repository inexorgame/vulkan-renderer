#include "InexorRenderer.hpp"

using namespace std;


namespace inexor {
namespace vulkan_renderer {

	
	void InexorRenderer::init_window(const int width, const int height, const std::string& window_name)
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		window = glfwCreateWindow(width, height, window_name.c_str(), nullptr, nullptr);
	}


	void InexorRenderer::close_window()
	{
		glfwDestroyWindow(window);
		glfwTerminate();
	}


	VkResult InexorRenderer::create_vulkan_instance(const std::string& application_name, const std::string& engine_name, const uint32_t application_version, const uint32_t engine_version, bool enable_validation_layers)
	{
		// Print some debug messages to the console.
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

		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pNext = NULL;
		app_info.pApplicationName = application_name.c_str();
		app_info.applicationVersion = application_version;
		app_info.pEngineName = engine_name.c_str();
		app_info.engineVersion = engine_version;
		app_info.apiVersion = VK_API_VERSION_1_0;

		// TODO: Should we switch to Vulkan 1.1?

		const std::vector<const char*> instance_extensions = 
		{
			// We need this to render to window surfaces.
			VK_KHR_SURFACE_EXTENSION_NAME,

			// We need this for validation.
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,

			// Add more extensions depending on the operating system.
			// TODO: Add more operating systems!
			#if defined(_WIN32)
				VK_KHR_WIN32_SURFACE_EXTENSION_NAME
			#endif
		};

		// TODO: Check if we need more device or instance extensions!

		VkInstanceCreateInfo instance_create_info = {};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pNext = NULL;
		instance_create_info.flags = NULL;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledExtensionCount = static_cast<uint32_t>(instance_extensions.size());
		instance_create_info.ppEnabledExtensionNames = instance_extensions.data();

		// Check if Khronos validation layer is available.
		if(enable_validation_layers)
		{
			const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

			// Check if this layer is available at instance leve..
			uint32_t instance_layer_count = 0;
			vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr);
			
			std::vector<VkLayerProperties> instance_layer_properties(instance_layer_count);
			vkEnumerateInstanceLayerProperties(&instance_layer_count, instance_layer_properties.data());

			bool validation_layer_available = false;

			for(VkLayerProperties layer : instance_layer_properties)
			{
				if(0 == strcmp(validation_layer_name, layer.layerName))
				{
					// Yes, this validation layer is available!
					validation_layer_available = true;
					break;
				}
			}

			if(validation_layer_available)
			{
				instance_create_info.ppEnabledLayerNames = &validation_layer_name;
				instance_create_info.enabledLayerCount = 1;
			}
			else
			{
				display_error_message("Error: Validation layer VK_LAYER_KHRONOS_validation not present, validation is disabled.");
			}
		}

		return vkCreateInstance(&instance_create_info, nullptr, &vulkan_instance);
	}


	void InexorRenderer::enumerate_physical_devices()
	{
		VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_physical_devices, NULL);

		if(VK_SUCCESS != result)
		{
			std::string error_message = "Error: " + get_error_string(result);
			display_error_message(error_message);
		}

		if(number_of_physical_devices <= 0)
		{
			display_error_message("Error: Could not find any GPU's!");
		}

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Number of available GPUs: " << number_of_physical_devices << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		// Preallocate memory for the available graphics cards.
		graphics_cards.resize(number_of_physical_devices);

		result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_physical_devices, graphics_cards.data());

		if(VK_SUCCESS != result)
		{
			std::string error_message = "Error: Could not enumerate physical devices! " + get_error_string(result);
			display_error_message(error_message);
		}

		// TODO: Add GPU selection based on command line arguments.

		for(std::size_t i=0; i<number_of_physical_devices; i++)
		{
			print_graphics_card_info(graphics_cards[i]);
			cout << endl;
		}
	}


	VkResult InexorRenderer::create_physical_device(const VkPhysicalDevice& graphics_card)
	{
		uint32_t number_of_queue_families = 0;

		// Ask for the family queues.
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
			cout << "VK_QUEUE_GRAPHICS_BIT " << (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) << endl;
			cout << "VK_QUEUE_COMPUTE_BIT " << (queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) << endl;
			cout << "VK_QUEUE_TRANSFER_BIT " << (queue_family_properties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) << endl;
			cout << "VK_QUEUE_SPARSE_BINDING_BIT " << (queue_family_properties[i].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT) << endl;
			cout << "VK_QUEUE_PROTECTED_BIT " << (queue_family_properties[i].queueFlags & VK_QUEUE_PROTECTED_BIT) << endl;

			cout << "Queue Count: " << queue_family_properties[i].queueCount << endl;
			cout << "Timestamp Valid Bits: " << queue_family_properties[i].timestampValidBits << endl;

			uint32_t width = queue_family_properties[i].minImageTransferGranularity.width;
			uint32_t height = queue_family_properties[i].minImageTransferGranularity.width;
			uint32_t depth = queue_family_properties[i].minImageTransferGranularity.depth;
			
			cout << "Min Image Timestamp Granularity: " << width << ", " << height << ", " << depth << endl;
			cout << endl;
		}

		VkDeviceQueueCreateInfo device_queue_create_info = {};

		device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_create_info.pNext = NULL;
		device_queue_create_info.flags = NULL;

		// TODO: Look which queue family fits best for what we want to do.
		device_queue_create_info.queueFamilyIndex = NULL;

		// TODO: Check if 4 queues are even supported!
		device_queue_create_info.queueCount = 4;

		const float queue_priorities[] = {1.0f, 1.0f, 1.0f, 1.0f};
		device_queue_create_info.pQueuePriorities = queue_priorities;


		VkDeviceCreateInfo device_create_info = {};
		VkPhysicalDeviceFeatures used_features = {};

		device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_create_info.pNext = NULL;
		device_create_info.flags = NULL;
		device_create_info.queueCreateInfoCount = 1;
		device_create_info.pQueueCreateInfos = &device_queue_create_info;
		device_create_info.enabledLayerCount = NULL;
		device_create_info.ppEnabledLayerNames = NULL;
		device_create_info.enabledExtensionCount = NULL;
		device_create_info.ppEnabledExtensionNames = NULL;
		device_create_info.pEnabledFeatures = &used_features;

		
		// TODO: Lets pick the best device instead of the default device.
		// TODO: Let the user choose which device to use.
		return vkCreateDevice(graphics_card, &device_create_info, NULL, &device);
	}


	bool InexorRenderer::init_vulkan()
	{
		cout << "Initialising Vulkan instance." << endl;

		VkResult result = create_vulkan_instance(INEXOR_APPLICATION_NAME, INEXOR_ENGINE_NAME, INEXOR_APPLICATION_VERSION, INEXOR_ENGINE_VERSION, true);

		if(VK_SUCCESS != result)
		{
			std::string error_message = "Error: " + get_error_string(result);
			display_error_message(error_message);
			return false;
		}

		enumerate_physical_devices();

		// Let's just use the first one in the array for now.
		// TODO: Implement a mechanism to select a graphics card.
		// TODO: In case multiple graphics cards are available let the user select one.
		VkPhysicalDevice selected_graphics_card = graphics_cards[0];

		result = create_physical_device(selected_graphics_card);

		if(VK_SUCCESS != result)
		{
			std::string error_message = "Error: " + get_error_string(result);
			display_error_message(error_message);
			return false;
		}

		// The number of available Vulkan layers.
		uint32_t number_of_layers = 0;

		// Ask for the number of available Vulkan layers.
		vkEnumerateInstanceLayerProperties(&number_of_layers, NULL);

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Number of instance layers: " << number_of_layers << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		std::vector<VkLayerProperties> layer_properties;

		layer_properties.resize(number_of_layers);

		vkEnumerateInstanceLayerProperties(&number_of_layers, layer_properties.data());

		// Loop through all available layers and print information about them.
		for(std::size_t i=0; i< number_of_layers; i++)
		{
			// Extract major, minor and patch version of spec.
			uint32_t spec_major = VK_VERSION_MAJOR(layer_properties[i].specVersion);
			uint32_t spec_minor = VK_VERSION_MINOR(layer_properties[i].specVersion);
			uint32_t spec_patch = VK_VERSION_PATCH(layer_properties[i].specVersion);

			cout << "Name: "         << layer_properties[i].layerName << endl;
			cout << "Spec Version: " << spec_major << "." << spec_minor << "." << spec_patch << endl;
			cout << "Impl Version: " << layer_properties[i].implementationVersion << endl;
			cout << "Description: "  << layer_properties[i].description << endl;
			cout << endl;
		}
		
		cout << endl;

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

		uint32_t number_of_device_layers = 0;
		vkEnumerateDeviceLayerProperties(selected_graphics_card, &number_of_device_layers, NULL);

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Number of device layers: " << number_of_device_layers << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		std::vector<VkLayerProperties> device_layer_properties;

		device_layer_properties.resize(number_of_device_layers);

		vkEnumerateDeviceLayerProperties(selected_graphics_card, &number_of_device_layers, device_layer_properties.data());

		for(std::size_t i=0; i<number_of_device_layers; i++)
		{
			uint32_t spec_major = VK_VERSION_MAJOR(layer_properties[i].specVersion);
			uint32_t spec_minor = VK_VERSION_MINOR(layer_properties[i].specVersion);
			uint32_t spec_patch = VK_VERSION_PATCH(layer_properties[i].specVersion);

			cout << "Name: "          << device_layer_properties[i].description << endl;
			cout << "Spec Version: "  << spec_major << "." << spec_minor << "." << spec_patch << endl;
			cout << "Impl Version : " << device_layer_properties[i].implementationVersion << endl;
			cout << "Description: "   << device_layer_properties[i].description << endl;
			cout << endl;
		}
		
		cout << endl;

		return true;
	}


	void InexorRenderer::print_graphics_card_info(const VkPhysicalDevice& graphics_card)
	{
		// The properties of the graphics card.
		VkPhysicalDeviceProperties graphics_card_properties;

		// Get the information about that graphics card.
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);

		// Print the name of the graphics card.
		cout << "Graphics card: " << graphics_card_properties.deviceName << endl;

		uint32_t VulkanAPIversion = graphics_card_properties.apiVersion;

		// The Vulkan version which is supported by the graphics card.
		cout << "Vulkan API supported version: " << VK_VERSION_MAJOR(VulkanAPIversion) << "." << VK_VERSION_MINOR(VulkanAPIversion) << "." << VK_VERSION_PATCH(VulkanAPIversion) << endl;

		// The driver version.
		// Always keep your graphics drivers up to date!
		// Note: The driver version format is NOT standardised!
		cout << "Driver version: " << VK_VERSION_MAJOR(graphics_card_properties.driverVersion) << "." << VK_VERSION_MINOR(graphics_card_properties.driverVersion) << "." << VK_VERSION_PATCH(graphics_card_properties.driverVersion) << endl;
		cout << "Vendor ID: " << graphics_card_properties.vendorID << endl;
		cout << "Device ID: " << graphics_card_properties.deviceID << endl;

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
		cout << "Number of heap types: " << graphics_card_memory_properties.memoryHeapCount << endl;

		// Loop through all memory types and list their features.
		for(std::size_t i=0; i<graphics_card_memory_properties.memoryTypeCount; i++)
		{
			cout << "Heap index: "<< graphics_card_memory_properties.memoryTypes[i].heapIndex << endl;
			
			auto &propertyFlag = graphics_card_memory_properties.memoryTypes[i].propertyFlags;

			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) cout << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) cout << "VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) cout << "VK_MEMORY_PROPERTY_HOST_COHERENT_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) cout << "VK_MEMORY_PROPERTY_HOST_CACHED_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) cout << "VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_PROTECTED_BIT) cout << "VK_MEMORY_PROPERTY_PROTECTED_BIT" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) cout << "VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD) cout << "VK_MEMORY_PROPERTY_DEVICE_UNCACHED_BIT_AMD" << endl;
			if(propertyFlag & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) cout << "VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT" << endl;

			cout << endl;
		}
	}


	void InexorRenderer::shutdown_vulkan()
	{
		vkDestroyInstance(vulkan_instance, nullptr);
	}


	InexorRenderer::InexorRenderer()
	{
		window = nullptr;
		number_of_physical_devices = 0;
		graphics_cards.clear();
		vulkan_instance = {};
	}


	InexorRenderer::~InexorRenderer()
	{
	}


	void InexorRenderer::init()
	{
		init_window(800, 600, "Inexor Vulkan Renderer");
		init_vulkan();
	}


	void InexorRenderer::run()
	{
		while(!glfwWindowShouldClose(window))
		{
			glfwPollEvents();
		}
	}


	void InexorRenderer::cleanup()
	{
		// TODO: Cleanup in reverse order of initialisation.
		close_window();
	}


};
};
