#include "descriptor_set_builder.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	VkResult InexorDescriptorSetBuilder::initialise(const VkDevice& device,
													const std::size_t number_of_images_in_swapchain,
	                                                const std::shared_ptr<InexorDescriptorSetManager> descriptor_set_manager,
													const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager)
	{
		assert(descriptor_set_manager);

		spdlog::debug("Initialising descriptor set builder.");

		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(descriptor_set_builder_mutex);

		this->device = device;
		this->number_of_images_in_swapchain = number_of_images_in_swapchain;
		this->descriptor_set_manager = descriptor_set_manager;
		this->debug_marker_manager = debug_marker_manager;

		descriptor_set_builder_initialised = true;

		return VK_SUCCESS;
	}


	VkResult InexorDescriptorSetBuilder::reset_builder()
	{
		assert(descriptor_set_builder_initialised);

		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(descriptor_set_builder_mutex);

		spdlog::debug("Resetting descriptor set builder.");

		descriptor_set_building_started = false;
		descriptor_set_layout_finished  = false;
		descriptor_sets_finished        = false;
		descriptor_writes_finished      = false;

		descriptor_set_construction.descriptor_sets.clear();
		descriptor_set_construction.descriptor_writes.clear();
		descriptor_set_construction.descriptor_set_layout_bindings.clear();

		return VK_SUCCESS;
	}

	
	VkResult InexorDescriptorSetBuilder::create_descriptor_pool(const std::vector<VkDescriptorPoolSize>& pool_sizes)
	{
		assert(descriptor_set_builder_initialised);
		assert(!pool_sizes.empty());
		assert(device);

		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(descriptor_set_builder_mutex);

		spdlog::debug("Creating new descriptor pool.");

		VkDescriptorPoolCreateInfo pool_create_info = {};

		pool_create_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
		pool_create_info.pPoolSizes    = pool_sizes.data();
		pool_create_info.maxSets       = static_cast<uint32_t>(number_of_images_in_swapchain);

		VkResult result = vkCreateDescriptorPool(device, &pool_create_info, nullptr, &descriptor_set_construction.descriptor_pool);
		vulkan_error_check(result);

		return VK_SUCCESS;
	}

	
	VkResult InexorDescriptorSetBuilder::start_building_descriptor_set(const std::string& internal_descriptor_set_name)
	{
		assert(descriptor_set_builder_initialised);
		assert(!descriptor_set_building_started);
		assert(!descriptor_set_layout_finished);
		assert(!descriptor_writes_finished);
		assert(!descriptor_sets_finished);

		spdlog::debug("Starting to build descriptor set '{}'.", internal_descriptor_set_name);
		
		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(descriptor_set_builder_mutex);

		// Store the name of the descriptor set.
		descriptor_set_construction.name = internal_descriptor_set_name;

		descriptor_set_building_started = true;

		return VK_SUCCESS;
	}

	
	VkResult InexorDescriptorSetBuilder::add_descriptor_set_layout_binding(const VkDescriptorSetLayoutBinding& descriptor_set_layout)
	{
		assert(descriptor_set_builder_initialised);
		assert(descriptor_set_building_started);
		assert(!descriptor_set_layout_finished);
		assert(!descriptor_writes_finished);
		assert(!descriptor_sets_finished);

		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(descriptor_set_builder_mutex);
		
		spdlog::debug("Adding descriptor set layout to '{}'.", descriptor_set_construction.name);
		
		// Add the descriptor set layout to the descriptor set we are constructing.
		descriptor_set_construction.descriptor_set_layout_bindings.push_back(descriptor_set_layout);

		return VK_SUCCESS;
	}


	VkResult InexorDescriptorSetBuilder::add_write_descriptor_set(const VkWriteDescriptorSet& write_descriptor_set)
	{
		assert(descriptor_set_builder_initialised);
		assert(descriptor_set_building_started);
		assert(descriptor_set_layout_finished);
		assert(!descriptor_writes_finished);
		assert(!descriptor_sets_finished);
		
		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(descriptor_set_builder_mutex);

		spdlog::debug("Adding write descriptor set to '{}'.", descriptor_set_construction.name);

		// Add the write descriptor set to the descriptor set we are constructing.
		descriptor_set_construction.descriptor_writes.push_back(write_descriptor_set);

		return VK_SUCCESS;
	}


	VkResult InexorDescriptorSetBuilder::create_descriptor_set_layouts()
	{
		assert(descriptor_set_builder_initialised);
		assert(descriptor_set_building_started);
		assert(!descriptor_writes_finished);
		assert(!descriptor_set_layout_finished);
		assert(!descriptor_sets_finished);

		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(descriptor_set_builder_mutex);

		spdlog::debug("Creating descriptor set layout for '{}'.", descriptor_set_construction.name);

		VkDescriptorSetLayoutCreateInfo layout_create_info = {};
		      
		layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layout_create_info.bindingCount = static_cast<uint32_t>(descriptor_set_construction.descriptor_set_layout_bindings.size());
        layout_create_info.pBindings    = descriptor_set_construction.descriptor_set_layout_bindings.data();

		VkResult result = vkCreateDescriptorSetLayout(device, &layout_create_info, nullptr, &descriptor_set_construction.descriptor_set_layout);
		vulkan_error_check(result);

		std::string descriptor_set_name = "Descriptor set '"+ descriptor_set_construction.name +"'.";

		// Use a Vulkan debug marker to assign an appropriate name to this descriptor set.
		debug_marker_manager->set_object_name(device, (uint64_t)(descriptor_set_construction.descriptor_set_layout), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, descriptor_set_name.c_str());

		descriptor_set_layout_finished = true;

		return VK_SUCCESS;
	}


	VkDescriptorSetLayout InexorDescriptorSetBuilder::get_current_descriptor_set_layout()
	{
		assert(descriptor_set_builder_initialised);
		assert(descriptor_set_layout_finished);
		return descriptor_set_construction.descriptor_set_layout;
	}


	VkResult InexorDescriptorSetBuilder::create_descriptor_sets()
	{
		assert(descriptor_set_builder_initialised);
		assert(descriptor_set_building_started);
		assert(descriptor_set_layout_finished);
		assert(!descriptor_writes_finished);
		assert(!descriptor_sets_finished);
		
		// Lock write access.
		std::unique_lock<std::shared_mutex> lock(descriptor_set_builder_mutex);

		spdlog::debug("Creating descriptor sets for '{}'.", descriptor_set_construction.name);
		
		std::vector<VkDescriptorSetLayout> layouts(number_of_images_in_swapchain, descriptor_set_construction.descriptor_set_layout);
        
		VkDescriptorSetAllocateInfo alloc_info = {};

        alloc_info.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.descriptorPool     = descriptor_set_construction.descriptor_pool;
        alloc_info.descriptorSetCount = static_cast<uint32_t>(number_of_images_in_swapchain);
        alloc_info.pSetLayouts        = layouts.data();
		
		descriptor_set_construction.descriptor_sets.clear();
        descriptor_set_construction.descriptor_sets.resize(number_of_images_in_swapchain);

		VkResult result = vkAllocateDescriptorSets(device, &alloc_info, descriptor_set_construction.descriptor_sets.data());
		vulkan_error_check(result);

		for(std::size_t i=0; i<number_of_images_in_swapchain; i++)
		{
			spdlog::debug("Updating descriptor set '{}' #{}.", descriptor_set_construction.name, i);

			for(std::size_t j=0; j<descriptor_set_construction.descriptor_writes.size(); j++)
			{
				// Overwrite the binding just to be sure the order is correct!
				descriptor_set_construction.descriptor_writes[j].dstBinding = static_cast<uint32_t>(j);
				descriptor_set_construction.descriptor_writes[j].dstSet     = descriptor_set_construction.descriptor_sets[i];
			}
            
			vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptor_set_construction.descriptor_writes.size()), descriptor_set_construction.descriptor_writes.data(), 0, nullptr);
        }

		descriptor_writes_finished = true;
		descriptor_sets_finished   = true;

		return VK_SUCCESS;
	}


	VkResult InexorDescriptorSetBuilder::finalize_building_descriptor_sets()
	{
		assert(descriptor_set_builder_initialised);
		assert(descriptor_set_building_started);
		assert(descriptor_set_layout_finished);
		assert(!descriptor_writes_finished);
		assert(!descriptor_sets_finished);
		assert(descriptor_set_construction.descriptor_sets.empty());
		assert(!descriptor_set_construction.descriptor_writes.empty());

		VkResult result = create_descriptor_sets();
		vulkan_error_check(result);

		// Store descriptor set.
		descriptor_set_manager->add_descriptor_set(descriptor_set_construction.name, descriptor_set_construction);

		// Reset the builder class!
		reset_builder();

		return VK_SUCCESS;
	}


};
};
