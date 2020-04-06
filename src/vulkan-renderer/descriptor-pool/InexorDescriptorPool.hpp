#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>


namespace inexor {
namespace vulkan_renderer {

	/// 
	struct InexorDescriptorPool
	{
		std::string name;

		std::vector<VkDescriptorPoolSize> pool_sizes;

		VkDescriptorPool pool;
	};


};
};
