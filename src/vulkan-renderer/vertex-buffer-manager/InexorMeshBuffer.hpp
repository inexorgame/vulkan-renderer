#pragma once

#include <vulkan/vulkan.h>

// Vulkan Memory Allocator.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#include "../../vma/vk_mem_alloc.h"


namespace inexor {
namespace vulkan_renderer {

	
	/// @class InexorMeshBuffer
	/// @brief A structure which bundles vertex buffer and index buffer (if existent).
	/// It contains all data which are related to memory allocations for these buffers.
	struct InexorMeshBuffer
	{
		VkBuffer vertex_buffer = VK_NULL_HANDLE;

		VkBufferCreateInfo vertex_buffer_create_info;
		
		VmaAllocation vertex_buffer_allocation;

		VmaAllocationInfo vertex_buffer_allocation_info;
		
		VmaAllocationCreateInfo vertex_buffer_allocation_create_info;

		uint32_t number_of_vertices = 0;


		// TODO!
		// Driver developers recommend that you store multiple
		// buffers, like the vertex and index buffer, into a single VkBuffer and use offsets
		// in commands like vkCmdBindVertexBuffers. The advantage is that your data
		// is more cache friendly in that case, because it’s closer together. It is even possible
		// to reuse the same chunk of memory for multiple resources if they are not
		// used during the same render operations, provided that their data is refreshed,
		// of course. This is known as aliasing and some Vulkan functions have explicit
		// flags to specify that you want to do this.


		// Don't forget that index buffers are optional!
		bool index_buffer_available = false;
	
		VkBuffer index_buffer = VK_NULL_HANDLE;

		VkBufferCreateInfo index_buffer_create_info;
		
		VmaAllocation index_buffer_allocation;

		VmaAllocationInfo index_buffer_allocation_info;
		
		VmaAllocationCreateInfo index_buffer_allocation_create_info;

		uint32_t number_of_indices = 0;

	};


};
};
