#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

// Vulkan Memory Allocator (VMA) library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#include "../../vma/vk_mem_alloc.h"

#include "../vertex-structure/InexorVertex.hpp"

#include <vector>
#include <iostream>


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

		uint32_t number_of_vertices;
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

			// The command pool for data transfer.
			VkCommandPool data_transfer_command_pool;

			// The Vulkan device.
			VkDevice vulkan_device;


		public:

			VulkanVertexBufferManager();

			~VulkanVertexBufferManager();
		

		protected:

			
			/// @brief Initialises a command pool for commands that are commited on the data transfer queue
			/// @param device The Vulkan device.
			VkResult initialise(const VkDevice& device, const uint32_t& data_transfer_queue_index);

			
			/// @brief Creates a new vertex buffer.
			/// @param vma_allocator The allocator of the Vulkan Memory Allocator library.
			/// @param memory_size The size of the vertex buffer.
			/// @param vertex_buffer The InexorVertexBuffer instance to fill.
			/// @param memory_usage The VMA memory usage flags.
			/// @param create_flags The VMA buffer create flags.
			VkResult create_vertex_buffer(const VmaAllocator& vma_allocator, const VkQueue& data_transfer_queue, const std::vector<InexorVertex>& vertices, InexorVertexBuffer& target_vertex_buffer);
		

			/// @brief Releases all Vulkan memory buffers.
			/// @param vma_allocator The allocator of the Vulkan Memory Allocator library.
			void shutdown_vertex_buffers(const VmaAllocator& vma_allocator);

	};

};
};
