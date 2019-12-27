#pragma once

#include <iostream>
#include <vector>
using namespace std;

#include <vulkan/vulkan.h>


namespace inexor {
namespace vulkan_renderer {


	// This helper class prints information about a specific graphics card to the console.
	class VulkanGraphicsCardInfoViewer
	{
		protected:

			// 
			void print_physical_device_queue_families(const VkPhysicalDevice& graphics_card);

			// 
			void print_instance_layer_properties();

			// 
			void print_instance_extensions();

			// 
			void print_device_layers(const VkPhysicalDevice& graphics_card);

			// 
			void print_surface_capabilities(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);

			// 
			void print_supported_surface_formats(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);

			// 
			void print_presentation_modes(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);

			// Gets the information on the graphics card and prints it to the console.
			void print_graphics_card_info(const VkPhysicalDevice& graphics_card);
			
			// 
			void print_driver_vulkan_version();

		public:

			// 
			VulkanGraphicsCardInfoViewer();

			// 
			~VulkanGraphicsCardInfoViewer();

	};

};
};
