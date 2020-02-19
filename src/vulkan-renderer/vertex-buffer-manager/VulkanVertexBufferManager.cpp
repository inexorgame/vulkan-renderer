#include "VulkanVertexBufferManager.hpp"
#include "../error-handling/VulkanErrorHandling.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	VulkanVertexBufferManager::VulkanVertexBufferManager()
	{
	}

	
	VulkanVertexBufferManager::~VulkanVertexBufferManager()
	{
	}

	
	VkResult VulkanVertexBufferManager::create_vertex_buffer(const VmaAllocator& vma_allocator, const VkDeviceSize& memory_size, InexorVertexBuffer& vertex_buffer, const VmaMemoryUsage& memory_usage, const VmaAllocationCreateFlags& create_flags)
	{
		VkBufferCreateInfo buffer_create_info = {};

		buffer_create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_create_info.size        = memory_size;
		buffer_create_info.usage       = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VmaAllocationCreateInfo allocation_create_info = {};
		allocation_create_info.usage = memory_usage;
		allocation_create_info.flags = create_flags;

		InexorVertexBuffer new_vertex_buffer;
		
		VmaAllocationInfo allocation_info;
		VkResult result = vmaCreateBuffer(vma_allocator, &buffer_create_info, &allocation_create_info, &new_vertex_buffer.buffer, &new_vertex_buffer.allocation, &allocation_info);

		new_vertex_buffer.allocation_create_info = allocation_create_info;
		new_vertex_buffer.buffer_create_info= buffer_create_info;
		new_vertex_buffer.allocation_info = allocation_info;
		
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
		}
		else
		{
			// Add the new memory block to our list.
			list_of_vertex_buffers.push_back(new_vertex_buffer);

			// Return the created buffer.
			vertex_buffer = new_vertex_buffer;
		}
		
		return result;
	}
	
	
	void VulkanVertexBufferManager::update_vertex_buffer_memory(InexorVertexBuffer& vertex_buffer, const InexorVertex* source_memory)
	{
		std::memcpy(vertex_buffer.allocation_info.pMappedData, source_memory, vertex_buffer.buffer_create_info.size);
	}

	
	void VulkanVertexBufferManager::shutdown_vertex_buffers(const VmaAllocator& vma_allocator)
	{
		// Loop through all vertex buffers and release their memoy.
		for(const auto& vertex_buffer : list_of_vertex_buffers)
		{
			// Destroy vertex buffer.		
			vmaDestroyBuffer(vma_allocator, vertex_buffer.buffer, vertex_buffer.allocation);
		}
	}


};
};
