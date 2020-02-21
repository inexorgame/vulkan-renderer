#include "VulkanSettingsDecisionMaker.hpp"
using namespace std;


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
		cout << "Deciding automatically how many images in swapchain to use." << endl;

		uint32_t number_of_images_in_swapchain = 0;

		VkSurfaceCapabilitiesKHR surface_capabilities = {};

		VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics_card, surface, &surface_capabilities);
		vulkan_error_check(result);

		// TODO: Refactor! How many images do we actually need? Is triple buffering the best option?

		// Determine how many images in swapchain to use.
		number_of_images_in_swapchain = surface_capabilities.minImageCount + 1;

		// If the maximum number of images available in swapchain is greater than our current number, chose it.
		if((surface_capabilities.maxImageCount > 0) && (surface_capabilities.maxImageCount < number_of_images_in_swapchain))
		{
			number_of_images_in_swapchain = surface_capabilities.maxImageCount;
		}

		return number_of_images_in_swapchain;
	}


	std::optional<VkSurfaceFormatKHR> VulkanSettingsDecisionMaker::decide_which_surface_color_format_in_swapchain_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		cout << "Deciding automatically which surface color format in swapchain to use." << endl;

		uint32_t number_of_available_surface_formats = 0;

		// First check how many surface formats are available.
		VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, surface, &number_of_available_surface_formats, nullptr);
		if(VK_SUCCESS != result)
		{
			std::string error_message = get_error_description_text(result);
			display_error_message(error_message);
			return std::nullopt;
		}

		if(0 == number_of_available_surface_formats)
		{
			std::string error_message = "Error: No surface formats could be found by fpGetPhysicalDeviceSurfaceFormatsKHR!";
			display_error_message(error_message);
			return std::nullopt;
		}
		
		// Preallocate memory for available surface formats.
		std::vector<VkSurfaceFormatKHR> available_surface_formats(number_of_available_surface_formats);
		
		// Get information about all surface formats available.
		result = vkGetPhysicalDeviceSurfaceFormatsKHR(graphics_card, surface, &number_of_available_surface_formats, available_surface_formats.data());
		vulkan_error_check(result);
		
		
		VkSurfaceFormatKHR accepted_color_format = {};

		
		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM.
		if(1 == number_of_available_surface_formats && VK_FORMAT_UNDEFINED == available_surface_formats[0].format)
		{
			accepted_color_format.format = VK_FORMAT_B8G8R8A8_UNORM;
			accepted_color_format.colorSpace = available_surface_formats[0].colorSpace;
		}
		else
		{
			// This vector contais all the formats that we can accept.
			// Currently we use VK_FORMAT_B8G8R8A8_UNORM only, since it's the norm.
			std::vector<VkFormat> accepted_formats = 
			{
				VK_FORMAT_B8G8R8A8_UNORM
				// Add more accepted formats here..
			};

			bool found_acceptable_format = false;

			// Loop through the list of available surface formats and compare with the list of acceptable formats.
			for(auto&& surface_format : available_surface_formats)
			{
				for(std::size_t i=0; i<accepted_formats.size(); i++)
				{
					if(accepted_formats[i] == surface_format.format)
					{
						accepted_color_format.format = surface_format.format;
						accepted_color_format.colorSpace = surface_format.colorSpace;

						found_acceptable_format = true;

						return accepted_color_format;
					}
				}
			}
			
			// In case VK_FORMAT_B8G8R8A8_UNORM is not available select the first available color format.
			if(!found_acceptable_format)
			{
				if(available_surface_formats.size() > 0)
				{
					accepted_color_format.format = available_surface_formats[0].format;
					accepted_color_format.colorSpace  = available_surface_formats[0].colorSpace;

					return accepted_color_format;
				}
				else
				{
					return std::nullopt;
				}
			}
		}

		return accepted_color_format;
	}


	VkBool32 VulkanSettingsDecisionMaker::decide_if_graphics_card_is_suitable(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		// The properties of the graphics card.
		VkPhysicalDeviceProperties graphics_card_properties;

		// The features of the graphics card.
		VkPhysicalDeviceFeatures graphics_card_features;

		// Get the information about that graphics card's properties.
		vkGetPhysicalDeviceProperties(graphics_card, &graphics_card_properties);
		
		// Get the information about the graphics card's features.
		vkGetPhysicalDeviceFeatures(graphics_card, &graphics_card_features);

		cout << "Checking suitability of graphics card " << graphics_card_properties.deviceName << "." << endl;

		// Step 1: Check if swapchain is supported.
		// In theory we could have used the code from VulkanAvailabilityChecks, but I didn't want
		// VulkanSettingsDecisionMaker to have a dependency just because of this one code part here.

		bool swapchain_is_supported = false;

		uint32_t number_of_available_device_extensions = 0;

		VkResult result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_available_device_extensions, nullptr);
		vulkan_error_check(result);

		if(0 == number_of_available_device_extensions)
		{
			display_error_message("Error: No Vulkan device extensions available!");

			// Since there are no device extensions available at all, the desired one is not supported either.
			swapchain_is_supported = false;
		}
		else
		{
			// Preallocate memory for device extensions.
			std::vector<VkExtensionProperties> device_extensions(number_of_available_device_extensions);
			result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_available_device_extensions, device_extensions.data());
			vulkan_error_check(result);

			// Loop through all available device extensions and search for the requested one.
			for(const VkExtensionProperties& device_extension : device_extensions)
			{
				if(0 == strcmp(device_extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
				{
					swapchain_is_supported = true;
				}
			}
		}

		if(!swapchain_is_supported)
		{
			cout << "This device is not suitable because it does not support swap chain!" << endl;
			return false;
		}


		// Step 2: Check if presentation is supported
		// TODO: Check if the selected device supports queue families for graphics bits and presentation!
		
		VkBool32 presentation_available = false;
		
		// Query if presentation is supported.
		result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, 0, surface, &presentation_available);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return false;
		}

		if(!presentation_available)
		{
			cout << "This device is not suitable because it does not support presentation!" << endl;
			return false;
		}


		// Add more suitability checks here if neccesary.
		// ....

		return true;
	}

	
	std::optional<VkPhysicalDevice> VulkanSettingsDecisionMaker::decide_which_graphics_card_to_use(const VkInstance& vulkan_instance, const VkSurfaceKHR& surface, const std::optional<uint32_t>& preferred_graphics_card_index)
	{
		uint32_t number_of_available_graphics_cards = 0;

		// First check how many graphics cards are available.
		VkResult result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_available_graphics_cards, nullptr);
		vulkan_error_check(result);

		if(0 == number_of_available_graphics_cards)
		{
			// In this case there are not Vulkan compatible graphics cards available!
			display_error_message("Error: Could not find any graphics cards!");
			return std::nullopt;
		}

		// Preallocate memory for the available graphics cards.
		std::vector<VkPhysicalDevice> available_graphics_cards(number_of_available_graphics_cards);

		// Get information about the available graphics cards.
		result = vkEnumeratePhysicalDevices(vulkan_instance, &number_of_available_graphics_cards, available_graphics_cards.data());
		vulkan_error_check(result);



		// ATTEMPT 1
		// If there is only 1 graphics card available, we don't have a choice and must use that one.
		// The preferred graphics card index which could have been specified by the user can either be this one or an invalid index!
		if(1 == number_of_available_graphics_cards)
		{
			cout << "Because there is only 1 graphics card available, we don't have a choice and must use that one." << endl;
			
			// Did the user specify a preferred GPU by command line argument?
			// If so, let's take a look at what he wanted us to use.
			if(preferred_graphics_card_index.has_value())
			{
				// Since we only have one graphics card to choose from, index 0 is our only option.
				if(0 != preferred_graphics_card_index.value())
				{
					// A little hint message just to be sure.
					cout << "Ignoring command line argument -GPU " << preferred_graphics_card_index.value() << " because there is only one GPU to chose from." << endl;
				}
				if(!(preferred_graphics_card_index.value() >= 0 && preferred_graphics_card_index.value() < available_graphics_cards.size()))
				{
					// Haha.
					cout << "Warning: Array index for selected graphics card would have been invalid anyways!" << endl;
				}
			}

			if(decide_if_graphics_card_is_suitable(available_graphics_cards[0], surface))
			{
				cout << "The only graphics card available is suitable for the application's purpose!" << endl;
				return available_graphics_cards[0];
			}
			else
			{
				std::string error_message = "Error: The only graphics card available is unsuitable for the application's purposes!";
				display_error_message(error_message);
				return std::nullopt;
			}
		}



		// ATTEMPT 2
		// There is more than 1 graphics cards available, but the user specified which one should be preferred.
		// It is important to note that the preferred graphics card can be unsuitable though! If that is the case,
		// the automatic graphics card selection mechanism is responsible for finding a suitable graphics card.
		// The user can also simply change the command line argument and try to prefer another graphics card.
		if(preferred_graphics_card_index.has_value())
		{
			// Check if this array index is valid!
			if(preferred_graphics_card_index.value() >= 0 && preferred_graphics_card_index < number_of_available_graphics_cards)
			{
				cout << "Command line parameter for desired GPU specified. Checking graphics card with index " << preferred_graphics_card_index.value() << "." << endl;
	
				// Check if the graphics card selected by the user meets all the criteria we need!
				if(decide_if_graphics_card_is_suitable(available_graphics_cards[preferred_graphics_card_index.value()], surface))
				{
					// We are done: Use the graphics card which was specified by the user's command line argument.
					return available_graphics_cards[preferred_graphics_card_index.value()];
				}
				else
				{
					cout << "Error: The preferred graphics card with index " << preferred_graphics_card_index.value() << " is not suitable for this application!" << endl;
					cout << "The array index is valid, but this graphics card does not fulfill all requirements!" << endl;

					// We are NOT done!
					// Try to select the best graphics card automatically!
				}
			}
			else
			{
				// No, this array index for available_graphics_cards is invalid!
				cout << "Error: invalid command line argument! Graphics card array index " << preferred_graphics_card_index.value() << " is invalid!" << endl;

				// We are NOT done!
				// Try to select the best graphics card automatically!
			}
		}
		else
		{
			// Give the user a little hint message.
			cout << "No command line argument for preferred graphics card given." << endl;
			cout << "You have more than 1 graphics card available on your machine." << endl;
			cout << "Specify which one to use by passing -GPU <number> as command line argument." << endl;
			cout << "Please be aware that the first index is 0." << endl;
		}



		// Attempt 3
		// There are more than 1 graphics card available and the user did not specify which one to use.
		// If there are exactly 2 graphics card and one of them is VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU and the other one
		// is VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU, we should prefere the real graphics card over the integrated one.
		// We also need to check if the VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU one is suitable though!
		// If that is not the case, we check if the VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU is suitable and use it instead.
		// If both are unsuitable, there are no suitable graphics cards available on this machine!
		if(2 == number_of_available_graphics_cards)
		{
			


		}



		// ATTEMPT 4
		// - There is more than 1 graphics card available.
		// - The user did not specify which one to prefer.
		// - It's not like there are 2 GPUs, one of them a real graphics card and another one an integrated one.
		// We now have to sort our all GPUs that do not support swapchain and presentation.
		// After this we have to rank them by a score!
		
		// The suitable graphics cards (by array index).
		std::vector<std::size_t> suitable_graphics_cards;

		// Loop through all available graphics cards and sort out the unsuitable ones.
		for(std::size_t i=0; i<number_of_available_graphics_cards; i++)
		{
			if(decide_if_graphics_card_is_suitable(available_graphics_cards[i], surface))
			{
				cout << "Adding graphics card index " << i << " to the list of suitable graphics cards." << endl;

				// Add this graphics card to the list of suitable graphics cards.
				suitable_graphics_cards.push_back(i);
			}
			else
			{
				cout << "Sorting out graphics card index " << i << " because it is unsuitable for this application's purpose!" << endl;
			}
		}
		
		// How many graphics cards have been sorted out?
		auto how_many_graphics_card_disqualified = number_of_available_graphics_cards - suitable_graphics_cards.size();

		if(how_many_graphics_card_disqualified > 0)
		{
			cout << how_many_graphics_card_disqualified << " have been disqualified because they are unsuitable for the application's purpose!" << endl;
		}

		// We could not find any suitable graphics card!
		if(0 == suitable_graphics_cards.size())
		{
			cout << "Error: Could not find suitable graphics card automatically." << endl;
			return std::nullopt;
		}

		// Only 1 graphics card is suitable, let's choose that one.
		if(1 == suitable_graphics_cards.size())
		{
			cout << "There is only 1 suitable graphics card available." << endl;
			return available_graphics_cards[0];
		}

		// We have more than one suitable graphics card.
		// There is at least one graphics card that is suitable at least.

		// ----------------------------------------
		// TODO: Implement score ranking mechanism!
		// ----------------------------------------

		// For now, let's just use the first index of the suitable graphics cards.
		// This might be not the best choice after all, but it simplifies the situation for now.
		return available_graphics_cards[0];
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


	std::optional<VkPresentModeKHR> VulkanSettingsDecisionMaker::decide_which_presentation_mode_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		uint32_t number_of_available_present_modes = 0;

		// First check how many present modes are available for the selected combination of graphics card and window surface.
		VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(graphics_card, surface, &number_of_available_present_modes, nullptr);
		vulkan_error_check(result);

		if(0 == number_of_available_present_modes)
		{
			std::string error_message("Error: No presentation modes available!");
			display_error_message(error_message);
			return std::nullopt;
		}

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
				return VK_PRESENT_MODE_MAILBOX_KHR;
			}
		}

		cout << "Info: VK_PRESENT_MODE_MAILBOX_KHR is not supported by the regarded device." << endl;
		cout << "Let's see if VK_PRESENT_MODE_FIFO_KHR is supported." << endl;

		for(auto present_mode : available_present_modes)
		{
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
			// VK_PRESENT_MODE_FIFO_KHR specifies that the presentation engine waits for the next vertical blanking period to update the current image.
			// Tearing cannot be observed. An internal queue is used to hold pending presentation requests. New requests are appended to the end of the queue,
			// and one request is removed from the beginning of the queue and processed during each vertical blanking period in which the queue is non-empty.
			// This is the only value of presentMode that is required to be supported.
			if(VK_PRESENT_MODE_FIFO_KHR == present_mode)
			{
				return VK_PRESENT_MODE_FIFO_KHR;
			}
		}
		
		cout << "Info: VK_PRESENT_MODE_FIFO_KHR is not supported by the regarded device." << endl;

		// Lets try with any present mode available!
		if(available_present_modes.size() > 0)
		{
			// Let's just pick the first one.
			return available_present_modes[0];
		}

		cout << "Error: The selected graphics card does not support any presentation at all!" << endl;
		
		return std::nullopt;
	}


	void VulkanSettingsDecisionMaker::decide_width_and_height_of_swapchain_extent(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface, uint32_t& window_width, uint32_t& window_height, VkExtent2D& swapchain_extent)
	{
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
			// TODO: Refactor! Do it the way the Vulkan Tutorial is doing it.
			
			// If the surface size is defined, the swap chain size must match.
			swapchain_extent = surface_capabilities.currentExtent;
			window_width     = surface_capabilities.currentExtent.width;
			window_height    = surface_capabilities.currentExtent.height;
		}
	}

	
	std::optional<uint32_t> VulkanSettingsDecisionMaker::find_graphics_queue_family(const VkPhysicalDevice& graphics_card)
	{
		uint32_t number_of_available_queue_families = 0;

		// First check how many queue families are available.
		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

		cout << "There are " << number_of_available_queue_families << " queue families available." << endl;

		// Preallocate memory for the available queue families.
		std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);
		
		// Get information about the available queue families.
		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, available_queue_families.data());

		// Loop through all available queue families and look for a suitable one.
		for(std::size_t i=0; i<available_queue_families.size(); i++)
		{
			if(available_queue_families[i].queueCount > 0)
			{
				// Check if this queue family supports graphics.
				if(available_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					// Ok this queue family supports graphics!
					return static_cast<uint32_t>(i);
				}
			}
		}

		// In this case we could not find any suitable graphics queue family!
		return std::nullopt;
	}
			
			
	std::optional<uint32_t> VulkanSettingsDecisionMaker::find_presentation_queue_family(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		uint32_t number_of_available_queue_families = 0;

		// First check how many queue families are available.
		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

		cout << "There are " << number_of_available_queue_families << " queue families available." << endl;

		// Preallocate memory for the available queue families.
		std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);
		
		// Get information about the available queue families.
		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, available_queue_families.data());


		// Loop through all available queue families and look for a suitable one.
		for(std::size_t i=0; i<available_queue_families.size(); i++)
		{			
			if(available_queue_families[i].queueCount > 0)
			{
				uint32_t this_queue_family_index = static_cast<uint32_t>(i);

				VkBool32 presentation_available = false;

				// Query if presentation is supported.
				VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, this_queue_family_index, surface, &presentation_available);
				vulkan_error_check(result);
				
				if(presentation_available)
				{
					return this_queue_family_index;
				}
			}
		}
		
		// In this case we could not find any suitable present queue family!
		return std::nullopt;
	}


	std::optional<uint32_t> VulkanSettingsDecisionMaker::find_distinct_data_transfer_queue_family(const VkPhysicalDevice& graphics_card)
	{
		uint32_t number_of_available_queue_families = 0;

		// First check how many queue families are available.
		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

		cout << "There are " << number_of_available_queue_families << " queue families available." << endl;

		// Preallocate memory for the available queue families.
		std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);
		
		// Get information about the available queue families.
		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, available_queue_families.data());


		// Loop through all available queue families and look for a suitable one.
		for(std::size_t i=0; i<available_queue_families.size(); i++)
		{			
			if(available_queue_families[i].queueCount > 0)
			{
				if(!(available_queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
				{
					if(available_queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
					{
						uint32_t this_queue_family_index = static_cast<uint32_t>(i);
						return this_queue_family_index;
					}
				}
			}
		}

		// In this case we could not find any distinct transfer queue family!
		return std::nullopt;
	}


	std::optional<uint32_t> VulkanSettingsDecisionMaker::find_any_data_transfer_queue_family(const VkPhysicalDevice& graphics_card)
	{
		uint32_t number_of_available_queue_families = 0;

		// First check how many queue families are available.
		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, nullptr);

		cout << "There are " << number_of_available_queue_families << " queue families available." << endl;

		// Preallocate memory for the available queue families.
		std::vector<VkQueueFamilyProperties> available_queue_families(number_of_available_queue_families);
		
		// Get information about the available queue families.
		vkGetPhysicalDeviceQueueFamilyProperties(graphics_card, &number_of_available_queue_families, available_queue_families.data());


		// Loop through all available queue families and look for a suitable one.
		for(std::size_t i=0; i<available_queue_families.size(); i++)
		{			
			if(available_queue_families[i].queueCount > 0)
			{
				// All we care about is VK_QUEUE_TRANSFER_BIT.
				// It is very likely that this queue family has VK_QUEUE_GRAPHICS_BIT as well!
				if(available_queue_families[i].queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					uint32_t this_queue_family_index = static_cast<uint32_t>(i);
					return this_queue_family_index;
				}
			}
		}

		// In this case we could not find any suitable transfer queue family at all!
		// Data transfer from CPU to GPU is not possible in this case!
		return std::nullopt;
	}


};
};
