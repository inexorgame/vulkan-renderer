#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

// Vulkan Memory Allocator (VMA) library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#include "../../vma/vk_mem_alloc.h"

#include "../vertex-structure/InexorVertex.hpp"

#include <vector>


namespace inexor {
namespace vulkan_renderer {

	
	/// @class InexorVertexBufferMemory
	/// @brief We will store every allocation of memory in an instance of this
	/// structure, so we can store all allocations in a std::vector and shutdown
	/// every allocation in shutdown_vertex_buffers() at once!
	struct InexorVertexBuffer
	{
		VkBuffer buffer;

		VkBufferCreateInfo buffer_create_info;
		
		VmaAllocation allocation;

		VmaAllocationInfo allocation_info;
		
		VmaAllocationCreateInfo allocation_create_info;
	};


	/// @class VulkanVertexBufferManager
	/// @brief A manager class for vertex buffers.
	/// @note Buffers in Vulkan are regions of memory used for storing arbitrary data that can be read by the graphics card.
	/// @note Unlike the Vulkan objects, buffers do not automatically allocate memory for themselves.
	class VulkanVertexBufferManager
	{
		private:

			// The vertex buffers.
			std::vector<InexorVertexBuffer> list_of_vertex_buffers;


		public:

			VulkanVertexBufferManager();

			~VulkanVertexBufferManager();
		

		protected:


			/// @brief Creates a new vertex buffer.
			/// @param vma_allocator The allocator of the Vulkan Memory Allocator library.
			/// @param memory_size The size of the vertex buffer.
			/// @param vertex_buffer The InexorVertexBuffer instance to fill.
			/// @param memory_usage The VMA memory usage flags.
			/// @param create_flags The VMA buffer create flags.
			VkResult VulkanVertexBufferManager::create_vertex_buffer(const VmaAllocator& vma_allocator,
			                                                         const VkDeviceSize& memory_size,
                                                                     InexorVertexBuffer& vertex_buffer,
																	 const VmaMemoryUsage& memory_usage = VMA_MEMORY_USAGE_CPU_TO_GPU,
																	 const VmaAllocationCreateFlags& create_flags = VMA_ALLOCATION_CREATE_MAPPED_BIT);
		
			/// @brief Uses memory mapping to update vertex buffer.
			/// @warning You should use a staging buffer, since memory mapping is slow!
			/// @param vertex_buffer The Inexor vertex buffer which contains the bundled information.
			/// @param source_memory A pointer to The InexorVertex vertex data.
			void update_vertex_buffer_memory(InexorVertexBuffer& vertex_buffer, const InexorVertex* source_memory);

			/// @brief Releases all Vulkan memory buffers.
			/// @param vma_allocator The allocator of the Vulkan Memory Allocator library.
			void shutdown_vertex_buffers(const VmaAllocator& vma_allocator);

	};

};
};
