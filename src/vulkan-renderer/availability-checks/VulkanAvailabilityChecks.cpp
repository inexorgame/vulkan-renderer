#include "VulkanAvailabilityChecks.hpp"


namespace inexor {
namespace vulkan_renderer {


	VulkanAvailabilityChecks::VulkanAvailabilityChecks()
	{
	}


	VulkanAvailabilityChecks::~VulkanAvailabilityChecks()
	{
	}


	bool VulkanAvailabilityChecks::is_instance_extension_available(const std::string& instance_extension_name)
	{
		assert(instance_extension_name.length()>0);

		uint32_t number_of_available_instance_extensions = 0;

		// First ask Vulkan how many instance extensions are available on the system.
		VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &number_of_available_instance_extensions, NULL);
		vulkan_error_check(result);

		if(0 == number_of_available_instance_extensions)
		{	
			// It should be a very rare case that no instance extensions are available at all. Still we have to consider this!
			display_error_message("Error: No Vulkan instance extensions available!");

			// Since there are no instance extensions available at all, the desired one is not supported either.
			return false;
		}
		else
		{
			// Preallocate memory for extension properties.
			std::vector<VkExtensionProperties> instance_extensions(number_of_available_instance_extensions);

			// Get the information about the available instance extensions.
			result = vkEnumerateInstanceExtensionProperties(NULL, &number_of_available_instance_extensions, instance_extensions.data());
			vulkan_error_check(result);

			// Loop through all available instance extensions and search for the requested one.
			for(const VkExtensionProperties& instance_extension : instance_extensions)
			{
				// Compare the name of the current instance extension with the requested one.
				if(0 == strcmp(instance_extension.extensionName, instance_extension_name.c_str()))
				{
					// Yes, this instance extension is supported!
					return true;
				}
			}
		}
		
		// No, this instance extension could not be found and thus is not supported!
		return false;
	}

	
	bool VulkanAvailabilityChecks::is_instance_layer_available(const std::string& instance_layer_name)
	{
		assert(instance_layer_name.length()>0);

		uint32_t number_of_available_instance_layers = 0;

		// First ask Vulkan how many instance layers are available on the system.
		VkResult result = vkEnumerateInstanceLayerProperties(&number_of_available_instance_layers, nullptr);
		vulkan_error_check(result);

		if(0 == number_of_available_instance_layers)
		{
			// It should be a very rare case that no instance layers are available at all. Still we have to consider this!
			display_error_message("Error: No Vulkan instance layers available!");

			// Since there are no instance layers available at all, the desired one is not supported either.
			return false;
		}
		else
		{
			// Preallocate memory for layer properties.
			std::vector<VkLayerProperties> instance_layers(number_of_available_instance_layers);
			
			// Get the information about the available instance layers.
			result = vkEnumerateInstanceLayerProperties(&number_of_available_instance_layers, instance_layers.data());
			vulkan_error_check(result);
		
			// Loop through all available instance layers and search for the requested one.
			for(const VkLayerProperties& instance_layer : instance_layers)
			{
				// Compare the name of the current instance extension with the requested one.
				if(0 == strcmp(instance_layer.layerName, instance_layer_name.c_str()))
				{
					// Yes, this instance extension is supported!
					return true;
				}
			}
		}

		// No, this instance layer could not be found and thus is not supported!
		return false;
	}
	
	
	bool VulkanAvailabilityChecks::is_device_layer_available(const VkPhysicalDevice& graphics_card, const std::string& device_layer_name)
	{
		assert(graphics_card);
		assert(device_layer_name.length()>0);

		uint32_t number_of_available_device_layers = 0;
		
		// First ask Vulkan how many device layers are available on the system.
		VkResult result = vkEnumerateDeviceLayerProperties(graphics_card, &number_of_available_device_layers, nullptr);
		vulkan_error_check(result);

		if(0 == number_of_available_device_layers)
		{
			// It should be a very rare case that no device layers are available at all. Still we have to consider this!
			display_error_message("Error: No Vulkan device layers available!");

			// Since there are no device layers available at all, the desired one is not supported either.
			return false;
		}
		else
		{
			// Preallocate memory for device layers.
			std::vector<VkLayerProperties> device_layer_properties(number_of_available_device_layers);
			
			// Get the information about the available device layers.
			result = vkEnumerateDeviceLayerProperties(graphics_card, &number_of_available_device_layers, device_layer_properties.data());
			vulkan_error_check(result);
		
			// Loop through all available device layers and search for the requested one.
			for(const VkLayerProperties& device_layer : device_layer_properties)
			{
				if(0 == strcmp(device_layer.layerName, device_layer_name.c_str()))
				{
					// Yes, this device layer is supported!
					return true;
				}
			}
		}

		// No, this device layer could not be found and thus is not supported!
		return false;
	}


	bool VulkanAvailabilityChecks::is_device_extension_available(const VkPhysicalDevice& graphics_card, const std::string& device_extension_name)
	{
		assert(graphics_card);
		assert(device_extension_name.length()>0);

		uint32_t number_of_available_device_extensions = 0;
		
		// First ask Vulkan how many device extensions are available on the system.
		VkResult result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_available_device_extensions, nullptr);
		vulkan_error_check(result);

		if(0 == number_of_available_device_extensions)
		{
			// It should be a very rare case that no device extension are available at all. Still we have to consider this!
			display_error_message("Error: No Vulkan device extensions available!");

			// Since there are no device extensions available at all, the desired one is not supported either.
			return false;
		}
		else
		{
			// Preallocate memory for device extensions.
			std::vector<VkExtensionProperties> device_extensions(number_of_available_device_extensions);
			
			// Get the information about the available device extensions.
			result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_available_device_extensions, device_extensions.data());
			vulkan_error_check(result);

			// Loop through all available device extensions and search for the requested one.
			for(const VkExtensionProperties& device_extension : device_extensions)
			{
				if(0 == strcmp(device_extension.extensionName, device_extension_name.c_str()))
				{
					// Yes, this device extension is supported!
					return true;
				}
			}
		}

		// No, this device extension could not be found and thus is not supported!
		return false;
	}


	bool VulkanAvailabilityChecks::is_presentation_available(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		assert(graphics_card);
		assert(surface);

		VkBool32 presentation_available = false;
		
		// Query if presentation is supported.
		VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, 0, surface, &presentation_available);

		// I don't know what the value of presentation_available might be in case of an error, so handle it explicitely.
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return false;
		}

		return presentation_available;
	}


	bool VulkanAvailabilityChecks::is_swapchain_available(const VkPhysicalDevice& graphics_card)
	{
		assert(graphics_card);

		// Just overload the other method. VK_KHR_SWAPCHAIN_EXTENSION_NAME is a nice macro which Vulkan defines for us.
		return is_device_extension_available(graphics_card, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}


};
};
