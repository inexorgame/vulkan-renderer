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

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Supported Vulkan API version: " << api_major_version << "." << api_minor_version << "." << api_version_patch << endl;
		cout << "--------------------------------------------------------------------------" << endl;

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
			cout << "--------------------------------------------------------------------------" << endl;
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

		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Supported surface formats: " << number_of_supported_formats << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		std::vector<VkSurfaceFormatKHR> surface_formats(number_of_supported_formats);

		vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, surface_formats.data());

		// 
		const std::unordered_map<int, std::string> surface_format_names =
		{
			{0, "VK_FORMAT_UNDEFINED"},
			{1, "VK_FORMAT_R4G4_UNORM_PACK8"},
			{2, "VK_FORMAT_R4G4B4A4_UNORM_PACK16"},
			{3, "VK_FORMAT_B4G4R4A4_UNORM_PACK16"},
			{4, "VK_FORMAT_R5G6B5_UNORM_PACK16"},
			{5, "VK_FORMAT_B5G6R5_UNORM_PACK16"},
			{6, "VK_FORMAT_R5G5B5A1_UNORM_PACK16"},
			{7, "VK_FORMAT_B5G5R5A1_UNORM_PACK16"},
			{8, "VK_FORMAT_A1R5G5B5_UNORM_PACK16"},
			{9, "VK_FORMAT_R8_UNORM"},
			{10, "VK_FORMAT_R8_SNORM"},
			{11, "VK_FORMAT_R8_USCALED"},
			{12, "VK_FORMAT_R8_SSCALED"},
			{13, "VK_FORMAT_R8_UINT"},
			{14, "VK_FORMAT_R8_SINT"},
			{15, "VK_FORMAT_R8_SRGB"},
			{16, "VK_FORMAT_R8G8_UNORM"},
			{17, "VK_FORMAT_R8G8_SNORM"},
			{18, "VK_FORMAT_R8G8_USCALED"},
			{19, "VK_FORMAT_R8G8_SSCALED"},
			{20, "VK_FORMAT_R8G8_UINT"},
			{21, "VK_FORMAT_R8G8_SINT"},
			{22, "VK_FORMAT_R8G8_SRGB"},
			{23, "VK_FORMAT_R8G8B8_UNORM"},
			{24, "VK_FORMAT_R8G8B8_SNORM"},
			{25, "VK_FORMAT_R8G8B8_USCALED"},
			{26, "VK_FORMAT_R8G8B8_SSCALED"},
			{27, "VK_FORMAT_R8G8B8_UINT"},
			{28, "VK_FORMAT_R8G8B8_SINT"},
			{29, "VK_FORMAT_R8G8B8_SRGB"},
			{30, "VK_FORMAT_B8G8R8_UNORM"},
			{31, "VK_FORMAT_B8G8R8_SNORM"},
			{32, "VK_FORMAT_B8G8R8_USCALED"},
			{33, "VK_FORMAT_B8G8R8_SSCALED"},
			{34, "VK_FORMAT_B8G8R8_UINT"},
			{35, "VK_FORMAT_B8G8R8_SINT"},
			{36, "VK_FORMAT_B8G8R8_SRGB"},
			{37, "VK_FORMAT_R8G8B8A8_UNORM"},
			{38, "VK_FORMAT_R8G8B8A8_SNORM"},
			{39, "VK_FORMAT_R8G8B8A8_USCALED"},
			{40, "VK_FORMAT_R8G8B8A8_SSCALED"},
			{41, "VK_FORMAT_R8G8B8A8_UINT"},
			{42, "VK_FORMAT_R8G8B8A8_SINT"},
			{43, "VK_FORMAT_R8G8B8A8_SRGB"},
			{44, "VK_FORMAT_B8G8R8A8_UNORM"},
			{45, "VK_FORMAT_B8G8R8A8_SNORM"},
			{46, "VK_FORMAT_B8G8R8A8_USCALED"},
			{47, "VK_FORMAT_B8G8R8A8_SSCALED"},
			{48, "VK_FORMAT_B8G8R8A8_UINT"},
			{49, "VK_FORMAT_B8G8R8A8_SINT"},
			{50, "VK_FORMAT_B8G8R8A8_SRGB"},
			{51, "VK_FORMAT_A8B8G8R8_UNORM_PACK32"},
			{52, "VK_FORMAT_A8B8G8R8_SNORM_PACK32"},
			{53, "VK_FORMAT_A8B8G8R8_USCALED_PACK32"},
			{54, "VK_FORMAT_A8B8G8R8_SSCALED_PACK32"},
			{55, "VK_FORMAT_A8B8G8R8_UINT_PACK32"},
			{56, "VK_FORMAT_A8B8G8R8_SINT_PACK32"},
			{57, "VK_FORMAT_A8B8G8R8_SRGB_PACK32"},
			{58, "VK_FORMAT_A2R10G10B10_UNORM_PACK32"},
			{59, "VK_FORMAT_A2R10G10B10_SNORM_PACK32"},
			{60, "VK_FORMAT_A2R10G10B10_USCALED_PACK32"},
			{61, "VK_FORMAT_A2R10G10B10_SSCALED_PACK32"},
			{62, "VK_FORMAT_A2R10G10B10_UINT_PACK32"},
			{63, "VK_FORMAT_A2R10G10B10_SINT_PACK32"},
			{64, "VK_FORMAT_A2B10G10R10_UNORM_PACK32"},
			{65, "VK_FORMAT_A2B10G10R10_SNORM_PACK32"},
			{66, "VK_FORMAT_A2B10G10R10_USCALED_PACK32"},
			{67, "VK_FORMAT_A2B10G10R10_SSCALED_PACK32"},
			{68, "VK_FORMAT_A2B10G10R10_UINT_PACK32"},
			{69, "VK_FORMAT_A2B10G10R10_SINT_PACK32"},
			{70, "VK_FORMAT_R16_UNORM"},
			{71, "VK_FORMAT_R16_SNORM"},
			{72, "VK_FORMAT_R16_USCALED"},
			{73, "VK_FORMAT_R16_SSCALED"},
			{74, "VK_FORMAT_R16_UINT"},
			{75, "VK_FORMAT_R16_SINT"},
			{76, "VK_FORMAT_R16_SFLOAT"},
			{77, "VK_FORMAT_R16G16_UNORM"},
			{78, "VK_FORMAT_R16G16_SNORM"},
			{79, "VK_FORMAT_R16G16_USCALED"},
			{80, "VK_FORMAT_R16G16_SSCALED"},
			{81, "VK_FORMAT_R16G16_UINT"},
			{82, "VK_FORMAT_R16G16_SINT"},
			{83, "VK_FORMAT_R16G16_SFLOAT"},
			{84, "VK_FORMAT_R16G16B16_UNORM"},
			{85, "VK_FORMAT_R16G16B16_SNORM"},
			{86, "VK_FORMAT_R16G16B16_USCALED"},
			{87, "VK_FORMAT_R16G16B16_SSCALED"},
			{88, "VK_FORMAT_R16G16B16_UINT"},
			{89, "VK_FORMAT_R16G16B16_SINT"},
			{90, "VK_FORMAT_R16G16B16_SFLOAT"},
			{91, "VK_FORMAT_R16G16B16A16_UNORM"},
			{92, "VK_FORMAT_R16G16B16A16_SNORM"},
			{93, "VK_FORMAT_R16G16B16A16_USCALED"},
			{94, "VK_FORMAT_R16G16B16A16_SSCALED"},
			{95, "VK_FORMAT_R16G16B16A16_UINT"},
			{96, "VK_FORMAT_R16G16B16A16_SINT"},
			{97, "VK_FORMAT_R16G16B16A16_SFLOAT"},
			{98, "VK_FORMAT_R32_UINT"},
			{99, "VK_FORMAT_R32_SINT"},
			{100, "VK_FORMAT_R32_SFLOAT"},
			{101, "VK_FORMAT_R32G32_UINT"},
			{102, "VK_FORMAT_R32G32_SINT"},
			{103, "VK_FORMAT_R32G32_SFLOAT"},
			{104, "VK_FORMAT_R32G32B32_UINT"},
			{105, "VK_FORMAT_R32G32B32_SINT"},
			{106, "VK_FORMAT_R32G32B32_SFLOAT"},
			{107, "VK_FORMAT_R32G32B32A32_UINT"},
			{108, "VK_FORMAT_R32G32B32A32_SINT"},
			{109, "VK_FORMAT_R32G32B32A32_SFLOAT"},
			{110, "VK_FORMAT_R64_UINT"},
			{111, "VK_FORMAT_R64_SINT"},
			{112, "VK_FORMAT_R64_SFLOAT"},
			{113, "VK_FORMAT_R64G64_UINT"},
			{114, "VK_FORMAT_R64G64_SINT"},
			{115, "VK_FORMAT_R64G64_SFLOAT"},
			{116, "VK_FORMAT_R64G64B64_UINT"},
			{117, "VK_FORMAT_R64G64B64_SINT"},
			{118, "VK_FORMAT_R64G64B64_SFLOAT"},
			{119, "VK_FORMAT_R64G64B64A64_UINT"},
			{120, "VK_FORMAT_R64G64B64A64_SINT"},
			{121, "VK_FORMAT_R64G64B64A64_SFLOAT"},
			{122, "VK_FORMAT_B10G11R11_UFLOAT_PACK32"},
			{123, "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32"},
			{124, "VK_FORMAT_D16_UNORM"},
			{125, "VK_FORMAT_X8_D24_UNORM_PACK32"},
			{126, "VK_FORMAT_D32_SFLOAT"},
			{127, "VK_FORMAT_S8_UINT"},
			{128, "VK_FORMAT_D16_UNORM_S8_UINT"},
			{129, "VK_FORMAT_D24_UNORM_S8_UINT"},
			{130, "VK_FORMAT_D32_SFLOAT_S8_UINT"},
			{131, "VK_FORMAT_BC1_RGB_UNORM_BLOCK"},
			{132, "VK_FORMAT_BC1_RGB_SRGB_BLOCK"},
			{133, "VK_FORMAT_BC1_RGBA_UNORM_BLOCK"},
			{134, "VK_FORMAT_BC1_RGBA_SRGB_BLOCK"},
			{135, "VK_FORMAT_BC2_UNORM_BLOCK"},
			{136, "VK_FORMAT_BC2_SRGB_BLOCK"},
			{137, "VK_FORMAT_BC3_UNORM_BLOCK"},
			{138, "VK_FORMAT_BC3_SRGB_BLOCK"},
			{139, "VK_FORMAT_BC4_UNORM_BLOCK"},
			{140, "VK_FORMAT_BC4_SNORM_BLOCK"},
			{141, "VK_FORMAT_BC5_UNORM_BLOCK"},
			{142, "VK_FORMAT_BC5_SNORM_BLOCK"},
			{143, "VK_FORMAT_BC6H_UFLOAT_BLOCK"},
			{144, "VK_FORMAT_BC6H_SFLOAT_BLOCK"},
			{145, "VK_FORMAT_BC7_UNORM_BLOCK"},
			{146, "VK_FORMAT_BC7_SRGB_BLOCK"},
			{147, "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK"},
			{148, "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK"},
			{149, "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK"},
			{150, "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK"},
			{151, "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK"},
			{152, "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK"},
			{153, "VK_FORMAT_EAC_R11_UNORM_BLOCK"},
			{154, "VK_FORMAT_EAC_R11_SNORM_BLOCK"},
			{155, "VK_FORMAT_EAC_R11G11_UNORM_BLOCK"},
			{156, "VK_FORMAT_EAC_R11G11_SNORM_BLOCK"},
			{157, "VK_FORMAT_ASTC_4x4_UNORM_BLOCK"},
			{158, "VK_FORMAT_ASTC_4x4_SRGB_BLOCK"},
			{159, "VK_FORMAT_ASTC_5x4_UNORM_BLOCK"},
			{160, "VK_FORMAT_ASTC_5x4_SRGB_BLOCK"},
			{161, "VK_FORMAT_ASTC_5x5_UNORM_BLOCK"},
			{162, "VK_FORMAT_ASTC_5x5_SRGB_BLOCK"},
			{163, "VK_FORMAT_ASTC_6x5_UNORM_BLOCK"},
			{164, "VK_FORMAT_ASTC_6x5_SRGB_BLOCK"},
			{165, "VK_FORMAT_ASTC_6x6_UNORM_BLOCK"},
			{166, "VK_FORMAT_ASTC_6x6_SRGB_BLOCK"},
			{167, "VK_FORMAT_ASTC_8x5_UNORM_BLOCK"},
			{168, "VK_FORMAT_ASTC_8x5_SRGB_BLOCK"},
			{169, "VK_FORMAT_ASTC_8x6_UNORM_BLOCK"},
			{170, "VK_FORMAT_ASTC_8x6_SRGB_BLOCK"},
			{171, "VK_FORMAT_ASTC_8x8_UNORM_BLOCK"},
			{172, "VK_FORMAT_ASTC_8x8_SRGB_BLOCK"},
			{173, "VK_FORMAT_ASTC_10x5_UNORM_BLOCK"},
			{174, "VK_FORMAT_ASTC_10x5_SRGB_BLOCK"},
			{175, "VK_FORMAT_ASTC_10x6_UNORM_BLOCK"},
			{176, "VK_FORMAT_ASTC_10x6_SRGB_BLOCK"},
			{177, "VK_FORMAT_ASTC_10x8_UNORM_BLOCK"},
			{178, "VK_FORMAT_ASTC_10x8_SRGB_BLOCK"},
			{179, "VK_FORMAT_ASTC_10x10_UNORM_BLOCK"},
			{180, "VK_FORMAT_ASTC_10x10_SRGB_BLOCK"},
			{181, "VK_FORMAT_ASTC_12x10_UNORM_BLOCK"},
			{182, "VK_FORMAT_ASTC_12x10_SRGB_BLOCK"},
			{183, "VK_FORMAT_ASTC_12x12_UNORM_BLOCK"},
			{184, "VK_FORMAT_ASTC_12x12_SRGB_BLOCK"},
			{1000156000, "VK_FORMAT_G8B8G8R8_422_UNORM"},
			{1000156001, "VK_FORMAT_B8G8R8G8_422_UNORM"},
			{1000156002, "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM"},
			{1000156003, "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM"},
			{1000156004, "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM"},
			{1000156005, "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM"},
			{1000156006, "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM"},
			{1000156007, "VK_FORMAT_R10X6_UNORM_PACK16"},
			{1000156008, "VK_FORMAT_R10X6G10X6_UNORM_2PACK16"},
			{1000156009, "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16"},
			{1000156010, "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16"},
			{1000156011, "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16"},
			{1000156012, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16"},
			{1000156013, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16"},
			{1000156014, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16"},
			{1000156015, "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16"},
			{1000156016, "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16"},
			{1000156017, "VK_FORMAT_R12X4_UNORM_PACK16"},
			{1000156018, "VK_FORMAT_R12X4G12X4_UNORM_2PACK16"},
			{1000156019, "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16"},
			{1000156020, "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16"},
			{1000156021, "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16"},
			{1000156022, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16"},
			{1000156023, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16"},
			{1000156024, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16"},
			{1000156025, "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16"},
			{1000156026, "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16"},
			{1000156027, "VK_FORMAT_G16B16G16R16_422_UNORM"},
			{1000156028, "VK_FORMAT_B16G16R16G16_422_UNORM"},
			{1000156029, "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM"},
			{1000156030, "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM"},
			{1000156031, "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM"},
			{1000156032, "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM"},
			{1000156033, "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM"},
			{1000054000, "VK_FORMAT_PVRTC1_2BPP_UNORM_BLOCK_IMG"},
			{1000054001, "VK_FORMAT_PVRTC1_4BPP_UNORM_BLOCK_IMG"},
			{1000054002, "VK_FORMAT_PVRTC2_2BPP_UNORM_BLOCK_IMG"},
			{1000054003, "VK_FORMAT_PVRTC2_4BPP_UNORM_BLOCK_IMG"},
			{1000054004, "VK_FORMAT_PVRTC1_2BPP_SRGB_BLOCK_IMG"},
			{1000054005, "VK_FORMAT_PVRTC1_4BPP_SRGB_BLOCK_IMG"},
			{1000054006, "VK_FORMAT_PVRTC2_2BPP_SRGB_BLOCK_IMG"},
			{1000054007, "VK_FORMAT_PVRTC2_4BPP_SRGB_BLOCK_IMG"},
			{1000066000, "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK_EXT"},
			{1000066001, "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK_EXT"},
			{1000066002, "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK_EXT"},
			{1000066003, "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK_EXT"},
			{1000066004, "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK_EXT"},
			{1000066005, "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK_EXT"},
			{1000066006, "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK_EXT"},
			{1000066007, "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK_EXT"},
			{1000066008, "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK_EXT"},
			{1000066009, "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK_EXT"},
			{1000066010, "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK_EXT"},
			{1000066011, "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK_EXT"},
			{1000066012, "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK_EXT"},
			{1000066013, "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK_EXT"}
		};


		for(std::size_t i=0; i<number_of_supported_formats; i++)
		{
			// We will not print the text which corresponds to the format.
			// You can look it up in the official Vulkan documentation.
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkFormat.html

			std::unordered_map<int, std::string>::const_iterator surface_format_lookup = surface_format_names.find(surface_formats[i].format);
			
			if(surface_format_lookup == surface_format_names.end())
			{
				// Name not found, print number instead.
				cout << surface_formats[i].format << endl;
			}
			else
			{
				cout << surface_format_names.at(surface_formats[i].format) << endl;
			}
		}

		cout << endl;
	}


	void VulkanGraphicsCardInfoViewer::print_presentation_modes(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
	{
		uint32_t number_of_present_modes = 0;
		
		// First check how many presentation modes are available.
		vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, vulkan_surface, &number_of_present_modes, nullptr);
	
		cout << "--------------------------------------------------------------------------" << endl;
		cout << "Available present modes: " << number_of_present_modes << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		// Preallocate memory for the presentation modes.
		std::vector<VkPresentModeKHR> present_modes(number_of_present_modes);

		vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, vulkan_surface, &number_of_present_modes, present_modes.data());

		const std::unordered_map<int, std::string> present_mode_names ={
			{0, "VK_PRESENT_MODE_IMMEDIATE_KHR"},
			{1, "VK_PRESENT_MODE_MAILBOX_KHR"},
			{2, "VK_PRESENT_MODE_FIFO_KHR"},
			{3, "VK_PRESENT_MODE_FIFO_RELAXED_KHR"},
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
				cout << present_mode_names.at(present_modes[i]) << endl;
			}
		}

		cout << endl;
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
		//cout << "Discrete queue priorities: " << graphics_card_properties.limits.discreteQueuePriorities << endl;

		cout << endl;

		cout << "Physical device limits:" << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		// 
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
		

		VkPhysicalDeviceFeatures graphics_card_features;

		// Check which features are supported by this graphics card.
		vkGetPhysicalDeviceFeatures(graphics_card, &graphics_card_features);

		cout << "Physical device features:" << endl;
		cout << "--------------------------------------------------------------------------" << endl;

		// 
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
		cout << "Checking memory properties." << endl;
		cout << "--------------------------------------------------------------------------" << endl;

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
	}


};
};
