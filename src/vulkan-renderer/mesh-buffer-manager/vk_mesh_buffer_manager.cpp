#include "vk_mesh_buffer_manager.hpp"
#include "../error-handling/vk_error_handling.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	InexorMeshBufferManager::InexorMeshBufferManager()
	{
	}

	
	InexorMeshBufferManager::~InexorMeshBufferManager()
	{
	}

	
	VkResult InexorMeshBufferManager::initialise(const VkDevice& device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager,  const VmaAllocator& vma_allocator, const uint32_t& transfer_queue_family_index, const VkQueue& data_transfer_queue)
	{
		assert(device);
		assert(vma_allocator);
		assert(data_transfer_queue);
		assert(debug_marker_manager);

		this->device = device;
		this->vma_allocator = vma_allocator;
		this->data_transfer_queue = data_transfer_queue;
		this->debug_marker_manager = debug_marker_manager;
		
		spdlog::debug("Initialising Vulkan mesh buffer manager.");
		spdlog::debug("Creating command pool for mesh buffer manager.");

		VkCommandPoolCreateInfo command_pool_create_info = {};

		command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		command_pool_create_info.pNext = nullptr;
		command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		// This might be a distinct data transfer queue exclusively offers VK
		command_pool_create_info.queueFamilyIndex = transfer_queue_family_index;

		// Create a second command pool for all commands that are going to be executed in the data transfer queue.
		VkResult result = vkCreateCommandPool(device, &command_pool_create_info, nullptr, &data_transfer_command_pool);
		vulkan_error_check(result);
		
		// 
		debug_marker_manager->set_object_name(device, (uint64_t)(data_transfer_command_pool), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, "Command pool for VulkanMeshBufferManager.");
		
		spdlog::debug("Creating command pool for mesh buffer manager.");
		
		VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

		command_buffer_allocate_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		command_buffer_allocate_info.commandPool        = data_transfer_command_pool;
		command_buffer_allocate_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		command_buffer_allocate_info.commandBufferCount = 1;

		spdlog::debug("Allocating command buffers for mesh buffer manager.");

		// Allocate a command buffer for data transfer commands.
		result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &data_transfer_command_buffer);
		vulkan_error_check(result);
		
		// 
		debug_marker_manager->set_object_name(device, (uint64_t)(data_transfer_command_buffer), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, "Command buffer for VulkanMeshBufferManager.");

		return result;
	}


	VkResult InexorMeshBufferManager::create_buffer(std::string buffer_description, InexorBuffer& buffer_object, const VkDeviceSize& buffer_size, const VkBufferUsageFlags& buffer_usage, const VmaMemoryUsage& memory_usage)
	{
		assert(vma_allocator);
		assert(debug_marker_manager);
		
		spdlog::debug("Creating a mesh buffer.");

		buffer_object.create_info.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		buffer_object.create_info.size        = buffer_size;
		buffer_object.create_info.usage       = buffer_usage;
		buffer_object.create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		buffer_object.allocation_create_info.usage     = memory_usage;
		buffer_object.allocation_create_info.flags     = VMA_ALLOCATION_CREATE_MAPPED_BIT|VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
		buffer_object.allocation_create_info.pUserData = buffer_description.data();

		VkResult result = vmaCreateBuffer(vma_allocator, &buffer_object.create_info, &buffer_object.allocation_create_info, &buffer_object.buffer, &buffer_object.allocation, &buffer_object.allocation_info);
		vulkan_error_check(result);

		return result;
	}


	VkResult InexorMeshBufferManager::upload_data_to_gpu()
	{
		assert(data_transfer_queue);
		assert(debug_marker_manager);

		// TODO: Add debug markers?

		spdlog::debug("Uploading mesh data from CPU to GPU using staging buffers.");

		VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &data_transfer_command_buffer;

		// TODO: Add VkFence! For no we will use vkQueueWaitIdle.
		VkResult result = vkQueueSubmit(data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Wait until copying memory is done!
		result = vkQueueWaitIdle(data_transfer_queue);
		vulkan_error_check(result);

		spdlog::debug("Uploading finished.");

		return result;
	}


	VkResult InexorMeshBufferManager::create_vertex_buffer(const std::string& internal_buffer_name, const std::vector<InexorVertex>& vertices, std::vector<InexorMeshBuffer>& mesh_buffers)
	{
		assert(vertices.size() > 0);
		assert(vma_allocator);
		assert(data_transfer_command_pool);
		assert(debug_marker_manager);

		spdlog::debug("Creating new mesh buffer for {} vertices.", vertices.size());

		spdlog::warn("This vertex buffer doesn't have an associated index buffer!");
		spdlog::warn("Using index buffers can improve performance significantly!");


		// In general, it is inefficient to use normal memory mapping to a vertex buffer.
		// It is highly advised to use a staging buffer which will be filled with the vertex data.
		// Once the staging buffer is filled, a queue command can be executed to use a transfer queue
		// to upload the data to the GPU memory.

		spdlog::debug("Creating staging vertex buffer.");

		// Create a staging vertex buffer.
		InexorBuffer staging_vertex_buffer;

		// Calculate the size of the vertex buffer and the index buffer.
		std::size_t vertex_buffer_size = sizeof(InexorVertex) * vertices.size();

		std::string internal_staging_buffer_name = "Staging buffer for "+ internal_buffer_name;

		VkResult result = create_buffer(internal_staging_buffer_name, staging_vertex_buffer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}
		
		std::string staging_vertex_buffer_name = "Staging vertex buffer '"+ internal_buffer_name +"'";

		// 
		debug_marker_manager->set_object_name(device, (uint64_t)(staging_vertex_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, staging_vertex_buffer_name.c_str());

		spdlog::debug("Copying mesh data from RAM to staging vertex buffer.");

		// Copy the vertex data to the staging vertex bufer.
		std::memcpy(staging_vertex_buffer.allocation_info.pMappedData, vertices.data(), vertex_buffer_size);

		// No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.
		
		spdlog::debug("Creating vertex buffer.");

		InexorBuffer vertex_buffer;

		result = create_buffer(internal_buffer_name, vertex_buffer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}
		
		std::string vertex_buffer_name = "Vertex buffer '"+ internal_buffer_name +"'";

		// Give this vertex buffer an appropriate name.
		debug_marker_manager->set_object_name(device, (uint64_t)(vertex_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, vertex_buffer_name.c_str());

		spdlog::debug("Specifying copy region of staging vertex buffer.");

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

		spdlog::debug("Beginning command buffer recording for copy command.");

		result = vkBeginCommandBuffer(data_transfer_command_buffer, &cmd_buffer_begin_info);
		vulkan_error_check(result);

		spdlog::debug("Specifying copy operation in command buffer.");
		vkCmdCopyBuffer(data_transfer_command_buffer, staging_vertex_buffer.buffer, vertex_buffer.buffer, 1, &vertex_buffer_copy_region);

		spdlog::debug("Ending command buffer recording for copy command.");

		// End command buffer recording.
		result = vkEndCommandBuffer(data_transfer_command_buffer);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}
		

		// Submit buffer copy command to data transfer queue.
		upload_data_to_gpu();
		
		spdlog::debug("Storing mesh buffer in output.");

		InexorMeshBuffer new_mesh_buffer;

		// Store the vertex buffer.
		new_mesh_buffer.vertex_buffer = vertex_buffer;

		// Yes, there is an index buffer available!
		new_mesh_buffer.index_buffer_available = false;

		// Store the number of vertices and indices.
		new_mesh_buffer.number_of_vertices = static_cast<uint32_t>(vertices.size());

		// Store the internal description of this buffer.
		new_mesh_buffer.description = internal_buffer_name;

		mesh_buffers.push_back(new_mesh_buffer);

		// Add this buffer to the list.
		list_of_meshes.push_back(new_mesh_buffer);
		
		spdlog::debug("Destroying staging vertex buffer.");

		// Destroy staging vertex buffer and its memory!
		vmaDestroyBuffer(vma_allocator, staging_vertex_buffer.buffer, staging_vertex_buffer.allocation);

		return result;
	}

	
	VkResult InexorMeshBufferManager::create_vertex_buffer_with_index_buffer(const std::string& internal_buffer_name, const std::vector<InexorVertex>& vertices, const std::vector<uint32_t> indices, std::vector<InexorMeshBuffer>& mesh_buffers)
	{
		assert(indices.size() > 0);
		assert(vertices.size() > 0);
		assert(vma_allocator);
		assert(data_transfer_command_pool);
		assert(data_transfer_command_buffer);
		assert(debug_marker_manager);

		// In general, it is inefficient to use normal memory mapping to a vertex buffer.
		// It is highly advised to use a staging buffer which will be filled with the vertex data.
		// Once the staging buffer is filled, a queue command can be executed to use a transfer queue
		// to upload the data to the GPU memory.
		
		// Calculate the size of the vertex buffer and the index buffer.
		std::size_t vertex_buffer_size = sizeof(InexorVertex) * vertices.size();
		std::size_t index_buffer_size  = sizeof(InexorVertex) * indices.size();
		
		spdlog::debug("Creating new mesh buffer for {} vertices.", vertex_buffer_size);
		spdlog::debug("Creating new mesh buffer for {} indices.",  index_buffer_size);

		spdlog::debug("Creating staging vertex buffer for {}.", internal_buffer_name);

		// Create a staging vertex buffer.
		InexorBuffer staging_vertex_buffer;

		VkResult result = create_buffer(internal_buffer_name, staging_vertex_buffer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Give this staging buffer an appropriate name.
		std::string staging_vertex_buffer_name = "Staging vertex buffer "+ internal_buffer_name;

		debug_marker_manager->set_object_name(device, (uint64_t)(staging_vertex_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, staging_vertex_buffer_name.c_str());
		
		spdlog::debug("Copying mesh data from RAM to staging index buffer for {}.", internal_buffer_name);

		// Copy the vertex data to the staging vertex bufer.
		std::memcpy(staging_vertex_buffer.allocation_info.pMappedData, vertices.data(), vertex_buffer_size);

		// No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.

		spdlog::debug("Creating staging index buffer for {}.", internal_buffer_name);

		InexorBuffer staging_index_buffer;

		result = create_buffer(internal_buffer_name, staging_index_buffer, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		std::string staging_index_buffer_name = "Staging index buffer '"+ internal_buffer_name +"'.";

		debug_marker_manager->set_object_name(device, (uint64_t)(staging_index_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, staging_index_buffer_name.c_str());
		
		spdlog::debug("Copying mesh data from RAM to staging index buffer for {}.", internal_buffer_name);

		// Copy the index data to the staging index buffer.
		std::memcpy(staging_index_buffer.allocation_info.pMappedData, indices.data(), index_buffer_size);

		InexorBuffer vertex_buffer;

		result = create_buffer(internal_buffer_name, vertex_buffer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}
		
		// Give this staging buffer an appropriate name.
		std::string vertex_buffer_name = "Vertex buffer '"+ internal_buffer_name +"'.";

		debug_marker_manager->set_object_name(device, (uint64_t)(staging_vertex_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, vertex_buffer_name.c_str());

		InexorBuffer index_buffer;
		
		result = create_buffer(internal_buffer_name, index_buffer, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT|VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}
		
		std::string index_buffer_name = "Index buffer '"+ internal_buffer_name +"'.";

		debug_marker_manager->set_object_name(device, (uint64_t)(staging_index_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, index_buffer_name.c_str());

		spdlog::debug("Specifying copy region of staging vertex buffer for {}.", internal_buffer_name);

		VkBufferCopy vertex_buffer_copy_region = {};

		vertex_buffer_copy_region.srcOffset = 0;
		vertex_buffer_copy_region.dstOffset = 0;
		vertex_buffer_copy_region.size      = vertex_buffer.create_info.size;
		

		spdlog::debug("Specifying copy region of staging index buffer for {}", internal_buffer_name);

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
		
		spdlog::debug("Beginning command buffer recording for copy commands.");

		result = vkBeginCommandBuffer(data_transfer_command_buffer, &cmd_buffer_begin_info);
		vulkan_error_check(result);
		
		spdlog::debug("Specifying vertex buffer copy operation in command buffer.");
		
		vkCmdCopyBuffer(data_transfer_command_buffer, staging_vertex_buffer.buffer, vertex_buffer.buffer, 1, &vertex_buffer_copy_region);
		
		debug_marker_manager->set_object_name(device, (uint64_t)(vertex_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, vertex_buffer_name.c_str());

		spdlog::debug("Specifying index buffer copy operation in command buffer.");
		
		vkCmdCopyBuffer(data_transfer_command_buffer, staging_index_buffer.buffer, index_buffer.buffer, 1, &index_buffer_copy_region);
		
		debug_marker_manager->set_object_name(device, (uint64_t)(index_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, index_buffer_name.c_str());

		spdlog::debug("Ending command buffer recording for copy commands.");

		// End command buffer recording.
		result = vkEndCommandBuffer(data_transfer_command_buffer);
		if(VK_SUCCESS != result)
		{
			vulkan_error_check(result);
			return result;
		}

		// Submit buffer copy command to data transfer queue.
		upload_data_to_gpu();

		spdlog::debug("Storing mesh buffer in output.");

		InexorMeshBuffer new_mesh_buffer;

		// Store the vertex buffer.
		new_mesh_buffer.vertex_buffer = vertex_buffer;

		// Yes, there is an index buffer available!
		new_mesh_buffer.index_buffer_available = true;

		// Store the index buffer.
		new_mesh_buffer.index_buffer = index_buffer;

		// Store the number of vertices and indices.
		new_mesh_buffer.number_of_vertices = static_cast<uint32_t>(vertices.size());
		new_mesh_buffer.number_of_indices  = static_cast<uint32_t>(indices.size());

		// Store the internal description of this buffer.
		new_mesh_buffer.description = internal_buffer_name;

		// Add this buffer to he global list of meshes.
		mesh_buffers.push_back(new_mesh_buffer);

		// Add this buffer to the list.
		list_of_meshes.push_back(new_mesh_buffer);

		spdlog::debug("Destroying staging vertex buffer.");

		// Destroy staging vertex buffer and its memory!
		vmaDestroyBuffer(vma_allocator, staging_vertex_buffer.buffer, staging_vertex_buffer.allocation);
		
		spdlog::debug("Destroying staging index buffer.");

		// Destroy staging index buffer and its memory!
		vmaDestroyBuffer(vma_allocator, staging_index_buffer.buffer, staging_index_buffer.allocation);

		return result;
	}


	void InexorMeshBufferManager::shutdown_vertex_buffers()
	{
		assert(device);
		assert(vma_allocator);
		assert(data_transfer_command_pool);

		// Loop through all vertex buffers and release their memoy.
		for(const auto& mesh_buffer : list_of_meshes)
		{
			spdlog::debug("Destroying vertex buffer {}.", mesh_buffer.description);

			// Destroy vertex buffer.
			vmaDestroyBuffer(vma_allocator, mesh_buffer.vertex_buffer.buffer, mesh_buffer.vertex_buffer.allocation);
			
			// Destroy index buffer if existent.
			if(mesh_buffer.index_buffer_available)
			{
				spdlog::debug("Destroying index buffer {}.", mesh_buffer.description);

				// Destroy corresponding index buffer.
				vmaDestroyBuffer(vma_allocator, mesh_buffer.index_buffer.buffer, mesh_buffer.index_buffer.allocation);
			}
		}

		spdlog::debug("Destroying command pool for VulkanMeshBufferManager.");

		list_of_meshes.clear();

		vkDestroyCommandPool(device, data_transfer_command_pool, nullptr);
	}


};
};
