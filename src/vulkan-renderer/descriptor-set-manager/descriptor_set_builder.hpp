#pragma once

#include "descriptor_set_manager.hpp"
#include "../debug-marker/vk_debug_marker_manager.hpp"

#include <vulkan/vulkan.h>

#include <spdlog/spdlog.h>

#include <memory>
#include <shared_mutex>


namespace inexor {
namespace vulkan_renderer {


	// TOOD: Only accept shared pointers of descriptor set managers which are initialised.
	class InexorDescriptorSetBuilder
	{
		private:
			
			std::shared_mutex descriptor_set_builder_mutex;

			std::shared_ptr<InexorDescriptorSetManager> descriptor_set_manager;

			std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

			bool descriptor_set_builder_initialised = false;

			bool descriptor_set_building_started = false;

			bool descriptor_set_layout_finished = false;

			bool descriptor_sets_finished = false;

			bool descriptor_writes_finished = false;

			std::size_t number_of_images_in_swapchain = 0;

			VkDevice device;

			InexorDescriptorSet descriptor_set_construction;

			/// @brief Resets the descriptor set builder so a new build can start.
			VkResult reset_builder();


		public:

			InexorDescriptorSetBuilder() = default;
			

			~InexorDescriptorSetBuilder() = default;

			
			/// @brief Initialises descriptor set builder.
			/// The descriptor set builder needs to be initialised only once.
			/// @param descriptor_set_manager [in] A shared pointer to a descriptor set manager.
			VkResult initialise(const VkDevice& device,
								const std::size_t number_of_images_in_swapchain,
			                    const std::shared_ptr<InexorDescriptorSetManager> descriptor_set_manager,
								const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager);

			
			/// TOOD: Descriptor pool manager.
			/// @brief Creates the descriptor pools.
			/// @param pool_sizes [in] The pool sizes.
			VkResult create_descriptor_pool(const std::vector<VkDescriptorPoolSize>& pool_sizes);


			/// @brief Starts building a new descriptor set.
			/// Building can only start if descriptor set builder has already been initialised.
			/// @param internal_descriptor_set_name [in] The internal name of the descriptor set.
			VkResult start_building_descriptor_set(const std::string& internal_descriptor_set_name);


			/// @brief Adds a descriptor set layout binding.
			/// You can add as many write descriptor sets as your architecture requires.
			/// @param descriptor_set_layout [in] The descriptor set layout.
			VkResult add_descriptor_set_layout_binding(const VkDescriptorSetLayoutBinding& descriptor_set_layout);
			

			/// @brief Returns a descriptor set.
			/// @param internal_descriptor_set_name [in] The internal name of the descriptor set.
			/// @return The VkDescriptorSetLayout associated with the key (if existent), std::nullopt otherwise.
			VkDescriptorSetLayout get_current_descriptor_set_layout();

			
			/// @brief Adds a write descriptor set to the descriptor set.
			/// You can add as many write descriptor sets as your architecture requires.
			/// @param write_descriptor_set [in] The write descriptor set.
			VkResult add_write_descriptor_set(const VkWriteDescriptorSet& write_descriptor_set);

			
			/// @brief Creates the descriptor set layout.
			VkResult create_descriptor_set_layouts();

			
			/// @brief Creates the descriptor sets.
			VkResult create_descriptor_sets();


			/// @brief Ends building the descriptor set.
			/// The final result will be stored by internal name as key using InexorDescriptorSetManager.
			VkResult finalize_building_descriptor_sets();


	};

};
};
