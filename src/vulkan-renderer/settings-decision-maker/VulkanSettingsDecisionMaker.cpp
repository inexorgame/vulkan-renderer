#include "VulkanSettingsDecisionMaker.hpp"


namespace inexor {
namespace vulkan_renderer {


	VulkanSettingsDecisionMaker::VulkanSettingsDecisionMaker()
	{
	}


	VulkanSettingsDecisionMaker::~VulkanSettingsDecisionMaker()
	{
	}
	
	
	uint32_t VulkanSettingsDecisionMaker::decide_how_many_images_in_swapchain_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		uint32_t number_of_images_in_swapchain = 0;

		VkSurfaceCapabilitiesKHR surface_capabilities = {};

		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
		vulkan_error_check(result);

		// Determine how many images in swapchain to use.
		number_of_images_in_swapchain = surface_capabilities.minImageCount + 1;

		// If the maximum number of images available in swapchain is greater than our current number, chose it
		if((surface_capabilities.maxImageCount > 0) && (surface_capabilities.maxImageCount < number_of_images_in_swapchain))
		{
			number_of_images_in_swapchain = surface_capabilities.maxImageCount;
		}

		return number_of_images_in_swapchain;
	}


	VkResult VulkanSettingsDecisionMaker::decide_which_surface_color_format_in_swapchain_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface, VkFormat& color_format, VkColorSpaceKHR& color_space)
	{
		uint32_t number_of_surface_formats = 0;

		// First check how many surface formats are available.
		VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, surface, &number_of_surface_formats, nullptr);
		if(VK_SUCCESS != result) return result;

		if(!(number_of_surface_formats > 0))
		{
			std::string error_message = "Error: No surface formats could be found by fpGetPhysicalDeviceSurfaceFormatsKHR!";
			display_error_message(error_message);
		}
		
		// Preallocate memory for available surface formats.
		std::vector<VkSurfaceFormatKHR> available_surface_formats(number_of_surface_formats);
		
		// Get information about all surface formats available.
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, surface, &number_of_surface_formats, available_surface_formats.data());
		vulkan_error_check(result);

		
		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM.
		if(1 == number_of_surface_formats && VK_FORMAT_UNDEFINED == available_surface_formats[0].format)
		{
			color_format = VK_FORMAT_B8G8R8A8_UNORM;
			color_space = available_surface_formats[0].colorSpace;
		}
		else
		{
			// Loop through the list of available surface formats and
			// check for the presence of VK_FORMAT_B8G8R8A8_UNORM.
			bool found_B8G8R8A8_UNORM = false;

			for(auto&& surface_format : available_surface_formats)
			{
				if(VK_FORMAT_B8G8R8A8_UNORM == surface_format.format)
				{
					color_format = surface_format.format;
					color_space = surface_format.colorSpace;

					found_B8G8R8A8_UNORM = true;
					break;
				}
			}
			
			// In case VK_FORMAT_B8G8R8A8_UNORM is not available select the first available color format.
			if(!found_B8G8R8A8_UNORM)
			{
				color_format = available_surface_formats[0].format;
				color_space  = available_surface_formats[0].colorSpace;
			}
		}

		return VK_SUCCESS;
	}

	
	VkPhysicalDevice VulkanSettingsDecisionMaker::decide_which_graphics_card_to_use(const VkInstance& vulkan_instance, const int& preferred_graphics_card_index)
	{
		cout << "Deciding automatically which graphics card to use." << endl;
		
		uint32_t number_of_available_graphics_cards = 0;

		// First check how many graphics cards are available.
		VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_available_graphics_cards, nullptr);
		vulkan_error_check(result);

		if(number_of_available_graphics_cards <= 0)
		{
			display_error_message("Error: Could not find any graphics cards!");
			// TODO: Shutdown Vulkan and application!
		}
		
		cout << "There are " << number_of_available_graphics_cards << " graphics cards available." << endl;

		// Preallocate memory for the available graphics cards.
		std::vector<VkPhysicalDevice> available_graphics_cards(number_of_available_graphics_cards);

		// Get information about the available graphics cards.
		result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_available_graphics_cards, available_graphics_cards.data());
		vulkan_error_check(result);

		// If there is only 1 graphics card available, we don't have a choice and must use that one.
		if(1 == available_graphics_cards.size())
		{
			cout << "Because there is only 1 graphics card, we don't have a choice and must use that one." << endl;

			// We are done: use the first graphics card!
			return available_graphics_cards[0];
		}

		// The user passed a command line parameter -gpu_index <NUMBER> which determines the graphics card array index to use.
		if(-1 != preferred_graphics_card_index)
		{
			// Check if this array index is valid!
			if(preferred_graphics_card_index >= 0 && preferred_graphics_card_index < available_graphics_cards.size())
			{
				cout << "Command line parameter -gpu_index specified. Using graphics card index " << preferred_graphics_card_index << "." << endl;
	
				// TODO: Check if the graphics card selected by the user meets all the criteria we need!

				// We are done: Use the graphics card which was specified by the user's command line argument.
				return available_graphics_cards[preferred_graphics_card_index];
			}
			else
			{
				// No, this array index for available_graphics_cards is invalid!
				cout << "Error: invalid command line argument! Graphics card array index " << preferred_graphics_card_index << " is invalid!" << endl;

				// We are NOT done: Select the best graphics card automatically!
			}
		}

		cout << "No command line argument for preferred graphics card given. Detecting best graphics card automatically." << endl;

		uint32_t available_graphics_cards_array_index = 0;
		
		

		// TODO: Implement!

		// TODO: Check if the selected device supports swapchains.
		// TODO: Check if the selected device supports queue families for graphics bits and presentation!
		// TODO: Prefere VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU!




		return available_graphics_cards[available_graphics_cards_array_index];
	}

	
	VkSurfaceTransformFlagsKHR VulkanSettingsDecisionMaker::decide_which_image_transformation_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		// Bitmask of VkSurfaceTransformFlagBitsKHR.
		VkSurfaceTransformFlagsKHR pre_transform = {};

		VkSurfaceCapabilitiesKHR surface_capabilities = {};

		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
		vulkan_error_check(result);

		if(surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			// We prefer a non-rotated transform.
			pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			pre_transform = surface_capabilities.currentTransform;
		}

		return pre_transform;
	}


	VkPresentModeKHR VulkanSettingsDecisionMaker::decide_which_presentation_mode_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		uint32_t number_of_available_present_modes = 0;

		// First check how many present modes are available for the selected combination of graphics card and window surface.
		VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, surface, &number_of_available_present_modes, nullptr);
		vulkan_error_check(result);

		// Preallocate memory for the available present modes.
		std::vector<VkPresentModeKHR> available_present_modes(number_of_available_present_modes);
		
		// Get information about the available present modes.
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, surface, &number_of_available_present_modes, available_present_modes.data());
		vulkan_error_check(result);

		for(auto present_mode : available_present_modes)
		{
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
			// VK_PRESENT_MODE_MAILBOX_KHR specifies that the presentation engine waits for the next vertical blanking period
			// to update the current image. Tearing cannot be observed. An internal single-entry queue is used to hold pending
			// presentation requests. If the queue is full when a new presentation request is received, the new request replaces
			// the existing entry, and any images associated with the prior entry become available for re-use by the application.
			// One request is removed from the queue and processed during each vertical blanking period in which the queue is non-empty.
			if(VK_PRESENT_MODE_MAILBOX_KHR == present_mode)
			{
				return present_mode;
			}
		}

		cout << "Info: VK_PRESENT_MODE_MAILBOX_KHR is not supported by the regarded device." << endl;
		cout << "Let's hope VK_PRESENT_MODE_FIFO_KHR is supported." << endl;

		for(auto present_mode : available_present_modes)
		{
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
			// VK_PRESENT_MODE_FIFO_KHR specifies that the presentation engine waits for the next vertical blanking period to update the current image.
			// Tearing cannot be observed. An internal queue is used to hold pending presentation requests. New requests are appended to the end of the queue,
			// and one request is removed from the beginning of the queue and processed during each vertical blanking period in which the queue is non-empty.
			// This is the only value of presentMode that is required to be supported.
			if(VK_PRESENT_MODE_FIFO_KHR == present_mode)
			{
				return present_mode;
			}
		}

		cout << "Error: FIFO present mode is not supported by the swap chain!" << endl;

		// Lets try with any present mode available!
		if(available_present_modes.size() > 0)
		{
			// Let's just pick the first one.
			return available_present_modes[0];
		}

		cout << "Error: The regarded graphics card does not support any presentation at all!" << endl;
		
		// TODO: Shutdown Vulkan and application.

		return VK_PRESENT_MODE_MAX_ENUM_KHR;
	}


	void VulkanSettingsDecisionMaker::decide_width_and_height_of_swapchain_extent(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface, uint32_t& window_width, uint32_t& window_height, VkExtent2D& swapchain_extent)
	{
		// Bitmask of VkSurfaceTransformFlagBitsKHR.
		VkSurfaceCapabilitiesKHR surface_capabilities = {};

		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
		vulkan_error_check(result);

		if(surface_capabilities.currentExtent.width == UINT32_MAX && surface_capabilities.currentExtent.height == UINT32_MAX)
		{
			// The size of the window dictates the swapchain's extent.
			swapchain_extent.width  = window_width;
			swapchain_extent.height = window_height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match.
			swapchain_extent = surface_capabilities.currentExtent;
			window_width     = surface_capabilities.currentExtent.width;
			window_height    = surface_capabilities.currentExtent.height;

		}
	}


};
};
