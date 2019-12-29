#pragma once

#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

#include <vulkan/vulkan.h>

#include "../error-handling/VulkanErrorHandling.hpp"


namespace inexor {
namespace vulkan_renderer {


	// Prints information related to graphics card's capabilities and limits to the console.
	class VulkanGraphicsCardInfoViewer
	{
		public:

			// 
			VulkanGraphicsCardInfoViewer();

			// 
			~VulkanGraphicsCardInfoViewer();

		protected:

			// Uses vkEnumerateInstanceVersion to query which version of the Vulkan API is supported on this system.
			// https://vulkan.lunarg.com/doc/view/latest/windows/vkspec.html#vkEnumerateInstanceVersion
			void print_driver_vulkan_version();
			
			// 
			void print_physical_device_queue_families(const VkPhysicalDevice& graphics_card);

			// 
			void print_instance_layers();

			// 
			void print_instance_extensions();

			// 
			void print_device_layers(const VkPhysicalDevice& graphics_card);

			// 
			void print_device_extensions(const VkPhysicalDevice& graphics_card);

			// 
			void print_surface_capabilities(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);

			// 
			void print_supported_surface_formats(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);

			// 
			void print_presentation_modes(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);

			// Gets the information on the graphics card and prints it to the console.
			void print_graphics_card_info(const VkPhysicalDevice& graphics_card);

	};

};
};
