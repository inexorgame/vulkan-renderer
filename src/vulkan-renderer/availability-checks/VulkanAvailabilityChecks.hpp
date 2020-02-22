#pragma once

#include "../error-handling/VulkanErrorHandling.hpp"

#include <string>
#include <vector>
#include <optional>
#include <assert.h>


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanAvailabilityChecks
	/// @brief This class bundles various availability checks.
	/// @note In Vulkan we always need to check if a feature or a setting that we want to use is available in the current system.
	/// Since Vulkan API is designed to work on various platforms, we must take into account the platform's different capabilities!
	/// @todo Remove display_error_message dependency? Might use std::cout so this class can be used by other programmers.
	class VulkanAvailabilityChecks
	{
		public:

			VulkanAvailabilityChecks();

			~VulkanAvailabilityChecks();


		protected:


			/// @brief Checks if a certain Vulkan instance layer is available on the system.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateInstanceLayerProperties.html
			/// @param instance_layer_name The name of the Vulkan instance layer.
			/// @return true if the Vulkan instance layer is available, false otherwise.
			/// @note Available instance layers can be enabled by passing them as parameter during Vulkan instance creation.
			bool is_instance_layer_available(const std::string& instance_layer_name);
			
			
			/// @brief Checks if a certain Vulkan instance extension is available on the system.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateInstanceExtensionProperties.html
			/// @param instance_extension_name The name of the Vulkan instance extension.
			/// @return true if the Vulkan instance extension is available, false otherwise.
			/// @note Available instance extensions can be enabled by passing them as parameter during Vulkan instance creation.
			bool is_instance_extension_available(const std::string& instance_extension_name);


			/// @brief Checks if a certain Vulkan device layer is available on the system.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateDeviceLayerProperties.html
			/// @note Device layers and device extensions are coupled to a certain graphics card which needs to be specified as parameter.
			/// @param graphics_card The selected graphics card.
			/// @param device_layer_name The name of the Vulkan device layer.
			/// @return true if the Vulkan device layer is available, false otherwise.
			/// @note Available device layers can be enabled by passing them as a parameter during Vulkan device creation.
			bool is_device_layer_available(const VkPhysicalDevice& graphics_card, const std::string& device_layer_name);
			

			/// @brief Checks if a certain Vulkan device extension is available on the system.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkEnumerateDeviceExtensionProperties.html
			/// @param graphics_card The selected graphics card.
			/// @param device_extension_name The name of the Vulkan device extension.
			/// @return true if the Vulkan device extension is available, false otherwise.
			/// @note Available device extensions can be enabled by passing them as a parameter during Vulkan device creation.
			/// @note Device layers and device extensions are coupled to a certain graphics card which needs to be specified as parameter.
			bool is_device_extension_available(const VkPhysicalDevice& graphics_card, const std::string& device_extension_name);


			/// @brief Checks if presentation is available for a certain combination of graphics card and window surface.
			/// The present mode describes how the rendered image will be presented on the screen.
			/// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/vkGetPhysicalDeviceSurfaceSupportKHR.html
			/// @param graphics_card The selected graphics card.
			/// @param surface The window surface.
			/// @return true if presentation is available, false otherwise.
			bool is_presentation_available(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);
			

			/// @brief Checks if swapchain is available for a certain graphics card.
			/// @param graphics_card The selected graphics card.
			/// @return true if swapchain is available, false otherwise.
			bool is_swapchain_available(const VkPhysicalDevice& graphics_card);


	};

};
};
