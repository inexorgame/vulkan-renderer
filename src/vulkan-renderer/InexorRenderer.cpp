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


	bool InexorRenderer::init_vulkan()
	{
		cout << "Setting up Vulkan." << endl;

		VkApplicationInfo appInfo = {};

		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Inexor";
		appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
		appInfo.pEngineName = "Inexor";
		appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);

		// TODO: Should we switch to VK_API_VERSION_1_1 ?
		appInfo.apiVersion = VK_API_VERSION_1_0;


		VkInstanceCreateInfo createInfo = {};

		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;

		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		createInfo.enabledExtensionCount = glfwExtensionCount;
		createInfo.ppEnabledExtensionNames = glfwExtensions;
		createInfo.enabledLayerCount = 0;


		// Let's create an instance.
		VkResult result = vkCreateInstance(&createInfo, nullptr, &vulkan_instance);

		if(VK_SUCCESS != result)
		{
			cout << "vkCreateInstance failed!" << endl;
			return false;
		}

		cout << "vkCreateInstance succeeded." << endl;

		// Lets ask how many graphics cards are in thie machine.
		result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_physical_devices, NULL);

		cout << "There are " << number_of_physical_devices << " graphics cards available." << endl;

		// Preallocate memory: resize the vector of graphics cards.
		graphics_cards.resize(number_of_physical_devices);

		cout << "Getting information about graphics cards" << endl;

		// Fill out the information of the graphics cards.
		result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_physical_devices, &graphics_cards[0]);

		// Loop through all available graphics card and print information about them to the console.
		for(std::size_t i=0; i< number_of_physical_devices; i++)
		{
			print_graphics_card_info(graphics_cards[i]);
			cout << endl;
		}

		// Let's just use the first one in the array for now.
		// TODO: Implement a mechanism to select a graphics card.
		// TODO: In case multiple graphics cards are available let the user select one.
		VkPhysicalDevice selected_graphics_card = graphics_cards[0];

		uint32_t number_of_queue_families = 0;

		// Ask for the family queues.
		vkGetPhysicalDeviceQueueFamilyProperties(selected_graphics_card, &number_of_queue_families, NULL);

		cout << "Number of queue families: " << number_of_queue_families << endl;

		// The queue families of the selected graphics card.
		std::vector<VkQueueFamilyProperties> queue_family_properties;

		// Preallocate memory for the family queues.
		queue_family_properties.resize(number_of_queue_families);

		// Get information about physical device queue family properties.
		vkGetPhysicalDeviceQueueFamilyProperties(selected_graphics_card, &number_of_queue_families, &queue_family_properties[0]);

		// Loop through all available queue families.
		for(std::size_t i=0; i< number_of_queue_families; i++)
		{
			cout << endl;
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
		}


		VkDeviceQueueCreateInfo device_queue_create_info = {};

		device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_create_info.pNext = NULL;
		device_queue_create_info.flags = NULL;

		// TODO: Look which queue family fits best for what we want to do.
		device_queue_create_info.queueFamilyIndex = NULL;

		// TODO: Check if 4 queues are even supported!
		device_queue_create_info.queueCount = 4;
		device_queue_create_info.pQueuePriorities = NULL;


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
		result = vkCreateDevice(selected_graphics_card, &device_create_info, NULL, &device);

		return true;
	}


	void InexorRenderer::print_graphics_card_info(const VkPhysicalDevice& graphics_card)
	{
		// The properties of the graphics card.
		VkPhysicalDeviceProperties graphics_card_properties;

		// Get the information about that graphics card.
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);

		// Print the name of the device.
		cout << "Graphics card: " << graphics_card_properties.deviceName << endl;

		uint32_t VulkanAPIversion = graphics_card_properties.apiVersion;

		// The Vulkan version which is supported by the graphics card.
		cout << "Vulkan API supported version: " << VK_VERSION_MAJOR(VulkanAPIversion) << "." << VK_VERSION_MINOR(VulkanAPIversion) << "." << VK_VERSION_PATCH(VulkanAPIversion) << endl;

		// Always keep your graphics drivers up to date!

		// The driver version.
		// @note: The driver version format is NOT standardised!
		cout << "Driver version: " << VK_VERSION_MAJOR(graphics_card_properties.driverVersion) << "." << VK_VERSION_MINOR(graphics_card_properties.driverVersion) << "." << VK_VERSION_PATCH(graphics_card_properties.driverVersion) << endl;

		// Vendor ID.
		cout << "Vender ID: " << graphics_card_properties.vendorID << endl;

		// Device ID.
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

		if(graphics_card_properties.deviceType <= 4)
		{
			cout << "Device type: " << graphics_card_types[graphics_card_properties.deviceType] << endl;
		}

		// From the Vulkan documentation:
		// The number of discrete priorities that can be assigned to a queue based on the value of each member
		// of VkDeviceQueueCreateInfo::pQueuePriorities.This must be at least 2, and levels must be spread evenly
		// over the range, with at least one level at 1.0, and another at 0.0.
		cout << "discreteQueuePriorities: " << graphics_card_properties.limits.discreteQueuePriorities << endl;


		VkPhysicalDeviceFeatures graphics_card_features;

		// Check which features are supported by this graphics card.
		vkGetPhysicalDeviceFeatures(graphics_card, &graphics_card_features);

		// We will only print some of the features in the structure.
		// For more information check the Vulkan documentation.

		// Check if geometry shader is supported.
		cout << "Geometry shader supported: " << ((graphics_card_features.geometryShader) ? "yes" : "no") << endl;

		// TODO: Check for more features if neccesary.


		// Check memory properties of this graphics card.
		VkPhysicalDeviceMemoryProperties graphics_card_memory_properties;

		vkGetPhysicalDeviceMemoryProperties(graphics_card, &graphics_card_memory_properties);

		cout << "Getting memory properties of this graphics card." << endl;
		
		// TODO: Do something with the memory properties?
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
		close_window();
	}


};
};
