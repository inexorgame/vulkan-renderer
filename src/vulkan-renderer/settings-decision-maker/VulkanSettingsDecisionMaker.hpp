#pragma once

#include "../error-handling/VulkanErrorHandling.hpp"

#include <vector>

#include <vulkan/vulkan.h>


namespace inexor {
namespace vulkan_renderer {

	
	// This class makes automatic decisions on e.g. the following topics:
	// If more than 1 graphics card is available, which one should be used?
	// Which one of the available surface color format should be used?
	// Which one of the available graphics card's queue families should be used?
	// Which one of the available presentation modes should be used?
	// ...
	class VulkanSettingsDecisionMaker
	{
		public:
			
			// 
			VulkanSettingsDecisionMaker();

			// 
			~VulkanSettingsDecisionMaker();


		protected:

			// Option A: The graphics card can be chosen by the user manually.
			// Option B: The "best" graphics card will be determined by the engine automatically.
			VkPhysicalDevice decide_which_graphics_card_to_use(const VkInstance& vulkan_instance);

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

			// TODO: Implement!
			VkPresentModeKHR decide_which_presentation_mode_to_use();
			
		
			//decide_which_image_color_space_to_use();
			//decide_which_image_sharing_mode_to_use();

	};

};
};
