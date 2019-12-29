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

		// Search for the requested instance extension.
		if(std::find(instance_extensions.begin(), instance_extensions.end(), instance_extension_name) != instance_extensions.end())
		{
			// Yes, this instance extension is supported!
			return true;
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
		
		// Search for the requested instance layer.
		if(std::find(instance_layer_properties.begin(), instance_layer_properties.end(), instance_layer_name) != instance_layer_properties.end())
		{
			// Yes, this instance layer is supported!
			return true;
		}

		// No, this instance layer could not be found and thus is not supported!
		return false;
	}


};
};
