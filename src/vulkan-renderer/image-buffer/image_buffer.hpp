#pragma once

#include <optional>

// Vulkan Memory Allocator library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// License: MIT
#include <vma/vma_usage.h>


namespace inexor
{
	namespace vulkan_renderer
	{


		/// 
		/// 
		/// 
		struct InexorImageBuffer
		{
			VmaAllocation allocation = VK_NULL_HANDLE;

			VmaAllocationInfo allocation_info = {};

			VmaAllocationCreateInfo allocation_create_info = {};

			VkImage image = VK_NULL_HANDLE;

			VkImageView image_view = VK_NULL_HANDLE;

			VkFormat format;

		};


	};
};
