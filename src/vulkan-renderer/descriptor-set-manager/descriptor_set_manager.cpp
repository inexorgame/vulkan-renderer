#include "descriptor_set_manager.hpp"


namespace inexor {
namespace vulkan_renderer {


	VkResult InexorDescriptorSetManager::initialise(const VkDevice& device,
	                                                const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager,
													const std::size_t number_of_images_in_swapchain)
	{
		assert(device);
		assert(debug_marker_manager);

		spdlog::debug("Initialising descriptor set manager.");

		this->device = device;
		this->debug_marker_manager = debug_marker_manager;

		descriptor_set_manger_initialised = true;

		return VK_SUCCESS;
	}
	
	
	VkResult InexorDescriptorSetManager::add_descriptor_set(const std::string& internal_descriptor_set_name,
															const InexorDescriptorSet& descriptor_set_source)
	{
		assert(!internal_descriptor_set_name.empty());

		if(does_key_exist(internal_descriptor_set_name))
		{
			spdlog::error("A descriptor set with internal name '{}' already exists!", internal_descriptor_set_name);
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		std::shared_ptr<InexorDescriptorSet> descriptor_set = std::make_shared<InexorDescriptorSet>(descriptor_set_source);

		add_entry(internal_descriptor_set_name, descriptor_set);

		return VK_SUCCESS;
	}
	
	
	std::vector<VkDescriptorSet> InexorDescriptorSetManager::get_descriptor_sets(const std::string& internal_descriptor_set_name)
	{
		if(does_key_exist(internal_descriptor_set_name))
		{
			auto entry = get_entry(internal_descriptor_set_name);

			if(entry.has_value())
			{
				return entry.value()->descriptor_sets;
			}
		}
		else
		{
			spdlog::error("Descriptor set with internal name '{}' does not exist!", internal_descriptor_set_name);
		}

		// Return an empty vector instead of std::optional<std::vector<VkDescriptorSet>>
		return std::vector<VkDescriptorSet>();
	}


	VkResult InexorDescriptorSetManager::shutdown_descriptor_sets(bool clear_descriptor_layout_bindings)
	{
		assert(descriptor_set_manger_initialised);
		assert(device);
		
		spdlog::debug("Destroying descriptor set layout.");

		auto descriptor_sets = get_all_values();

		for(auto& descriptor_set : descriptor_sets)
		{
			vkDestroyDescriptorSetLayout(device, descriptor_set->descriptor_set_layout, nullptr);
			descriptor_set->descriptor_set_layout = VK_NULL_HANDLE;
			
			if(VK_NULL_HANDLE != descriptor_set->descriptor_pool)
			{
				vkDestroyDescriptorPool(device, descriptor_set->descriptor_pool, nullptr);
				descriptor_set->descriptor_pool = VK_NULL_HANDLE;
			}
		}
		
		spdlog::debug("Destroying descriptor pool.");

		if(clear_descriptor_layout_bindings)
		{
			spdlog::debug("Clearing descriptor set layout bindings as well.");

			for(auto& descriptor_set : descriptor_sets)
			{
				descriptor_set->descriptor_set_layout_bindings.clear();
			}
		}
		
		delete_all_entries();

		return VK_SUCCESS;
	}

	
};
};
