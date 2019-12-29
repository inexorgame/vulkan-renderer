#pragma once

#include <vulkan/vulkan.h>


namespace inexor {
namespace vulkan_renderer {

	
	// This class makes automatic decisions on e.g. the following topics:
	// If more than 1 graphics card is available, which one should be used?
	// Which one of the available surface color format should be used?
	// Which one of the available graphics card's queue families should be used?
	// Which one of the available presentation modes should be used?
	// ...
	class VulkanDecisionMaker
	{
		public:
			
			VulkanDecisionMaker();

			~VulkanDecisionMaker();

		protected:

			// TODO: Implement!
			void decide_which_graphics_card_queue_family_to_use();
			
			// TODO: Implement!
			void decide_how_many_images_in_swap_chain_to_use();
			
			// TODO: Implement!
			void decide_which_surface_color_format_for_swap_chain_images_to_use();

			// TODO: Implement!
			void decide_size_of_swap_chain_images();

			// TODO: Implement!
			void decide_which_pre_transformation_to_use();

			// TODO: Implement!
			void decide_which_presentation_mode_to_use();
			
	};

};
};
