#pragma once

#include "vulkan-renderer/gpu-memory-buffer/gpu_memory_buffer.hpp"

#include <vulkan/vulkan.h>

// Vulkan Memory Allocator library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// License: MIT
#include <vma/vma_usage.h>


namespace inexor
{
	namespace vulkan_renderer
	{


		/// @class InexorMeshBuffer
		/// @brief A structure which bundles vertex buffer and index buffer (if existent).
		/// It contains all data which are related to memory allocations for these buffers.
		/// @todo Driver developers recommend that you store multiple
		/// buffers, like the vertex and index buffer, into a single VkBuffer and use offsets
		/// in commands like vkCmdBindVertexBuffers. The advantage is that your data
		/// is more cache friendly in that case, because it’s closer together. It is even possible
		/// to reuse the same chunk of memory for multiple resources if they are not
		/// used during the same render operations, provided that their data is refreshed,
		/// of course. This is known as aliasing and some Vulkan functions have explicit
		/// flags to specify that you want to do this.
		struct InexorMeshBuffer
		{
			InexorBuffer vertex_buffer;

			InexorBuffer index_buffer;

			uint32_t number_of_vertices = 0;

			uint32_t number_of_indices = 0;

			std::string description = "";

			// Don't forget that index buffers are optional!
			bool index_buffer_available = false;
		};


	};
};
