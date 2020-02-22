#include "VulkanMeshBufferManager.hpp"
#include "../error-handling/VulkanErrorHandling.hpp"

using namespace std;


namespace inexor {
namespace vulkan_renderer {

	
	VulkanMeshBufferManager::VulkanMeshBufferManager()
	{
	}

	
	VulkanMeshBufferManager::~VulkanMeshBufferManager()
	{
	}

	
	VkResult VulkanMeshBufferManager::initialise(const VkDevice& device, const VmaAllocator& vma_allocator, const uint32_t& transfer_queue_family_index, const VkQueue& data_transfer_queue)
	{
		assert(device);
		assert(vma_allocator);
		assert(data_transfer_queue);

		vulkan_device = device;
		vma_allocator_handle = vma_allocator;
		vulkan_data_transfer_queue = data_transfer_queue;

		cout << "Creating command pool for vertex buffer manager." << endl;

		VkCommandPoolCreateInfo command_pool_create_info = {};

		command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext            = nullptr;
		command_pool_create_info.flags            = 0;

		// This might be a distinct data transfer queue exclusively offers VK
		command_pool_create_info.queueFamilyIndex = transfer_queue_family_index;

		// Create a second command pool for all commands that are going to be executed in the data transfer queue.
		VkResult result = vkCreateCommandPool(device, &command_pool_create_info, nullptr, &data_transfer_command_pool);
		vulkan_error_check(result);


		cout << "Creating command pool for vertex buffer manager." << endl;
		
		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

		command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool        = data_transfer_command_pool;
		command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1;

		// Allocate a command buffer for data transfer commands.
		result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &data_transfer_command_buffer);
		vulkan_error_check(result);

		return result;
	}


	VkResult VulkanMeshBufferManager::create_buffer(InexorBuffer& buffer, const VkBufferUsageFlags& buffer_usage, const VmaMemoryUsage& memory_usage)
	{
		assert(vma_allocator_handle);

		buffer.create_info.sType            = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer.create_info.size             = buffer.size;
		buffer.create_info.usage            = buffer_usage;
		buffer.create_info.sharingMode      = VK_SHARING_MODE_EXCLUSIVE;

		buffer.allocation_create_info.usage = memory_usage;
		buffer.allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkResult result = vmaCreateBuffer(vma_allocator_handle, &buffer.create_info, &buffer.allocation_create_info, &buffer.buffer, &buffer.allocation, &buffer.allocation_info);
		vulkan_error_check(result);
		
		return result;
	}


	VkResult VulkanMeshBufferManager::upload_data_to_gpu()
	{
		VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &data_transfer_command_buffer;

		// TODO: Add VkFence! For no we will use vkQueueWaitIdle.
		VkResult result = vkQueueSubmit(vulkan_data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Wait until copying memory is done!
		result = vkQueueWaitIdle(vulkan_data_transfer_queue);
		vulkan_error_check(result);

		return result;
	}


	VkResult VulkanMeshBufferManager::create_vertex_buffer(const VmaAllocator& vma_allocator, const std::vector<InexorVertex>& vertices, InexorMeshBuffer& mesh_buffer)
	{
		assert(vma_allocator);
		assert(vertices.size() > 0);
		assert(data_transfer_command_pool);

		// In general, it is inefficient to use normal memory mapping to a vertex buffer.
		// It is highly advised to use a staging buffer which will be filled with the vertex data.
		// Once the staging buffer is filled, a queue command can be executed to use a transfer queue
		// to upload the data to the GPU memory.
		
		// Calculate the size of the vertex buffer and the index buffer.
		std::size_t vertex_buffer_size = sizeof(InexorVertex) * vertices.size();

		// Create a staging vertex buffer.
		InexorBuffer staging_vertex_buffer(vertex_buffer_size);

		VkResult result = create_buffer(staging_vertex_buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Copy the vertex data to the staging vertex bufer.
		std::memcpy(staging_vertex_buffer.allocation_info.pMappedData, vertices.data(), vertex_buffer_size);

		// No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
		
		InexorBuffer vertex_buffer(vertex_buffer_size);

		result = create_buffer(vertex_buffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}


		VkBufferCopy vertex_buffer_copy_region = {};

		vertex_buffer_copy_region.srcOffset = 0;
		vertex_buffer_copy_region.dstOffset = 0;
		vertex_buffer_copy_region.size      = vertex_buffer.create_info.size;


		// It should be noted that it is more efficient to use queues which are specifically designed for this task.
		// We need to look for queues which have VK_QUEUE_TRANSFER_BIT, but not VK_QUEUE_GRAPHICS_BIT!
		// In some talks about Vulkan it was mentioned that not using dedicated transfer queues is one of the biggest mistakes when using Vulkan.
		// Copy vertex data from staging buffer to vertex buffer to upload it to GPU memory!
		
		VkCommandBufferBeginInfo cmd_buffer_begin_info;
		
		cmd_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buffer_begin_info.pNext            = nullptr;
		cmd_buffer_begin_info.pInheritanceInfo = nullptr;
		
		// We’re only going to use the command buffer once and wait with returning from the function until
		// the copy operation has finished executing. It’s good practice to tell the driver about our intent
		// using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
		cmd_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		result = vkBeginCommandBuffer(data_transfer_command_buffer, &cmd_buffer_begin_info);
		vulkan_error_check(result);

		vkCmdCopyBuffer(data_transfer_command_buffer, staging_vertex_buffer.buffer, vertex_buffer.buffer, 1, &vertex_buffer_copy_region);

		// End command buffer recording.
		result = vkEndCommandBuffer(data_transfer_command_buffer);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Submit buffer copy command to data transfer queue.
		upload_data_to_gpu();

		// Store the vertex buffer.
		mesh_buffer.vertex_buffer = vertex_buffer;

		// Yes, there is an index buffer available!
		mesh_buffer.index_buffer_available = false;

		// Store the number of vertices and indices.
		mesh_buffer.number_of_vertices = static_cast<uint32_t>(vertices.size());

		// Add this buffer to the list.
		list_of_meshes.push_back(mesh_buffer);

		// Destroy staging vertex buffer and its memory!
		vmaDestroyBuffer(vma_allocator, staging_vertex_buffer.buffer, staging_vertex_buffer.allocation);

		return result;
	}

	
	VkResult VulkanMeshBufferManager::create_vertex_buffer_with_index_buffer(const VmaAllocator& vma_allocator, const std::vector<InexorVertex>& vertices, const std::vector<uint32_t> indices, InexorMeshBuffer& mesh_buffer)
	{
		assert(vma_allocator);
		assert(vertices.size() > 0);
		assert(indices.size() > 0);
		assert(data_transfer_command_pool);

		// In general, it is inefficient to use normal memory mapping to a vertex buffer.
		// It is highly advised to use a staging buffer which will be filled with the vertex data.
		// Once the staging buffer is filled, a queue command can be executed to use a transfer queue
		// to upload the data to the GPU memory.
		
		// Calculate the size of the vertex buffer and the index buffer.
		std::size_t vertex_buffer_size = sizeof(InexorVertex) * vertices.size();
		std::size_t index_buffer_size = sizeof(InexorVertex) * indices.size();


		// Create a staging vertex buffer.
		InexorBuffer staging_vertex_buffer(vertex_buffer_size);

		VkResult result = create_buffer(staging_vertex_buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Copy the vertex data to the staging vertex bufer.
		std::memcpy(staging_vertex_buffer.allocation_info.pMappedData, vertices.data(), vertex_buffer_size);

		// No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.


		InexorBuffer staging_index_buffer(index_buffer_size);

		result = create_buffer(staging_index_buffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}


		// Copy the index data to the staging index buffer.
		std::memcpy(staging_index_buffer.allocation_info.pMappedData, indices.data(), index_buffer_size);

		
		InexorBuffer vertex_buffer(vertex_buffer_size);

		result = create_buffer(vertex_buffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		InexorBuffer index_buffer(index_buffer_size);
		
		result = create_buffer(index_buffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		VkBufferCopy vertex_buffer_copy_region = {};

		vertex_buffer_copy_region.srcOffset = 0;
		vertex_buffer_copy_region.dstOffset = 0;
		vertex_buffer_copy_region.size      = vertex_buffer.create_info.size;


		VkBufferCopy index_buffer_copy_region = {};

		index_buffer_copy_region.srcOffset = 0;
		index_buffer_copy_region.dstOffset = 0;
		index_buffer_copy_region.size      = index_buffer.create_info.size;


		// It should be noted that it is more efficient to use queues which are specifically designed for this task.
		// We need to look for queues which have VK_QUEUE_TRANSFER_BIT, but not VK_QUEUE_GRAPHICS_BIT!
		// In some talks about Vulkan it was mentioned that not using dedicated transfer queues is one of the biggest mistakes when using Vulkan.
		// Copy vertex data from staging buffer to vertex buffer to upload it to GPU memory!
		
		VkCommandBufferBeginInfo cmd_buffer_begin_info;
		
		cmd_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_buffer_begin_info.pNext            = nullptr;
		cmd_buffer_begin_info.pInheritanceInfo = nullptr;
		
		// We’re only going to use the command buffer once and wait with returning from the function until
		// the copy operation has finished executing. It’s good practice to tell the driver about our intent
		// using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
		cmd_buffer_begin_info.flags            = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		result = vkBeginCommandBuffer(data_transfer_command_buffer, &cmd_buffer_begin_info);
		vulkan_error_check(result);

		vkCmdCopyBuffer(data_transfer_command_buffer, staging_vertex_buffer.buffer, vertex_buffer.buffer, 1, &vertex_buffer_copy_region);
		vkCmdCopyBuffer(data_transfer_command_buffer, staging_index_buffer.buffer, index_buffer.buffer, 1, &index_buffer_copy_region);

		// End command buffer recording.
		result = vkEndCommandBuffer(data_transfer_command_buffer);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Submit buffer copy command to data transfer queue.
		upload_data_to_gpu();

		// Store the vertex buffer.
		mesh_buffer.vertex_buffer = vertex_buffer;

		// Yes, there is an index buffer available!
		mesh_buffer.index_buffer_available = true;

		// Store the index buffer.
		mesh_buffer.index_buffer = index_buffer;

		// Store the number of vertices and indices.
		mesh_buffer.number_of_vertices = static_cast<uint32_t>(vertices.size());
		mesh_buffer.number_of_indices = static_cast<uint32_t>(indices.size());

		// Add this buffer to the list.
		list_of_meshes.push_back(mesh_buffer);

		// Destroy staging vertex buffer and its memory!
		vmaDestroyBuffer(vma_allocator, staging_vertex_buffer.buffer, staging_vertex_buffer.allocation);
		
		// Destroy staging index buffer and its memory!
		vmaDestroyBuffer(vma_allocator, staging_index_buffer.buffer, staging_index_buffer.allocation);

		return result;
	}


	void VulkanMeshBufferManager::shutdown_vertex_buffers(const VmaAllocator& vma_allocator)
	{
		// Loop through all vertex buffers and release their memoy.
		for(const auto& mesh_buffer : list_of_meshes)
		{
			// Destroy vertex buffer.		
			vmaDestroyBuffer(vma_allocator, mesh_buffer.vertex_buffer.buffer, mesh_buffer.vertex_buffer.allocation);

			// TODO: vmaFreeMemory ?
			
			// Destroy index buffer if existent.
			if(mesh_buffer.index_buffer_available)
			{
				vmaDestroyBuffer(vma_allocator, mesh_buffer.index_buffer.buffer, mesh_buffer.index_buffer.allocation);
			}
		}


		cout << "Destroying command pool for vertex buffer manager." << endl;
		
		vkDestroyCommandPool(vulkan_device, data_transfer_command_pool, nullptr);
	}


};
};
