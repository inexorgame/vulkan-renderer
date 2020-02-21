#pragma once

#include "../error-handling/VulkanErrorHandling.hpp"

#include <string>
#include <vector>
#include <optional>


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanAvailabilityChecks
	/// @brief This class bundles various availability checks.
	/// @note In Vulkan we always need to check if a feature or a setting that we want to use is available in the current system.
	/// Since Vulkan API is designed to work on various platforms, we must take into account the platform's different capabilities!
	class VulkanAvailabilityChecks
	{
		public:

			VulkanAvailabilityChecks();

			~VulkanAvailabilityChecks();


		protected:


			/// @brief Checks if a certain Vulkan instance layer is available on the system.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateInstanceLayerProperties.html
			/// @note Available instance layers can then be enabled by passing them as parameter during Vulkan instance creation.
			bool check_instance_layer_availability(const std::string& instance_layer_name);
			
			
			/// @brief Checks if a certain Vulkan instance extension is available on the system.
			/// Available instance extensions can then be enabled by passing them as parameter during Vulkan instance creation.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateInstanceExtensionProperties.html
			/// @param instance_extension_name The name of the Vulkan instance extension.
			/// @return true if the instance extension is available, false otherwise.
			bool check_instance_extension_availability(const std::string& instance_extension_name);


			/// @brief Checks if a certain Vulkan device layer is available on the system.
			/// Available device layers can then be enabled by passing them as a parameter during Vulkan device creation.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateDeviceLayerProperties.html
			/// @note Device layers and device extensions are coupled to a certain graphics card which needs to be specified as parameter.
			/// @param graphics_card The selected graphics card.
			/// @return true if the device layer is available, false otherwise.
			bool check_device_layer_availability(const VkPhysicalDevice& graphics_card, const std::string& device_layer_name);
			

			/// @brief Checks if a certain Vulkan device extension is available on the system.
			/// Available device extensions can then be enabled by passing them as a parameter during Vulkan device creation.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateDeviceExtensionProperties.html
			/// @note Device layers and device extensions are coupled to a certain graphics card which needs to be specified as parameter.
			/// @param graphics_card The selected graphics card.
			/// @return true if the device extension is available, false otherwise.
			bool check_device_extension_availability(const VkPhysicalDevice& graphics_card, const std::string& device_extension_name);


			/// @brief Query if presentation is available for a certain combination of graphics card and surface.
			/// The present mode describes how the rendered image will be presented on the screen.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
			/// @param graphics_card The selected graphics card.
			/// @param surface The window surface.
			/// @return true if presentation is available, false otherwise.
			bool check_presentation_availability(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);
			

			/// @brief Checks if swapchain is available for a certain graphics card.
			/// @param graphics_card The selected graphics card.
			/// @return true if swapchain is available, false otherwise.
			bool check_swapchain_availability(const VkPhysicalDevice& graphics_card);


			/// @brief Checks if there is a queue family (index) which can be used for both graphics and presentation.
			/// @return The queue family index which can be used for both graphics and presentation (if existent), std::nullopt otherwise.
			std::optional<uint32_t> find_queue_family_for_both_graphics_and_presentation(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);


	};

};
};
