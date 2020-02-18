#pragma once

#include "../error-handling/VulkanErrorHandling.hpp"

#include <vector>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <optional>

#include <vulkan/vulkan.h>


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanSettingsDecisionMaker
	/// @brief This class makes automatic decisions which are relevant to setting up Vulkan:
	/// - Which graphics card will be used if more than 1 is available?
	/// - Which surface color format should be used?
	/// - Which graphics card's queue families should be used?
	/// - Which presentation modes should be used?
	class VulkanSettingsDecisionMaker
	{
		public:
			
			VulkanSettingsDecisionMaker();

			~VulkanSettingsDecisionMaker();


		protected:
			

			// TODO: Make every function call by reference and use VkResult for type.
			// TODO: Turn into std::optional values!

			// TODO
			//uint32_t decide_which_graphics_card_queue_family_index_to_use(const VkPhysicalDevice& graphics_card);
			//uint32_t decide_which_graphics_card_queue_family_index_to_use();
			//uint32_t decide_size_of_swapchain_images();
			//void decide_which_pre_transformation_to_use();
			//decide_which_image_color_space_to_use();
			//decide_which_image_sharing_mode_to_use();

			
			/// @brief Automatically decides if a graphics cardis suitable for this application's purpose!
			/// @note Add more checks to the validation mechanism if neccesary, e.h. check for geometry shader support.
			/// @param graphics_card The graphics card which will be checked for suitability.
			/// @return True if the graphics card is suitable, false otherwise.
			VkBool32 decide_if_graphics_card_is_suitable(const VkPhysicalDevice& graphics_card);


			/// @brief Automatically selects the best graphics card in case multiple are available.
			/// @note If there is only one graphics card available, we can't choose and must use it obviously.
			/// @note The user can manually specify which graphics card will be used by passing a command line argument.
			/// @note A graphics card is only suitable if it supports the following list of features:
			/// - One queue family that supports both graphics and present or 2 queue families for graphics and present.
			/// - Present mode VK_PRESENT_MODE_MAILBOX_KHR or VK_PRESENT_MODE_FIFO_KHR.
			/// - Support of double buffering, even better triple buffering.
			/// @param vulkan_instance A pointer to the Vulkan instance handle.
			/// @param preferred_graphics_card_index The preferred graphics card (by array index).
			/// @return The physical device which was chosen to be the best one.
			std::optional<VkPhysicalDevice> decide_which_graphics_card_to_use(const VkInstance& vulkan_instance, const uint32_t& preferred_graphics_card_index = UINT32_MAX);

			
			/// @brief Automatically decides how many images will be used in the swap chain.
			/// @param graphics_card The selected graphics card.
			/// @param surface The selected (window) surface.
			/// @return The number of images that will be used in swap chain.
			uint32_t decide_how_many_images_in_swapchain_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);
			

			/// @brief Automatically decides whcih surface color to use in swapchain.
			/// @param graphics_card The selected graphics card.
			/// @param surface The window surface.
			/// @param color_format The chosen color format.
			/// @param color_space The chosen color space.
			std::optional<VkSurfaceFormatKHR> decide_which_surface_color_format_in_swapchain_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);


			/// @brief Automatically decides which surface color format to use in swapchain.
			/// @param graphics_card The selected graphics card.
			/// @param surface The window surface.
			/// @param window_width The width of the window.
			/// @param window_height The height of the window.
			void decide_which_surface_color_format_in_swapchain_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface, uint32_t& image_width, uint32_t& image_height);


			/// @brief Automatically decides which width and height to use as swapchain extent.
			/// @param graphics_card The selected graphics card.
			/// @param surface The window surface.
			/// @param window_width The width of the window.
			/// @param window_height The height of the window.
			/// @param swapchain_extent The extent of the swapchain.
			void decide_width_and_height_of_swapchain_extent(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface, uint32_t& window_width, uint32_t& window_height, VkExtent2D& swapchain_extent);


			/// @brief Automatically finds the transform, relative to the presentation engine's natural orientation, applied to the image content prior to presentation.
			/// @param graphics_card The selected graphics card.
			/// @param surface The window surface.
			/// @return The image transform.
			VkSurfaceTransformFlagsKHR decide_which_image_transformation_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);


			/// @brief Automatically decides which presentation mode the presentation engine will be using.
			/// @note We can only use presentation modes that are available in the current system. The preferred presentation mode is VK_PRESENT_MODE_MAILBOX_KHR.
			/// @warning Just checking whether swap extension is supported is not enough because presentation support is a queue family property!
			/// A physical device may support swap chains, but that doesn't mean that all its queue families also support it.
			/// @param graphics_card The selected graphics card.
			/// @param surface The selected (window) surface.
			/// @return The presentation mode which will be used by the presentation engine.
			std::optional<VkPresentModeKHR> decide_which_presentation_mode_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);


			/// @brief Decides which graphics queue family index to use in case it is not possible to use one for both graphics and presentation.
			/// @warning This function should only be used when it is not possible to use one queue family for both graphics and presentation!
			/// @param graphics_card The selected graphics card.
			/// @return The index of the queue family which can be used for graphics.
			std::optional<uint32_t> decide_which_graphics_queue_family_to_use(const VkPhysicalDevice& graphics_card);
			
			
			/// @brief Decides which presentation queue family index to use in case it is not possible to use one for both graphics and presentation.
			/// @warning This function should only be used when it is not possible to use one queue family for both graphics and presentation!
			/// @param graphics_card The selected graphics card.
			/// @param surface The selected (window) surface.
			/// @return The index of the queue family which can be used for presentation.
			std::optional<uint32_t> decide_which_presentation_queue_family_to_use(const VkPhysicalDevice& graphics_card, const VkSurfaceKHR& surface);


	};

};
};
