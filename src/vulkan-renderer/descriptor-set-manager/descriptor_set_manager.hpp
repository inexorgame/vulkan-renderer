#pragma once

#include "../descriptor-set/vk_descriptor_set.hpp"
#include "../class-templates/manager_template.hpp"
#include "../debug-marker/vk_debug_marker_manager.hpp"
#include "../error-handling/vk_error_handling.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <cassert>


namespace inexor {
namespace vulkan_renderer {


	class InexorDescriptorSetManager : public ManagerClassTemplate<InexorDescriptorSet>
	{
		private:

			VkDevice device = VK_NULL_HANDLE;


			bool descriptor_set_manger_initialised = false;


			std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;
		
			// Declare these methods as private to enforce the use of the builder pattern.
			// Declare InexorDescriptorSetBuilder as friend class so it can access private methods.
			friend class InexorDescriptorSetBuilder;
			
			/// @brief Adds a new descriptor set.
			/// @param internal_descriptor_set_name [in] The name of the new descriptor set.
			VkResult add_descriptor_set(const std::string& internal_descriptor_set_name,
			                            const InexorDescriptorSet& descriptor_set);

			// TODO: Update/Delete methods.


		public:

		
			InexorDescriptorSetManager() = default;
			

			~InexorDescriptorSetManager() = default;

		
			/// @brief Initialises Vulkan descriptor set manager.
			/// @param device [in] The Vulkan device.
			/// @param debug_marker_manager [in] A shared pointer to the debug marker manager instance.
			/// @param number_of_images_in_swapchain [in] The number of images in swapchain.
			VkResult initialise(const VkDevice& device,
			                    const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager,
								const std::size_t number_of_images_in_swapchain);


			/// @brief Returns a certain descriptor set by key (internal name).
			/// @param internal_descriptor_set_name [in] The internal name of the descriptor set.
			std::vector<VkDescriptorSet> get_descriptor_sets(const std::string& internal_descriptor_set_name);



			/// @brief Destroys the descriptor sets and descriptor pool.
			/// @param clear_descriptor_layout_bindings [in] True if descriptor layout bindings vector should be cleared, false otherwise.
			/// We only need to clear the descriptor layout bindings when shutting down the entire application, but not for swapchain recreation.
			VkResult shutdown_descriptor_sets(bool clear_descriptor_layout_bindings = false);


	};


};
};
