#include "VulkanVertexBufferManager.hpp"
#include "../error-handling/VulkanErrorHandling.hpp"

using namespace std;


namespace inexor {
namespace vulkan_renderer {

	
	VulkanVertexBufferManager::VulkanVertexBufferManager()
	{
	}

	
	VulkanVertexBufferManager::~VulkanVertexBufferManager()
	{
	}

	
	VkResult VulkanVertexBufferManager::initialise(const VkDevice& device, const uint32_t& transfer_queue_family_index)
	{
		assert(device);

		vulkan_device = device;

		cout << "Creating vertex buffer manager command pool." << endl;

		VkCommandPoolCreateInfo command_pool_create_info = {};

		command_pool_create_info.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext            = nullptr;
		command_pool_create_info.flags            = 0;

		// This might be a distinct data transfer queue exclusively offers VK
		command_pool_create_info.queueFamilyIndex = transfer_queue_family_index;

		// Create a second command pool for all commands that are going to be executed in the data transfer queue.
		VkResult result = vkCreateCommandPool(vulkan_device, &command_pool_create_info, nullptr, &data_transfer_command_pool);
		vulkan_error_check(result);

		return result;
	}


	VkResult VulkanVertexBufferManager::create_vertex_buffer(const VmaAllocator& vma_allocator, const VkQueue& data_transfer_queue, const std::vector<InexorVertex>& vertices, InexorVertexBuffer& target_vertex_buffer)
	{
		assert(vma_allocator);
		assert(vulkan_device);
		assert(data_transfer_command_pool);
		assert(vertices.size() > 0);

		// In general, it is inefficient to use normal memory mapping to a vertex buffer.
		// It is highly advised to use a staging buffer which will be filled with the vertex data.
		// Once the staging buffer is filled, a queue command can be executed to use a transfer queue
		// to upload the data to the GPU memory.
		
		std::size_t vertex_buffer_size = sizeof(InexorVertex) * vertices.size();


		// Step 1: Create staging vertex buffer.
		VkBufferCreateInfo staging_buffer_create_info = {};

		staging_buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		staging_buffer_create_info.size        = vertex_buffer_size;
		staging_buffer_create_info.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		staging_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		// Use Vulkan Memory Allocator (VMA) library to allocate memory for the vertex buffer and staging buffer.
		// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator

		VmaAllocationCreateInfo stagign_buffer_allocate_create_info = {};

		stagign_buffer_allocate_create_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		stagign_buffer_allocate_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer staging_vertex_buffer = VK_NULL_HANDLE;
		VmaAllocation staging_vertex_buffer_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo staging_vertex_buffer_allocation_info = {};


		// Create the staging buffer.
		VkResult result = vmaCreateBuffer(vma_allocator, &staging_buffer_create_info, &stagign_buffer_allocate_create_info, &staging_vertex_buffer, &staging_vertex_buffer_allocation, &staging_vertex_buffer_allocation_info);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Copy the vertex data to the staging bufer.
		std::memcpy(staging_vertex_buffer_allocation_info.pMappedData, vertices.data(), vertex_buffer_size);

		// No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.


		// Step 2: Create vertex buffer.
		VkBufferCreateInfo vertex_buffer_create_info = { };

		vertex_buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertex_buffer_create_info.size        = vertex_buffer_size;
		vertex_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vertex_buffer_create_info.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		VmaAllocationCreateInfo vertex_buffer_allocation_create_info = {};

		vertex_buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vertex_buffer_allocation_create_info.flags = 0;
		
		VkBuffer vertex_buffer = VK_NULL_HANDLE;
		VmaAllocation vertex_buffer_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo vertex_buffer_allocation_info = {};


		result = vmaCreateBuffer(vma_allocator, &vertex_buffer_create_info, &vertex_buffer_allocation_create_info, &vertex_buffer, &vertex_buffer_allocation, &vertex_buffer_allocation_info);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}


		// Step 3: Create a command buffer for uploading the vertices to the GPU memory.
		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

		command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool        = data_transfer_command_pool;
		command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1;
		
		VkCommandBuffer temporary_command_buffer;

		result = vkAllocateCommandBuffers(vulkan_device, &command_buffer_allocate_info, &temporary_command_buffer);
		vulkan_error_check(result);


		// Step 4: Define a copy command for the command queue.
		VkBufferCopy copy_region = {};

		copy_region.srcOffset = 0;
		copy_region.dstOffset = 0;
		copy_region.size      = vertex_buffer_create_info.size;


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

		result = vkBeginCommandBuffer(temporary_command_buffer, &cmd_buffer_begin_info);
		vulkan_error_check(result);

		vkCmdCopyBuffer(temporary_command_buffer, staging_vertex_buffer, vertex_buffer, 1, &copy_region);


		// Step 5: Submit this command to the GPU.
		result = vkEndCommandBuffer(temporary_command_buffer);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}


		// TODO: Refactor this!
		VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

		submit_info.commandBufferCount   = 1;
		submit_info.pCommandBuffers      = &temporary_command_buffer;

		// TODO: Add VkFence! For no we will use vkQueueWaitIdle.
		result = vkQueueSubmit(data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Wait until copying memory is done!
		result = vkQueueWaitIdle(data_transfer_queue);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}


		// Step 6: Store the vertex buffer as output.
		target_vertex_buffer.vertex_buffer                        = vertex_buffer;
		target_vertex_buffer.vertex_buffer_create_info            = vertex_buffer_create_info;
		target_vertex_buffer.vertex_buffer_allocation             = vertex_buffer_allocation;
		target_vertex_buffer.vertex_buffer_allocation_info        = vertex_buffer_allocation_info;
		target_vertex_buffer.vertex_buffer_allocation_create_info = vertex_buffer_allocation_create_info;
		target_vertex_buffer.number_of_vertices                   = static_cast<uint32_t>(vertices.size());


		// Don't forget to declare that there is no index buffer for this vertex buffer!
		target_vertex_buffer.index_buffer_available = false;


		// Add this vertex buffer to the list.
		list_of_vertex_buffers.push_back(target_vertex_buffer);


		// Step 7: Destroy the staging buffer and its memory!
		vmaDestroyBuffer(vma_allocator, staging_vertex_buffer, staging_vertex_buffer_allocation);

		return result;
	}


	
	VkResult VulkanVertexBufferManager::create_vertex_buffer_with_index_buffer(const VmaAllocator& vma_allocator, const VkQueue& data_transfer_queue, const std::vector<InexorVertex>& vertices, const std::vector<uint32_t> indices, InexorVertexBuffer& target_vertex_buffer)
	{
		// TODO: Refactor all of this!

		assert(vma_allocator);
		assert(vulkan_device);
		assert(data_transfer_command_pool);

		assert(vertices.size() > 0);
		assert(indices.size() > 0);

		// In general, it is inefficient to use normal memory mapping to a vertex buffer.
		// It is highly advised to use a staging buffer which will be filled with the vertex data.
		// Once the staging buffer is filled, a queue command can be executed to use a transfer queue
		// to upload the data to the GPU memory.
		
		// Calculate the size of the vertex buffer and the index buffer.
		std::size_t vertex_buffer_size = sizeof(InexorVertex) * vertices.size();
		std::size_t index_buffer_size = sizeof(InexorVertex) * indices.size();



		// Create staging vertex buffer.
		VkBufferCreateInfo staging_vertex_buffer_create_info = {};

		staging_vertex_buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		staging_vertex_buffer_create_info.size        = vertex_buffer_size;
		staging_vertex_buffer_create_info.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		staging_vertex_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo stagign_vertex_buffer_allocation_create_info = {};

		stagign_vertex_buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		stagign_vertex_buffer_allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer staging_vertex_buffer = VK_NULL_HANDLE;
		VmaAllocation staging_vertex_buffer_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo staging_vertex_buffer_allocation_info = {};


		// Create the staging vertex buffer.
		VkResult result = vmaCreateBuffer(vma_allocator, &staging_vertex_buffer_create_info, &stagign_vertex_buffer_allocation_create_info, &staging_vertex_buffer, &staging_vertex_buffer_allocation, &staging_vertex_buffer_allocation_info);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Copy the vertex data to the staging vertex bufer.
		std::memcpy(staging_vertex_buffer_allocation_info.pMappedData, vertices.data(), vertex_buffer_size);

		// No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.



		// Create staging index buffer.
		VkBufferCreateInfo staging_index_buffer_create_info = {};

		staging_index_buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		staging_index_buffer_create_info.size        = index_buffer_size;
		staging_index_buffer_create_info.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		staging_index_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo staging_index_buffer_allocation_create_info = {};

		staging_index_buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		staging_index_buffer_allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

		VkBuffer staging_index_buffer = VK_NULL_HANDLE;
		VmaAllocation staging_index_buffer_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo staging_index_buffer_allocation_info = {};


		// Create the staging index buffer.
		result = vmaCreateBuffer(vma_allocator, &staging_index_buffer_create_info, &staging_index_buffer_allocation_create_info, &staging_index_buffer, &staging_index_buffer_allocation, &staging_index_buffer_allocation_info);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}


		// Copy the index data to the staging index buffer.
		std::memcpy(staging_index_buffer_allocation_info.pMappedData, indices.data(), index_buffer_size);


		
		// Create vertex buffer.
		VkBufferCreateInfo vertex_buffer_create_info = {};

		vertex_buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		vertex_buffer_create_info.size        = vertex_buffer_size;
		vertex_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		vertex_buffer_create_info.usage       = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

		VmaAllocationCreateInfo vertex_buffer_allocation_create_info = {};

		vertex_buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		vertex_buffer_allocation_create_info.flags = 0;
		
		VkBuffer vertex_buffer = VK_NULL_HANDLE;
		VmaAllocation vertex_buffer_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo vertex_buffer_allocation_info = {};


		// Create vertex buffer.
		result = vmaCreateBuffer(vma_allocator, &vertex_buffer_create_info, &vertex_buffer_allocation_create_info, &vertex_buffer, &vertex_buffer_allocation, &vertex_buffer_allocation_info);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}



		// Create index buffer.
		VkBufferCreateInfo index_buffer_create_info = {};

		index_buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		index_buffer_create_info.size  = index_buffer_size;
		index_buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		index_buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;


		VmaAllocationCreateInfo index_buffer_allocation_create_info = {};

		index_buffer_allocation_create_info.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		index_buffer_allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;


		VkBuffer index_buffer = VK_NULL_HANDLE;
		VmaAllocation index_buffer_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo index_buffer_allocation_info = {};


		// Create index buffer.
		result = vmaCreateBuffer(vma_allocator, &index_buffer_create_info, &index_buffer_allocation_create_info, &index_buffer, &index_buffer_allocation, &index_buffer_allocation_info);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}


		// Create a command buffer for uploading the vertices and indices to the GPU memory.
		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

		command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool        = data_transfer_command_pool;
		command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1;
		
		VkCommandBuffer temporary_command_buffer;

		result = vkAllocateCommandBuffers(vulkan_device, &command_buffer_allocate_info, &temporary_command_buffer);
		vulkan_error_check(result);


		VkBufferCopy vertex_buffer_copy_region = {};

		vertex_buffer_copy_region.srcOffset = 0;
		vertex_buffer_copy_region.dstOffset = 0;
		vertex_buffer_copy_region.size      = vertex_buffer_create_info.size;


		VkBufferCopy index_buffer_copy_region = {};

		index_buffer_copy_region.srcOffset = 0;
		index_buffer_copy_region.dstOffset = 0;
		index_buffer_copy_region.size      = index_buffer_create_info.size;


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

		result = vkBeginCommandBuffer(temporary_command_buffer, &cmd_buffer_begin_info);
		vulkan_error_check(result);

		vkCmdCopyBuffer(temporary_command_buffer, staging_vertex_buffer, vertex_buffer, 1, &vertex_buffer_copy_region);
		vkCmdCopyBuffer(temporary_command_buffer, staging_index_buffer, index_buffer, 1, &index_buffer_copy_region);

		// End command buffer recording.
		result = vkEndCommandBuffer(temporary_command_buffer);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}


		// TODO: submit_buffer_copy_command();


		// TODO: Refactor this!
		VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

		submit_info.commandBufferCount   = 1;
		submit_info.pCommandBuffers      = &temporary_command_buffer;

		// TODO: Add VkFence! For no we will use vkQueueWaitIdle.
		result = vkQueueSubmit(data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Wait until copying memory is done!
		result = vkQueueWaitIdle(data_transfer_queue);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}


		// Store the vertex buffer and index buffer as output.
		target_vertex_buffer.vertex_buffer                        = vertex_buffer;
		target_vertex_buffer.vertex_buffer_create_info            = vertex_buffer_create_info;
		target_vertex_buffer.vertex_buffer_allocation             = vertex_buffer_allocation;
		target_vertex_buffer.vertex_buffer_allocation_info        = vertex_buffer_allocation_info;
		target_vertex_buffer.vertex_buffer_allocation_create_info = vertex_buffer_allocation_create_info;
		target_vertex_buffer.number_of_vertices                   = static_cast<uint32_t>(vertices.size());

		// Don't forget to declare that there is no index buffer for this vertex buffer!
		target_vertex_buffer.index_buffer_available = true;

		target_vertex_buffer.index_buffer = index_buffer;
		target_vertex_buffer.index_buffer_create_info = index_buffer_create_info;
		target_vertex_buffer.index_buffer_allocation = index_buffer_allocation;
		target_vertex_buffer.index_buffer_allocation_info = index_buffer_allocation_info;
		target_vertex_buffer.number_of_indices = static_cast<uint32_t>(indices.size());


		// Add this buffer to the list.
		list_of_vertex_buffers.push_back(target_vertex_buffer);


		// Destroy staging vertex buffer and its memory!
		vmaDestroyBuffer(vma_allocator, staging_vertex_buffer, staging_vertex_buffer_allocation);
		
		// Destroy staging index buffer and its memory!
		vmaDestroyBuffer(vma_allocator, staging_index_buffer, staging_index_buffer_allocation);

		return result;
	}


	void VulkanVertexBufferManager::shutdown_vertex_buffers(const VmaAllocator& vma_allocator)
	{
		// Loop through all vertex buffers and release their memoy.
		for(const auto& vertex_buffer : list_of_vertex_buffers)
		{
			// Destroy vertex buffer.		
			vmaDestroyBuffer(vma_allocator, vertex_buffer.vertex_buffer, vertex_buffer.vertex_buffer_allocation);

			// Destroy index buffer if existent.
			if(vertex_buffer.index_buffer_available)
			{
				vmaDestroyBuffer(vma_allocator, vertex_buffer.index_buffer, vertex_buffer.index_buffer_allocation);
			}
		}

		vkDestroyCommandPool(vulkan_device, data_transfer_command_pool, nullptr);
	}


};
};
