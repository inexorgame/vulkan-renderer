#pragma once

#include "vulkan-renderer/gpu-memory-buffer/gpu_memory_buffer.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	/// 
	/// 
	/// 
	struct InexorUniformBuffer : public InexorBuffer
	{
		// 
		VkDescriptorBufferInfo descriptor_buffer_info;
		
		// 
		VkDescriptorSet descriptor_set;

	};


};
};
