#pragma once

#include "../error-handling/VulkanErrorHandling.hpp"

#include <vector>
#include <iostream>
using namespace std;

#include <vulkan/vulkan.h>


namespace inexor {
namespace vulkan_renderer {

	
	// This class makes automatic decisions on e.g. the following topics:
	// Which graphics card will be used if more than 1 is available?
	// Which surface color format should be used?
	// Which graphics card's queue families should be used?
	// Which presentation modes should be used?
	// The functions also verify that every selected option is available on the current system.
	class VulkanSettingsDecisionMaker
	{
		public:
			
			VulkanSettingsDecisionMaker();

			~VulkanSettingsDecisionMaker();


		protected:

			// Check which graphics card will be used in case multiple are available.
			// If there is only one graphics card available, we must use it without a choice.
			// The user can specify which graphics card will be used manually by passing a command line argument.
			// A graphics card is only suitable if it supports the following list of features:
			// ---------------------------------------------------------------------------------------------------------
			// 1) One queue family that supports both graphics and present or 2 queue families for graphics and present.
			// 2) Present mode VK_PRESENT_MODE_MAILBOX_KHR or VK_PRESENT_MODE_FIFO_KHR.
			// 3) Support of double buffering, even better triple buffering.
			// TODO: Add more criteria if neccesary!
			VkPhysicalDevice decide_which_graphics_card_to_use(const VkInstance& vulkan_instance, const int& preferred_graphics_card_index = -1);



			// This one is a little tricky!
			// TODO: Detailed explanation!
			
			// TODO: Implement!
			uint32_t decide_which_graphics_card_queue_family_index_to_use(const VkPhysicalDevice& graphics_card);

			// TODO: Implement!
			uint32_t decide_which_graphics_card_queue_family_index_to_use();
			



			// TODO: Implement!
			uint32_t decide_how_many_images_in_swap_chain_to_use();
			
			// The selected image format must be supported by the system!
			// https://vulkan.lunarg.com/doc/view/latest/windows/vkspec.html#formats-compatibility
			VkFormat decide_which_surface_color_format_for_swap_chain_images_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& vulkan_surface);

			// TODO: Implement!
			uint32_t decide_size_of_swap_chain_images();

			// TODO: Implement!
			void decide_which_pre_transformation_to_use();



			// Decides which presentation mode the presentation engine will be using. We can only use presentation modes which are
			// available in the current system. The preferred presentation mode is VK_PRESENT_MODE_MAILBOX_KHR.
			// Just checking whether swap extension is supported is not enough because presentation support is a queue family property!
			// A physical device may support swap chains, but that doesn't mean that all its queue families also support it.
			// https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkPresentModeKHR.html
			VkPresentModeKHR decide_which_presentation_mode_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);
			
		
			//decide_which_image_color_space_to_use();
			//decide_which_image_sharing_mode_to_use();

	};

};
};
