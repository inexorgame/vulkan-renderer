#pragma once

#include <vector>
#include <unordered_map>
#include <cassert>

#include <spdlog/spdlog.h>

#include <vulkan/vulkan.h>

#include "vulkan-renderer/error-handling/error_handling.hpp"
#include "vulkan-renderer/helpers/surface_formats.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	/// @class VulkanGraphicsCardInfoViewer
	/// @brief Prints information related to graphics card's capabilities and limits.
	class VulkanGraphicsCardInfoViewer
	{
		public:

			VulkanGraphicsCardInfoViewer() = default;

			~VulkanGraphicsCardInfoViewer() = default;


			// Uses vkEnumerateInstanceVersion to query which version of the Vulkan API is supported on this system.
			// https://vulkan.lunarg.com/doc/view/latest/windows/vkspec.html#vkEnumerateInstanceVersion
			void print_driver_vulkan_version();
			
			
			// Prints information about device queue families.
			// @param graphics_card The regarded graphics card.
			void print_physical_device_queue_families(const VkPhysicalDevice& graphics_card);


			// Prints which instance layers are available on this system.
			void print_instance_layers();


			// Prints which instance extensions are available on this system.
			void print_instance_extensions();


			// Prints which device layers are available for the regarded graphics card on this system.
			// @param graphics_card The regarded graphics card.
			void print_device_layers(const VkPhysicalDevice& graphics_card);


			// Prints which device extensions are available for the regarded graphics card on this system.
			// @param graphics_card The regarded graphics card.
			void print_device_extensions(const VkPhysicalDevice& graphics_card);

			
			// Prints supported surface capabilities of the regarded combination of graphics card and Vulkan (window) surface.
			// @param graphics_card The regarded graphics card.
			// @param vulkan_surface The regarded Vulkan (window) surface.
			void print_surface_capabilities(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);


			// Prints supported surface formats of the regarded combination of graphics card and Vulkan (window) surface.
			// @param graphics_card The regarded graphics card.
			// @param vulkan_surface The regarded Vulkan (window) surface.
			void print_supported_surface_formats(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);


			// Lists up all supported presentation modes.
			// @param graphics_card The regarded graphics card.
			// @param vulkan_surface The regarded Vulkan (window) surface.
			void print_presentation_modes(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);


			// Gets the information on the graphics card and prints it to the console.
			// @param graphics_card The regarded graphics card.
			void print_graphics_card_info(const VkPhysicalDevice& graphics_card);


			// Prints information about the limits of the graphics card.
			// @param graphics_card The regarded graphics card.
			void print_graphics_card_limits(const VkPhysicalDevice& graphics_card);


			// Prints information about the sparse properties of the graphics card.
			// @param graphics_card The regarded graphics card.
			void print_graphics_cards_sparse_properties(const VkPhysicalDevice& graphics_card);
			
			
			// Prints information about the features of the graphics card.
			// @param graphics_card The regarded graphics card.
			void print_graphics_card_features(const VkPhysicalDevice& graphics_card);


			// Prints information about the graphics card's memory properties.
			// @param graphics_card The regarded graphics card.
			void print_graphics_card_memory_properties(const VkPhysicalDevice& graphics_card);


			// Lists up all available physical devices.
			// @param vulkan_instance The instance of Vulkan.
			// @param vulkan_surface The regarded Vulkan (window) surface.
			void print_all_physical_devices(const VkInstance& vulkan_instance, const VkSurfaceKHR& vulkan_surface);

	};

};
};
