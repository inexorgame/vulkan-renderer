#include "vk_descriptor_set_manager.hpp"
#include "../error-handling/vk_error_handling.hpp"


namespace inexor {
namespace vulkan_renderer {


	VkResult VulkanDescriptorSetManager::initialise(const VkDevice& device, const std::string& internal_descriptor_set_name, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager, const std::size_t number_of_images_in_swapchain)
	{
		assert(device);
		assert(number_of_images_in_swapchain>0);
		
		spdlog::debug("Initialising descriptor set manager.");

		this->device = device;
		this->debug_marker_manager = debug_marker_manager;
		this->internal_descriptor_set_name = internal_descriptor_set_name;
		this->number_of_images_in_swapchain = number_of_images_in_swapchain;

		descriptor_set_manger_initialised = true;

		return VK_SUCCESS;
	}


	VkResult VulkanDescriptorSetManager::create_descriptor_pool(const std::vector<VkDescriptorPoolSize>& pool_sizes)
	{
		assert(descriptor_set_manger_initialised);
		assert(number_of_images_in_swapchain>0);
		assert(pool_sizes.size()>0);
		assert(device);

		VkDescriptorPoolCreateInfo pool_info = {};

		pool_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_info.pPoolSizes    = pool_sizes.data();
		pool_info.maxSets       = static_cast<uint32_t>(number_of_images_in_swapchain);

		VkResult result = vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptor_pool);
		vulkan_error_check(result);

		return VK_SUCCESS;
	}


	VkResult VulkanDescriptorSetManager::add_descriptor_set_layout_binding(const VkDescriptorSetLayoutBinding& descriptor_set_layout)
	{
		assert(descriptor_set_manger_initialised);

		spdlog::debug("Adding another descriptor set layout to '{}'.", internal_descriptor_set_name);

		descriptor_set_layout_bindings.push_back(descriptor_set_layout);

		return VK_SUCCESS;
	}


	VkResult VulkanDescriptorSetManager::add_write_descriptor_set(const VkWriteDescriptorSet& write_descriptor_set)
	{
		assert(descriptor_set_manger_initialised);

		spdlog::debug("Adding write descriptor set to '{}'", internal_descriptor_set_name);

		descriptor_writes.push_back(write_descriptor_set);

		return VK_SUCCESS;
	}


	VkResult VulkanDescriptorSetManager::create_descriptor_set_layouts()
	{
		assert(descriptor_set_manger_initialised);
		assert(device);
		assert(debug_marker_manager);
		assert(descriptor_set_layout_bindings.size()>0);

		spdlog::debug("Finalizing descriptor set '{}' using {} descriptor set layout bindings.", internal_descriptor_set_name, descriptor_set_layout_bindings.size());

		VkDescriptorSetLayoutCreateInfo layout_info = {};

        layout_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_info.bindingCount = static_cast<uint32_t>(descriptor_set_layout_bindings.size());
        layout_info.pBindings    = descriptor_set_layout_bindings.data();

		VkResult result = vkCreateDescriptorSetLayout(device, &layout_info, nullptr, &descriptor_set_layout);
		vulkan_error_check(result);

		std::string descriptor_set_name = "Descriptor set '"+ internal_descriptor_set_name +"'.";

		// Use a Vulkan debug marker to assign an appropriate name to this descriptor set.
		debug_marker_manager->set_object_name(device, (uint64_t)(descriptor_set_layout), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, descriptor_set_name.c_str());

		descriptor_set_layouts_finished = true;

		return VK_SUCCESS;
	}

	
	VkResult VulkanDescriptorSetManager::create_descriptor_sets()
	{
		assert(descriptor_set_manger_initialised);
		assert(descriptor_set_layouts_finished);
		assert(descriptor_set_layout);

		std::vector<VkDescriptorSetLayout> layouts(number_of_images_in_swapchain, descriptor_set_layout);
        
		VkDescriptorSetAllocateInfo alloc_info = {};

        alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool     = descriptor_pool;
        alloc_info.descriptorSetCount = static_cast<uint32_t>(number_of_images_in_swapchain);
        alloc_info.pSetLayouts        = layouts.data();

		descriptor_sets.clear();
        descriptor_sets.resize(number_of_images_in_swapchain);

		VkResult result = vkAllocateDescriptorSets(device, &alloc_info, descriptor_sets.data());
		vulkan_error_check(result);

		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			spdlog::debug("Updating descriptor set '{}' #{}.", internal_descriptor_set_name, i);

			for(std::size_t j=0; j<descriptor_writes.size(); j++)
			{
				// Overwrite the binding just to be sure the order is correct!
				descriptor_writes[j].dstBinding = static_cast<uint32_t>(j);
				descriptor_writes[j].dstSet     = descriptor_sets[i];
			}
            
			// TODO: Move out of loop?
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptor_writes.size()), descriptor_writes.data(), 0, nullptr);
        }

		descriptor_sets_finished = true;
		descriptor_writes_finished = true;

		return VK_SUCCESS;
	}


	// TODO: std::optional!
	VkDescriptorSet VulkanDescriptorSetManager::get_descriptor_set(const std::size_t descriptor_set_index)
	{
		assert(descriptor_set_manger_initialised);
		assert(descriptor_set_layouts_finished);
		assert(descriptor_sets_finished);
		assert(descriptor_set_index<descriptor_sets.size());

		return descriptor_sets[descriptor_set_index];
	}
	
	// TODO: std::optional!
	VkDescriptorSetLayout VulkanDescriptorSetManager::get_descriptor_set_layout()
	{
		assert(descriptor_set_manger_initialised);
		assert(descriptor_set_layouts_finished);
		assert(descriptor_set_layout);

		return descriptor_set_layout;
	}


	// TODO: Unify into one shutdown method!
	VkResult VulkanDescriptorSetManager::shutdown_descriptor_sets(bool clear_descriptor_layout_bindings)
	{
		assert(descriptor_set_manger_initialised);
		assert(device);
		
		spdlog::debug("Destroying descriptor set layout.");

		if(VK_NULL_HANDLE != descriptor_set_layout)
		{
			vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
			descriptor_set_layout = VK_NULL_HANDLE;
		}
		
		spdlog::debug("Destroying descriptor pool.");

		if(VK_NULL_HANDLE != descriptor_pool)
		{
			vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
			descriptor_pool = VK_NULL_HANDLE;
		}

		descriptor_sets.clear();
		descriptor_writes.clear();

		descriptor_sets_finished = false;
		descriptor_writes_finished = false;

		if(clear_descriptor_layout_bindings)
		{
			spdlog::debug("Clearing descriptor set layout bindings as well.");
			descriptor_set_layout_bindings.clear();
		}

		return VK_SUCCESS;
	}

	
};
};
