#pragma once

// Vulkan Memory Allocator library.
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// License: MIT.
#include <vma/vma_usage.h>

#include "vulkan-renderer/mesh-buffer/mesh_buffer.hpp"
#include "vulkan-renderer/error-handling/error_handling.hpp"
#include "vulkan-renderer/class-templates/manager_template.hpp"
#include "vulkan-renderer/gltf-model-manager/gltf_model_vertex.hpp"
#include "vulkan-renderer/debug-marker/debug_marker_manager.hpp"
#include "vulkan-renderer/gltf-model-manager/gltf_model_vertex.hpp"
#include "vulkan-renderer/gltf-model-manager/gltf_model_vertex.hpp"

#include <vector>
#include <shared_mutex>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>


namespace inexor
{
	namespace vulkan_renderer
	{


		/// @class InexorMeshBufferManager.
		/// @brief A manager class for vertex buffers and index buffers.
		/// @note Buffers in Vulkan are regions of memory used for storing arbitrary data that can be read by the graphics card.
		class InexorMeshBufferManager : public ManagerClassTemplate<InexorMeshBuffer>
		{
			private:

			std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

			VkCommandPool data_transfer_command_pool = VK_NULL_HANDLE;

			VkCommandBuffer data_transfer_command_buffer = VK_NULL_HANDLE;

			VkQueue data_transfer_queue = VK_NULL_HANDLE;

			VkDevice device = VK_NULL_HANDLE;

			VmaAllocator vma_allocator;

			std::shared_mutex mesh_buffer_manager_mutex;

			uint32_t data_transfer_queue_family_index = 0;

			bool mesh_buffer_manager_initialised = true;


			private:


			/// @brief Creates a command pool for mesh buffer manager.
			VkResult create_command_pool();


			/// @brief Submits buffer copy command to data transfer queue.
			VkResult upload_data_to_gpu();


			/// @brief Creates a buffer.
			/// @brief internal_buff [in] The internal 
			/// @param buffer [in] The buffer which will be created.
			/// @param buffer_size [in] The usage flags of the buffer.
			/// @param buffer_usage [in] The buffer usage flags.
			/// The default value for staging buffers is VK_BUFFER_USAGE_TRANSFER_SRC_BIT.
			/// The default value for vertex and index buffers is VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT.
			/// @param memory_usage [in] The VMA memory usage flags.
			/// The default value for staging buffers and normal buffers is VMA_MEMORY_USAGE_CPU_ONLY.
			VkResult create_buffer(std::string buffer_description,
								   InexorBuffer& buffer,
								   const VkDeviceSize& buffer_size,
								   const VkBufferUsageFlags& buffer_usage,
								   const VmaMemoryUsage& memory_usage);


			public:

			InexorMeshBufferManager() = default;

			~InexorMeshBufferManager() = default;


			/// @brief Initialises mesh buffer manager.
			/// @param device [in] The Vulkan device.
			/// @param debug_marker_manager [in] The Vulkan debug marker manager.
			/// @param vma_allocator [in] The Vulkan Memory Allocator library's allocator.
			/// @param data_transfer_queue_index [in] The data transfer queue family index.
			/// @param data_transfer_queue [in] The data transfer queue.
			VkResult initialise(const VkDevice& device,
								const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager,
								const VmaAllocator& vma_allocator,
								const uint32_t data_transfer_queue_family_index,
								const VkQueue& data_transfer_queue);


			/// @brief Creates a new vertex buffer of any Vertex structure type.
			/// @warning Creating a vertex buffer without index buffer is a bad idea because this decreases performance significantly!
			/// @param internal_buffer_name [in] The internal name of the vertex buffer.
			/// @param vertices [in] A pointer to the vertex data.
			/// @param size_of_vertex_structure [in] The memory size of the vertex data structure.
			/// @param number_of_vertices [in] The number of vertices.
			/// @param output_mesh_buffer [out] The mesh buffer which will be created.
			VkResult create_vertex_buffer(const std::string& internal_mesh_buffer_name,
										  const void* vertices,
										  const std::size_t size_of_vertex_structure,
										  const std::size_t number_of_vertices,
										  std::shared_ptr<InexorMeshBuffer>& output_mesh_buffer);


			/// @brief Creates a new vertex buffer and a corresponding index buffer.
			/// @param internal_mesh_buffer_name [in] The internal name of the vertex buffer 
			/// @param vertices [in] A pointer to the vertex data.
			/// It is recommended to store your vertices in a std::vector because you can just use .data() to pass the pointer to this method.
			/// @param size_of_vertex_structure [in] The memory size of the vertex data structure.
			/// @param number_of_vertices [in] The number of vertices.
			/// @param indices [in] A pointer to the index data.
			/// It is recommended to store your indices in a std::vector because you can just use .data() to pass the pointer to this method.
			/// @param size_of_index_structure [in] The memory size of the index data structure.
			/// @param number_of_indices [in] The number of indices.
			/// @param mesh_buffer_output [out] The mesh buffer which will be created.
			VkResult create_vertex_buffer_with_index_buffer(const std::string& internal_mesh_buffer_name,
															const void* vertices,
															const std::size_t size_of_vertex_structure,
															const std::size_t number_of_vertices,
															const void* indices,
															const std::size_t size_of_index_structure,
															const std::size_t number_of_indices,
															std::shared_ptr<InexorMeshBuffer>& mesh_buffer_output);


			// TODO: Overload those methods for established vertex data structures if desired.
			// TODO: Recycle buffers! Allocation of memory at during rendertime is expensive!
			// TODO: Destroy buffers.


			/// @brief Destroys all vertex buffers and index buffers.
			void shutdown_vertex_and_index_buffers();

		};

	};
};
