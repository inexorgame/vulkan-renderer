#pragma once

// Vulkan Memory Allocator library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// License: MIT
#include "../third_party/vma/vk_mem_alloc.h"


namespace inexor {
namespace vulkan_renderer {


	struct InexorDepthBuffer
	{
		VmaAllocation allocation = VK_NULL_HANDLE;

		VmaAllocationInfo allocation_info;
			
		VmaAllocationCreateInfo allocation_create_info = {};	
			
		VkImage image;

		VkImageView image_view;
			
		std::optional<VkFormat> format;

	};

};
};
