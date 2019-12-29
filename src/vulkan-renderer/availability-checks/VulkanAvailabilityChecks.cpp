#include "VulkanAvailabilityChecks.hpp"


namespace inexor {
namespace vulkan_renderer {


	VulkanAvailabilityChecks::VulkanAvailabilityChecks()
	{
	}


	VulkanAvailabilityChecks::~VulkanAvailabilityChecks()
	{
	}


	bool VulkanAvailabilityChecks::CheckInstanceExtensionSupport(const std::string& instance_extension_name)
	{
		uint32_t number_of_extensions = 0;
		VkResult result = vkEnumerateInstanceExtensionProperties(NULL, &number_of_extensions, NULL);
		vulkan_error_check(result);

		// Preallocate memory for extension properties.
		std::vector<VkExtensionProperties> instance_extensions(number_of_extensions);
		result = vkEnumerateInstanceExtensionProperties(NULL, &number_of_extensions, instance_extensions.data());
		vulkan_error_check(result);

		// Loop through all available instance extensions and search for the requested one.
		for(VkExtensionProperties extension : instance_extensions)
		{
			// Compare the name of the current instance extension with the requested one.
			if(0 == strcmp(extension.extensionName, instance_extension_name.c_str()))
			{
				// Yes, this instance extension is supported!
				return true;
			}
		}
		
		// No, this instance extension could not be found and thus is not supported!
		return false;
	}

	
	bool VulkanAvailabilityChecks::CheckInstanceLayerSupport(const std::string& instance_layer_name)
	{
		uint32_t number_of_instance_layers = 0;
		vkEnumerateInstanceLayerProperties(&number_of_instance_layers, nullptr);

		// Preallocate memory for layer properties.
		std::vector<VkLayerProperties> instance_layer_properties(number_of_instance_layers);
		vkEnumerateInstanceLayerProperties(&number_of_instance_layers, instance_layer_properties.data());
		
		// Loop through all available instance layers and search for the requested one.
		for(VkLayerProperties layer : instance_layer_properties)
		{
			if(0 == strcmp(layer.layerName, instance_layer_name.c_str()))
			{
				// Yes, this instance layer is supported!
				return true;
			}
		}
		
		// No, this instance layer could not be found and thus is not supported!
		return false;
	}


};
};
