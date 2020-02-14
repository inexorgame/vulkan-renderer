#pragma once

#include <string>
#include <vector>

#include "../error-handling/VulkanErrorHandling.hpp"


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanAvailabilityChecks
	/// @brief In Vulkan we always need to check if the features that we will be using is available on the current system.
	/// This class bundles various availability checks.
	class VulkanAvailabilityChecks
	{
		public:

			VulkanAvailabilityChecks();

			~VulkanAvailabilityChecks();


		protected:
		
			// Checks if a certain instance layer is available on the system.
			// Available instance layers can then be enabled by passing them as parameter for Vulkan instance creation.
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateInstanceLayerProperties.html
			bool check_instance_layer_availability(const std::string& instance_layer_name);
			
			
			// Checks if a certain instance extension is available on the system.
			// Available instance extensions can then be enabled by passing them as parameter for Vulkan instance creation.
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateInstanceExtensionProperties.html
			bool check_instance_extension_availability(const std::string& instance_extension_name);


			// Checks if a certain device layer is available on the system.
			// Available device layers can then be enabled by passing them as a parameter for Vulkan device creation.
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateDeviceLayerProperties.html
			bool check_device_layer_availability(const VkPhysicalDevice& graphics_card, const std::string& device_layer_name);
			

			// Checks if a certain device extension is available on the system.
			// Available device extensions can then be enabled by passing them as a parameter for Vulkan device creation.
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateDeviceExtensionProperties.html
			bool check_device_extension_availability(const VkPhysicalDevice& graphics_card, const std::string& device_extension_name);


			// Query if presentation is available on the system.
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
			bool check_presentation_availability(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);
			

			// Checks if swapchain is available on the system.
			bool check_swapchain_availability(const VkPhysicalDevice& graphics_card);

	};

};
};
