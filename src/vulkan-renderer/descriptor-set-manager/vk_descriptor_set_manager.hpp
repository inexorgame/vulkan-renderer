#pragma once

#include "../class-templates/manager_template.hpp"
#include "../debug-marker/vk_debug_marker_manager.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>
#include <assert.h>


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanDescriptorSetManager
	// TODO: Multiple descriptor sets! InexorDescriptorSet structure and manager class on top!
	class VulkanDescriptorSetManager
	{
		private:

			bool descriptor_set_manger_initialised = false;

			bool descriptor_set_layouts_finished = false;

			bool descriptor_sets_finished = false;
		
			bool descriptor_writes_finished = false;

			std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

			std::size_t number_of_descriptor_sets_in_layout = 0;

			std::size_t number_of_images_in_swapchain = 0;

			std::string internal_descriptor_set_name = "";

			VkDevice device;
			
			VkDescriptorPool descriptor_pool;

			VkDescriptorSetLayout descriptor_set_layout;

			std::vector<VkDescriptorSet> descriptor_sets;

			std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;

			std::vector<VkWriteDescriptorSet> descriptor_writes;
			

		public:
		
			VulkanDescriptorSetManager();
			
			~VulkanDescriptorSetManager();


		protected:
		
			/// @brief Initialises Vulkan descriptor set manager.
			/// @param device [in] The Vulkan device.
			/// @param internal_descriptor_set_name [in] The internal name of the descriptor set.
			/// @param debug_marker_manager [in] A shared pointer to the debug marker manager instance.
			/// @param number_of_images_in_swapchain [in] The number of images in swapchain.
			VkResult initialise(const VkDevice& device, const std::string& internal_descriptor_set_name, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager, const std::size_t number_of_images_in_swapchain);


			/// @brief Creates the descriptor pools.
			/// @param pool_sizes [in] The pool sizes.
			VkResult create_descriptor_pool(const std::vector<VkDescriptorPoolSize>& pool_sizes);


			/// @brief Adds a descriptor set layout binding.
			/// @note Parameter descriptor_set_layout.binding must be equivalent to descriptor_set_layout_bindings.size()!
			/// @param descriptor_set_layout [in] 
			/// @param write_descriptor_set [in] 
			VkResult add_descriptor_set_layout_binding(const VkDescriptorSetLayoutBinding& descriptor_set_layout);
			
			
			/// @brief 
			/// @param write_descriptor_set [in] The write descriptor set.
			VkResult add_write_descriptor_set(const VkWriteDescriptorSet& write_descriptor_set);

			
			/// @brief Creates the descriptor set layout.
			VkResult create_descriptor_set_layouts();


			/// @brief Creates the descriptor sets.
			VkResult create_descriptor_sets();


			/// @brief Returns a specific descriptor set by index.
			/// @param descriptor_set_index [in] The index of the descriptor set.
			VkDescriptorSet get_descriptor_set(const std::size_t descriptor_set_index);


			/// @brief Returns the descriptor set layout.
			VkDescriptorSetLayout get_descriptor_set_layout();


			/// @brief Destroys the descriptor sets and descriptor pool.
			/// @param clear_descriptor_layout_bindings [in] True if descriptor layout bindings vector should be cleared, false otherwise.
			/// We only need to clear the descriptor layout bindings when shutting down the entire application, but not for swapchain recreation.
			VkResult shutdown_descriptor_sets(bool clear_descriptor_layout_bindings = false);



	};


};
};
