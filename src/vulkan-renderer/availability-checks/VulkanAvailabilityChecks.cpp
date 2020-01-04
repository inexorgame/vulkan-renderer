#include "VulkanAvailabilityChecks.hpp"


namespace inexor {
namespace vulkan_renderer {


	VulkanAvailabilityChecks::VulkanAvailabilityChecks()
	{
	}


	VulkanAvailabilityChecks::~VulkanAvailabilityChecks()
	{
	}


	bool VulkanAvailabilityChecks::check_instance_extension_availability(const std::string& instance_extension_name)
	{
		uint32_t number_of_instance_extensions = 0;

		VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &number_of_instance_extensions, NULL);
		vulkan_error_check(result);

		// Preallocate memory for extension properties.
		std::vector<VkExtensionProperties> instance_extensions(number_of_instance_extensions);
		result = vkEnumerateInstanceExtensionProperties(NULL, &number_of_instance_extensions, instance_extensions.data());
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
		
		// No, this instance extension could not be found and thus is not supported!
		return false;
	}

	
	bool VulkanAvailabilityChecks::check_instance_layer_availability(const std::string& instance_layer_name)
	{
		uint32_t number_of_instance_layers = 0;

		VkResult result = vkEnumerateInstanceLayerProperties(&number_of_instance_layers, nullptr);
		vulkan_error_check(result);

		// Preallocate memory for layer properties.
		std::vector<VkLayerProperties> instance_layers(number_of_instance_layers);
		result = vkEnumerateInstanceLayerProperties(&number_of_instance_layers, instance_layers.data());
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

		// No, this instance layer could not be found and thus is not supported!
		return false;
	}
	
	
	bool VulkanAvailabilityChecks::check_device_layer_availability(const VkPhysicalDevice& graphics_card, const std::string& device_layer_name)
	{
		uint32_t number_of_device_layers = 0;

		VkResult result = vkEnumerateDeviceLayerProperties(graphics_card, &number_of_device_layers, nullptr);
		vulkan_error_check(result);

		// Preallocate memory for device layers.
		std::vector<VkLayerProperties> device_layer_properties(number_of_device_layers);
		result = vkEnumerateDeviceLayerProperties(graphics_card, &number_of_device_layers, device_layer_properties.data());
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

		// No, this device layer could not be found and thus is not supported!
		return false;
	}


	bool VulkanAvailabilityChecks::check_device_extension_availability(const VkPhysicalDevice& graphics_card, const std::string& device_extension_name)
	{
		uint32_t number_of_device_extensions = 0;

		VkResult result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_device_extensions, nullptr);
		vulkan_error_check(result);

		// Preallocate memory for device extensions.
		std::vector<VkExtensionProperties> device_extensions(number_of_device_extensions);
		result = vkEnumerateDeviceExtensionProperties(graphics_card, nullptr, &number_of_device_extensions, device_extensions.data());
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

		// No, this device extension could not be found and thus is not supported!
		return false;
	}


	bool VulkanAvailabilityChecks::check_presentation_availability(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface)
	{
		VkBool32 presentation_available = false;
		
		// Query if presentation is supported.
		// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
		VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(graphics_card, 0, surface, &presentation_available);
		vulkan_error_check(result);

		return presentation_available;
	}


	bool VulkanAvailabilityChecks::check_swapchain_availability(const VkPhysicalDevice& graphics_card)
	{
		return check_device_extension_availability(graphics_card, VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

};
};
