#pragma once

// Vulkan Memory Allocator library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// License: MIT
#include <vma/vma_usage.h>

#include <cstddef>
#include <string>


namespace inexor
{
	namespace vulkan_renderer
	{


		/// @class InexorStagingBuffer
		/// @brief An abstraction class for the creation of staging buffers.
		/// @note We can't add a std::mutex in here because that would force InexorBuffer to be uncopyable!
		struct InexorBuffer
		{
			// TODO: Inherit to make access unified in syntax!
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
