#pragma once

// Vulkan Memory Allocator library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// License: MIT
#include "../../third_party/vma/vk_mem_alloc.h"

#include <cstddef>
#include <string>


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorStagingBuffer
	/// @brief An abstraction class for the creation of staging buffers.
	struct InexorBuffer
	{				
		// TODO: Inherit to make access unified in syntax!
		// TODO: Add mutex!
		// TODO: Map/Unmap memory.

		std::string name;
				
		VkBuffer buffer = VK_NULL_HANDLE;

		VmaAllocation allocation = VK_NULL_HANDLE;

		VmaAllocationInfo allocation_info = {};

		VkBufferCreateInfo create_info = {};
				
		VmaAllocationCreateInfo allocation_create_info = {};
		

	};


};
};
