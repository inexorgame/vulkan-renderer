#include "VulkanSettingsDecisionMaker.hpp"


namespace inexor {
namespace vulkan_renderer {


	VulkanSettingsDecisionMaker::VulkanSettingsDecisionMaker()
	{
	}


	VulkanSettingsDecisionMaker::~VulkanSettingsDecisionMaker()
	{
	}
	

	VkFormat VulkanSettingsDecisionMaker::decide_which_surface_color_format_for_swap_chain_images_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface)
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
		VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, nullptr);
		vulkan_error_check(result);

		// Query information about all the supported surface formats.
		std::vector<VkSurfaceFormatKHR> surface_formats(number_of_supported_formats);
		
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, surface_formats.data());
		vulkan_error_check(result);

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

		uint32_t available_graphics_cards_array_index = 0;
		
		// TODO: Implement!

		// TODO: Check if the selected device supports swapchains.
		// TODO: Check if the selected device supports queue families for graphics bits and presentation!

		return available_graphics_cards[available_graphics_cards_array_index];
	}

	
	VkPresentModeKHR VulkanSettingsDecisionMaker::decide_which_presentation_mode_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		uint32_t number_of_available_present_modes = 0;

		// First check how many present modes are available for the selected combination of graphics card and surface.
		VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, surface, &number_of_available_present_modes, nullptr);
		vulkan_error_check(result);

		// Preallocate memory for the available present modes.
		std::vector<VkPresentModeKHR> available_present_modes(number_of_available_present_modes);
		
		// Get information about the available present modes.
		result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, surface, &number_of_available_present_modes, available_present_modes.data());
		vulkan_error_check(result);

		for(auto present_mode : available_present_modes)
		{
			if(VK_PRESENT_MODE_MAILBOX_KHR == present_mode)
			{
				return present_mode;
			}
		}

		for(auto present_mode : available_present_modes)
		{
			if(VK_PRESENT_MODE_FIFO_KHR == present_mode)
			{
				return present_mode;
			}
		}

		cout << "Error: FIFO present mode is not supported by the swap chain!" << endl;

		return static_cast<VkPresentModeKHR>(-1);
	}


};
};
