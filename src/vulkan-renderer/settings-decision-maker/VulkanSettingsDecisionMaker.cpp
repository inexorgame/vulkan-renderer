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
		uint32_t number_of_supported_formats = 0;
		
		// First check how many formats are supported.
		VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, nullptr);
		vulkan_error_check(result);

		// Query information about all the supported surface formats.
		std::vector<VkSurfaceFormatKHR> available_surface_formats(number_of_supported_formats);
		
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, vulkan_surface, &number_of_supported_formats, available_surface_formats.data());
		vulkan_error_check(result);

		
		// A list of image formats that we can accept.
		const std::vector<VkFormat> surface_format_wishlist =
		{
			// This is the default format which should be available everywhere.
			VK_FORMAT_B8G8R8A8_UNORM,

			// TODO: Add more formats to the wishlist.
			// The priority is decreasing from top to bottom
		};
		
		// We will enumerate all available image formats and compare it with our wishlist.

		// Is one of our selected formats supported?
		// IMPORTANT: Wishlist priority depends on the NESTING of the for loops!
		for(auto current_wished_surface_format : surface_format_wishlist)
		{
			for(auto current_surface_format : available_surface_formats)
			{
				if(current_wished_surface_format == current_surface_format.format)
				{
					return current_surface_format.format;
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


		// TODO: Refactor this into a wishlist!
		// IMPORTANT: Wishlist priority depends on the nesting of the for loops!

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
		// wait.. wtf

		// Lets try with any present mode available!
		if(available_present_modes.size() > 0)
		{
			// Let's just pick the first one.
			return available_present_modes[0];
		}

		cout << "Error: The regarded graphics card does not support any presentation at all!" << endl;
		
		return VK_PRESENT_MODE_MAX_ENUM_KHR;
	}


};
};
