#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

// Vulkan Memory Allocator.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#include "../../vma/vk_mem_alloc.h"

#include "../vertex-structure/InexorVertex.hpp"
#include "../vertex-buffer-manager/InexorVertexBuffer.hpp"

#include <vector>


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanVertexBufferManager
	/// @brief A manager class for vertex buffers.
	/// @note Buffers in Vulkan are regions of memory used for storing arbitrary data that can be read by the graphics card.
	/// @note Unlike the Vulkan objects, buffers do not automatically allocate memory for themselves.
	class VulkanVertexBufferManager
	{
		private:

			// The vertex buffers (which may has a corresponding index buffer linked to it).
			std::vector<InexorVertexBuffer> list_of_vertex_buffers;

			// The command pool for data transfer.
			VkCommandPool data_transfer_command_pool;

			// The Vulkan device.
			VkDevice vulkan_device;


		private:

			// TOOD: Refactoring here!

			// copy_index_buffer_data()
			// copy_vertex_buffer_data()
			// submit_data_transfer_command()
			// create_staging_buffers()
			// create vertex buffer

		public:

			VulkanVertexBufferManager();

			~VulkanVertexBufferManager();
		

		protected:

			
			/// @brief Initialises a command pool for commands that are commited on the data transfer queue
			/// @param device The Vulkan device.
			/// @param data_transfer_queue_index The queue family index which is used for data transfer.
			/// This is neccesary since we need to allocate a new command pool for the staging buffer!
			VkResult initialise(const VkDevice& device, const uint32_t& data_transfer_queue_index);

			
			/// @brief Creates a new vertex buffer.
			/// @param vma_allocator The allocator of the Vulkan Memory Allocator library.
			/// @param data_transfer_queue The VkQueue which is used for data transfer from CPU to GPU.
			/// @param vertices The vertices to fill into the vertex buffer.
			/// @param target_vertex_buffer The InexorVertexBuffer instance to fill.
			VkResult create_vertex_buffer(const VmaAllocator& vma_allocator, const VkQueue& data_transfer_queue, const std::vector<InexorVertex>& vertices, InexorVertexBuffer& target_vertex_buffer);
		
			
			/// @brief Creates a new vertex buffer with a corresponding index buffer.
			/// @param 
			/// @param 
			/// @param 
			/// @param 
			VkResult create_vertex_buffer_with_index_buffer(const VmaAllocator& vma_allocator, const VkQueue& data_transfer_queue, const std::vector<InexorVertex>& vertices, const std::vector<uint32_t> indices, InexorVertexBuffer& target_vertex_buffer);


			/// @brief Releases all Vulkan memory buffers.
			/// @param vma_allocator The allocator of the Vulkan Memory Allocator library.
			void shutdown_vertex_buffers(const VmaAllocator& vma_allocator);

	};

};
};
