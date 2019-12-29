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

	
	VkPhysicalDevice VulkanSettingsDecisionMaker::decide_which_graphics_card_to_use(const VkInstance& vulkan_instance)
	{
		uint32_t number_of_available_graphics_cards = 0;

		// First check how many graphics cards are available.
		VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_available_graphics_cards, nullptr);
		vulkan_error_check(result);

		if(number_of_available_graphics_cards <= 0)
		{
			display_error_message("Error: Could not find any graphics cards!");
			// TODO: Shutdown Vulkan and application!
		}

		// Preallocate memory for the available graphics cards.
		std::vector<VkPhysicalDevice> available_graphics_cards(number_of_available_graphics_cards);

		// Get information about the available graphics cards.
		result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_available_graphics_cards, available_graphics_cards.data());
		vulkan_error_check(result);

		// Let's just use the first graphics card in the array for now.
		return available_graphics_cards[0];
		
		// TODO: Implement a mechanism to select the "best" graphics card automatically.
		// TODO: In case multiple graphics cards are available let the user select one.
		// TODO: Select graphic card by command line parameter "-gpu" <index>
	}


};
};
