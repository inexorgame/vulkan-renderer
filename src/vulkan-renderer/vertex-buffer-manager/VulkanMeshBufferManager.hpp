#pragma once

#include <glm/glm.hpp>

#include <vulkan/vulkan.h>

// Vulkan Memory Allocator.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
#include "../../vma/vk_mem_alloc.h"

#include "../vertex-structure/InexorVertex.hpp"
#include "../vertex-buffer-manager/InexorMeshBuffer.hpp"
#include "../debug-marker/VulkanDebugMarkerManager.hpp"

#include <vector>

#include <spdlog/spdlog.h>


namespace inexor {
namespace vulkan_renderer {


	/// @class VulkanMeshBufferManager
	/// @brief A manager class for vertex buffers.
	/// @note Buffers in Vulkan are regions of memory used for storing arbitrary data that can be read by the graphics card.
	/// @note Unlike the Vulkan objects, buffers do not automatically allocate memory for themselves.
	class VulkanMeshBufferManager
	{
		private:

			
			// The debug marker manager.
			std::shared_ptr<VulkanDebugMarkerManager> dbg_marker_manager;

			// The mesh buffers.
			// TODO: Should we just use pointers for this?
			std::vector<InexorMeshBuffer> list_of_meshes;

			// The command pool for data transfer.
			VkCommandPool data_transfer_command_pool = VK_NULL_HANDLE;

			// The command buffer for data transfer to GPU memory.
			VkCommandBuffer data_transfer_command_buffer = VK_NULL_HANDLE;

			// The data transfer queue.
			VkQueue vulkan_data_transfer_queue = VK_NULL_HANDLE;

			/// 
			VkDevice vulkan_device = VK_NULL_HANDLE;

			// The Vulkan Memory Allocator handle.
			VmaAllocator vma_allocator_handle;


		public:

		
			/// @brief Create a buffer.
			/// @param buffer The InexorBuffer which will be created.
			/// @param buffer_usage The usage flags of the buffer. The default is value is VK_BUFFER_USAGE_TRANSFER_SRC_BIT for staging buffers.
			VkResult create_buffer(InexorBuffer& buffer, const VkBufferUsageFlags& buffer_usage, const VmaMemoryUsage& memory_usage);

			
		private:

			/// @brief Submits buffer copy command to data transfer queue.
			VkResult upload_data_to_gpu();


		public:

			VulkanMeshBufferManager();

			~VulkanMeshBufferManager();
		

		protected:

			
			/// @brief Initialises a command pool for commands that are commited on the data transfer queue
			/// @param device The Vulkan device.
			/// @param data_transfer_queue_index The queue family index which is used for data transfer.
			/// This is neccesary since we need to allocate a new command pool for the staging buffer!
			/// @param data_transfer_queue The VkQueue which is used for data transfer from CPU to GPU.
			VkResult initialise(const VkDevice& device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager_instance, const VmaAllocator& allocator, const uint32_t& data_transfer_queue_index, const VkQueue& data_transfer_queue);

			
			/// @brief Creates a new vertex buffer.
			/// @param vma_allocator The allocator of the Vulkan Memory Allocator library.
			/// @param vertices The vertices to fill into the vertex buffer.
			/// @param mesh_buffer The target InexorMeshBuffer instance to fill.
			VkResult create_vertex_buffer(const std::string& internal_buffer_name, const std::vector<InexorVertex>& vertices, std::vector<InexorMeshBuffer>& mesh_buffers);
		
			
			/// @brief Creates a new vertex buffer with a corresponding index buffer.
			/// @param vma_allocator The allocator of the Vulkan Memory Allocator library.
			/// @param vertices The vertices to fill into the vertex buffer.
			/// @param indices 
			/// @param mesh_buffer The target InexorMeshBuffer instance to fill.
			VkResult create_vertex_buffer_with_index_buffer(const std::string& internal_buffer_name, const std::vector<InexorVertex>& vertices, const std::vector<uint32_t> indices, std::vector<InexorMeshBuffer>& mesh_buffers);


			/// @brief Releases all Vulkan memory buffers.
			/// @param vma_allocator The allocator of the Vulkan Memory Allocator library.
			void shutdown_vertex_buffers();

	};

};
};
