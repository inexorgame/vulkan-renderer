#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>


namespace inexor {
namespace vulkan_renderer {


	struct InexorDescriptorSet
	{
		std::string name;

		VkDescriptorPool descriptor_pool;
			
		VkDescriptorSetLayout descriptor_set_layout;

		std::vector<VkDescriptorSet> descriptor_sets;

		std::vector<VkWriteDescriptorSet> descriptor_writes;

		std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;

	};


};
};
