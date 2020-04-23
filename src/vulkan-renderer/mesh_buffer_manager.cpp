#include "inexor/vulkan-renderer/mesh_buffer_manager.hpp"

namespace inexor::vulkan_renderer {

VkResult MeshBufferManager::init(const VkDevice &device, const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager,
                                 const VmaAllocator &vma_allocator, const uint32_t data_transfer_queue_family_index, const VkQueue &data_transfer_queue) {
    assert(device);
    assert(vma_allocator);
    assert(data_transfer_queue);
    assert(debug_marker_manager);

    std::shared_lock<std::shared_mutex> lock(mesh_buffer_manager_mutex);

    this->device = device;
    this->vma_allocator = vma_allocator;
    this->data_transfer_queue = data_transfer_queue;
    this->debug_marker_manager = debug_marker_manager;
    this->data_transfer_queue_family_index = data_transfer_queue_family_index;

    spdlog::debug("Initialising Vulkan mesh buffer manager.");
    spdlog::debug("Creating command pool for mesh buffer manager.");

    create_command_pool();

    mesh_buffer_manager_initialised = true;

    return VK_SUCCESS;
}

VkResult MeshBufferManager::create_buffer(std::string buffer_description, Buffer &buffer_object, const VkDeviceSize &buffer_size,
                                          const VkBufferUsageFlags &buffer_usage, const VmaMemoryUsage &memory_usage) {
    assert(mesh_buffer_manager_initialised);
    assert(vma_allocator);
    assert(debug_marker_manager);
    assert(buffer_size > 0);
    assert(!buffer_description.empty());

    spdlog::debug("Creating a buffer of size {} for vertex data of '{}'.", buffer_size, buffer_description);

    buffer_object.create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_object.create_info.size = buffer_size;
    buffer_object.create_info.usage = buffer_usage;
    buffer_object.create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buffer_object.allocation_create_info.usage = memory_usage;
    buffer_object.allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    buffer_object.allocation_create_info.pUserData = buffer_description.data();

    VkResult result = vmaCreateBuffer(vma_allocator, &buffer_object.create_info, &buffer_object.allocation_create_info, &buffer_object.buffer,
                                      &buffer_object.allocation, &buffer_object.allocation_info);
    vulkan_error_check(result);

    return result;
}

VkResult MeshBufferManager::create_command_pool() {
    VkCommandPoolCreateInfo command_pool_create_info = {};

    command_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    command_pool_create_info.pNext = nullptr;
    command_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    // This might be a distinct data transfer queue exclusively offers VK
    command_pool_create_info.queueFamilyIndex = data_transfer_queue_family_index;

    // Create a second command pool for all commands that are going to be executed in the data transfer queue.
    VkResult result = vkCreateCommandPool(device, &command_pool_create_info, nullptr, &data_transfer_command_pool);
    vulkan_error_check(result);

    //
    debug_marker_manager->set_object_name(device, (uint64_t)(data_transfer_command_pool), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT,
                                          "Command pool for VulkanMeshBufferManager.");

    spdlog::debug("Creating command pool for mesh buffer manager.");

    VkCommandBufferAllocateInfo command_buffer_allocate_info = {};

    command_buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_allocate_info.commandPool = data_transfer_command_pool;
    command_buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_allocate_info.commandBufferCount = 1;

    spdlog::debug("Allocating command buffers for mesh buffer manager.");

    // Allocate a command buffer for data transfer commands.
    result = vkAllocateCommandBuffers(device, &command_buffer_allocate_info, &data_transfer_command_buffer);
    vulkan_error_check(result);

    //
    debug_marker_manager->set_object_name(device, (uint64_t)(data_transfer_command_buffer), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT,
                                          "Command buffer for VulkanMeshBufferManager.");

    return VK_SUCCESS;
}

VkResult MeshBufferManager::upload_data_to_gpu() {
    assert(mesh_buffer_manager_initialised);
    assert(data_transfer_queue);
    assert(debug_marker_manager);

    // TODO: Add debug markers?

    spdlog::debug("Uploading mesh data from CPU to GPU using staging buffers.");

    VkSubmitInfo submit_info = {VK_STRUCTURE_TYPE_SUBMIT_INFO};

    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &data_transfer_command_buffer;

    // TODO: Add VkFence! For no we will use vkQueueWaitIdle.
    VkResult result = vkQueueSubmit(data_transfer_queue, 1, &submit_info, VK_NULL_HANDLE);
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return result;
    }

    // Wait until copying memory is done!
    result = vkQueueWaitIdle(data_transfer_queue);
    vulkan_error_check(result);

    spdlog::debug("Uploading finished.");

    return result;
}

VkResult MeshBufferManager::create_vertex_buffer(const std::string &internal_mesh_buffer_name, const void *vertices, const std::size_t size_of_vertex_structure,
                                                 const std::size_t number_of_vertices, std::shared_ptr<MeshBuffer> &output_mesh_buffer) {
    assert(mesh_buffer_manager_initialised);
    assert(size_of_vertex_structure > 0);
    assert(number_of_vertices > 0);
    assert(!internal_mesh_buffer_name.empty());
    assert(vertices);

    spdlog::debug("Creating new mesh buffer '{}' using {} vertices.", internal_mesh_buffer_name, number_of_vertices);

    spdlog::warn("This vertex buffer doesn't have an associated index buffer!");
    spdlog::warn("Using index buffers can improve performance significantly!");

    // Check if a mesh buffer with this name does already exist.
    if (does_key_exist(internal_mesh_buffer_name)) {
        spdlog::debug("A mesh buffer with the name '{}' does already exist!", internal_mesh_buffer_name);
        output_mesh_buffer = nullptr;
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // In general, it is inefficient to use normal memory mapping to a vertex buffer.
    // It is highly advised to use a staging buffer which will be filled with the vertex data.
    // Once the staging buffer is filled, a queue command can be executed to use a transfer queue
    // to upload the data to the GPU memory.

    spdlog::debug("Creating staging buffer for vertex data.");

    // TODO: Implement a StagingBuffer class?

    // Create a staging vertex buffer.
    Buffer staging_vertex_buffer;

    std::size_t vertex_buffer_size = size_of_vertex_structure * number_of_vertices;

    std::string internal_staging_buffer_name = "Staging buffer for " + internal_mesh_buffer_name;

    VkResult result =
        create_buffer(internal_staging_buffer_name, staging_vertex_buffer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return result;
    }

    std::string staging_vertex_buffer_name = "Staging vertex buffer '" + internal_mesh_buffer_name + "'.";

    //
    debug_marker_manager->set_object_name(device, (uint64_t)(staging_vertex_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                                          staging_vertex_buffer_name.c_str());

    spdlog::debug("Copying mesh data from RAM to staging buffer for vertex data");

    // Copy the vertex data to the staging vertex bufer.
    std::memcpy(staging_vertex_buffer.allocation_info.pMappedData, vertices, vertex_buffer_size);

    // No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.

    // TODO: Do we need to unmap memory now?

    spdlog::debug("Creating vertex buffer.");

    Buffer vertex_buffer;

    result = create_buffer(internal_mesh_buffer_name, vertex_buffer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                           VMA_MEMORY_USAGE_CPU_ONLY);
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return result;
    }

    std::string vertex_buffer_name = "Vertex buffer '" + internal_mesh_buffer_name + "'";

    // Give this vertex buffer an appropriate name.
    debug_marker_manager->set_object_name(device, (uint64_t)(vertex_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, vertex_buffer_name.c_str());

    spdlog::debug("Specifying copy region of vertex data's staging buffer.");

    VkBufferCopy vertex_buffer_copy_region = {};

    vertex_buffer_copy_region.srcOffset = 0;
    vertex_buffer_copy_region.dstOffset = 0;
    vertex_buffer_copy_region.size = vertex_buffer.create_info.size;

    // It should be noted that it is more efficient to use queues which are specifically designed for this task.
    // We need to look for queues which have VK_QUEUE_TRANSFER_BIT, but not VK_QUEUE_GRAPHICS_BIT!
    // In some talks about Vulkan it was mentioned that not using dedicated transfer queues is one of the biggest mistakes when using Vulkan.
    // Copy vertex data from staging buffer to vertex buffer to upload it to GPU memory!

    VkCommandBufferBeginInfo cmd_buffer_begin_info;

    cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buffer_begin_info.pNext = nullptr;
    cmd_buffer_begin_info.pInheritanceInfo = nullptr;

    // We’re only going to use the command buffer once and wait with returning from the function until
    // the copy operation has finished executing. It’s good practice to tell the driver about our intent
    // using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    spdlog::debug("Beginning command buffer recording for copy command.");

    result = vkBeginCommandBuffer(data_transfer_command_buffer, &cmd_buffer_begin_info);
    vulkan_error_check(result);

    spdlog::debug("Specifying copy operation in command buffer.");
    vkCmdCopyBuffer(data_transfer_command_buffer, staging_vertex_buffer.buffer, vertex_buffer.buffer, 1, &vertex_buffer_copy_region);

    spdlog::debug("Ending command buffer recording for copy command.");

    // End command buffer recording.
    result = vkEndCommandBuffer(data_transfer_command_buffer);
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return result;
    }

    // Submit buffer copy command to data transfer queue.
    upload_data_to_gpu();

    spdlog::debug("Storing mesh buffer in output.");

    std::shared_ptr<MeshBuffer> new_mesh_buffer = std::make_shared<MeshBuffer>();

    // Store the vertex buffer.
    new_mesh_buffer->vertex_buffer = vertex_buffer;

    // Yes, there is an index buffer available!
    new_mesh_buffer->index_buffer_available = false;

    // Store the number of vertices and indices.
    new_mesh_buffer->number_of_vertices = static_cast<uint32_t>(number_of_vertices);

    // Store the internal description of this buffer.
    new_mesh_buffer->description = internal_mesh_buffer_name;

    output_mesh_buffer = new_mesh_buffer;

    add_entry(internal_mesh_buffer_name, new_mesh_buffer);

    spdlog::debug("Destroying staging vertex buffer.");

    // Destroy staging vertex buffer and its memory!
    vmaDestroyBuffer(vma_allocator, staging_vertex_buffer.buffer, staging_vertex_buffer.allocation);

    return VK_SUCCESS;
}

VkResult MeshBufferManager::create_vertex_buffer_with_index_buffer(const std::string &internal_mesh_buffer_name, const void *vertices,
                                                                   const std::size_t size_of_vertex_structure, const std::size_t number_of_vertices,
                                                                   const void *indices, const std::size_t size_of_index_structure,
                                                                   const std::size_t number_of_indices, std::shared_ptr<MeshBuffer> &mesh_buffer_output) {
    assert(mesh_buffer_manager_initialised);
    assert(!internal_mesh_buffer_name.empty());
    assert(vertices);
    assert(indices);
    assert(size_of_vertex_structure > 0);
    assert(size_of_index_structure > 0);
    assert(number_of_vertices > 0);
    assert(number_of_indices > 0);
    assert(vma_allocator);
    assert(data_transfer_command_pool);
    assert(data_transfer_command_buffer);
    assert(debug_marker_manager);

    // Check if a mesh buffer with this name does already exist.
    if (does_key_exist(internal_mesh_buffer_name)) {
        spdlog::debug("A mesh buffer with the name '{}' does already exist!", internal_mesh_buffer_name);
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // In general, it is inefficient to use normal memory mapping to a vertex buffer.
    // It is highly advised to use a staging buffer which will be filled with the vertex data.
    // Once the staging buffer is filled, a queue command can be executed to use a transfer queue
    // to upload the data to the GPU memory.

    // Calculate the size of the vertex buffer and the index buffer.
    std::size_t vertex_buffer_size = size_of_vertex_structure * number_of_vertices;
    std::size_t index_buffer_size = size_of_index_structure * number_of_indices;

    spdlog::debug("Creating new mesh buffer for {} vertices.", vertex_buffer_size);
    spdlog::debug("Creating new mesh buffer for {} indices.", index_buffer_size);

    spdlog::debug("Creating staging vertex buffer for '{}'.", internal_mesh_buffer_name);

    // Create a staging vertex buffer.
    Buffer staging_vertex_buffer;

    std::string staging_vertex_buffer_name = "Staging vertex buffer " + internal_mesh_buffer_name;

    VkResult result =
        create_buffer(staging_vertex_buffer_name, staging_vertex_buffer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return result;
    }

    debug_marker_manager->set_object_name(device, (uint64_t)(staging_vertex_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                                          staging_vertex_buffer_name.c_str());

    spdlog::debug("Copying mesh data from RAM to staging index buffer for {}.", internal_mesh_buffer_name);

    // Copy the vertex data to the staging vertex bufer.
    std::memcpy(staging_vertex_buffer.allocation_info.pMappedData, vertices, vertex_buffer_size);

    // No need to flush stagingVertexBuffer memory because CPU_ONLY memory is always HOST_COHERENT.

    spdlog::debug("Creating staging index buffer for {}.", internal_mesh_buffer_name);

    // TODO: StagingBuffer ?
    Buffer staging_index_buffer;

    std::string staging_index_buffer_name = "Staging index buffer '" + internal_mesh_buffer_name + "'.";

    result = create_buffer(staging_index_buffer_name, staging_index_buffer, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return result;
    }

    //
    debug_marker_manager->set_object_name(device, (uint64_t)(staging_index_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT,
                                          staging_index_buffer_name.c_str());

    spdlog::debug("Copying mesh data from RAM to staging index buffer for '{}'.", internal_mesh_buffer_name);

    // Copy the index data to the staging index buffer.
    std::memcpy(staging_index_buffer.allocation_info.pMappedData, indices, index_buffer_size);

    Buffer vertex_buffer;

    std::string vertex_buffer_name = "Vertex buffer '" + internal_mesh_buffer_name + "'.";

    // TODO: Centralize buffer management? Should we really do it?
    result = create_buffer(vertex_buffer_name, vertex_buffer, vertex_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                           VMA_MEMORY_USAGE_CPU_ONLY);
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return result;
    }

    //
    debug_marker_manager->set_object_name(device, (uint64_t)(staging_vertex_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, vertex_buffer_name.c_str());

    Buffer index_buffer;

    std::string index_buffer_name = "Index buffer '" + internal_mesh_buffer_name + "'.";

    result = create_buffer(internal_mesh_buffer_name, index_buffer, index_buffer_size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                           VMA_MEMORY_USAGE_CPU_ONLY);
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return result;
    }

    //
    debug_marker_manager->set_object_name(device, (uint64_t)(staging_index_buffer.buffer), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, index_buffer_name.c_str());

    spdlog::debug("Specifying copy region of staging vertex buffer for {}.", internal_mesh_buffer_name);

    VkBufferCopy vertex_buffer_copy_region = {};

    vertex_buffer_copy_region.srcOffset = 0;
    vertex_buffer_copy_region.dstOffset = 0;
    vertex_buffer_copy_region.size = vertex_buffer.create_info.size;

    spdlog::debug("Specifying copy region of staging index buffer for {}", internal_mesh_buffer_name);

    VkBufferCopy index_buffer_copy_region = {};

    index_buffer_copy_region.srcOffset = 0;
    index_buffer_copy_region.dstOffset = 0;
    index_buffer_copy_region.size = index_buffer.create_info.size;

    // It should be noted that it is more efficient to use queues which are specifically designed for this task.
    // We need to look for queues which have VK_QUEUE_TRANSFER_BIT, but not VK_QUEUE_GRAPHICS_BIT!
    // In some talks about Vulkan it was mentioned that not using dedicated transfer queues is one of the biggest mistakes when using Vulkan.
    // Copy vertex data from staging buffer to vertex buffer to upload it to GPU memory!

    VkCommandBufferBeginInfo cmd_buffer_begin_info;

    cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buffer_begin_info.pNext = nullptr;
    cmd_buffer_begin_info.pInheritanceInfo = nullptr;

    // We’re only going to use the command buffer once and wait with returning from the function until
    // the copy operation has finished executing. It’s good practice to tell the driver about our intent
    // using VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT.
    cmd_buffer_begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

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
    if (VK_SUCCESS != result) {
        vulkan_error_check(result);
        return result;
    }

    // Submit buffer copy command to data transfer queue.
    upload_data_to_gpu();

    spdlog::debug("Storing mesh buffer in output.");

    std::shared_ptr<MeshBuffer> new_mesh_buffer = std::make_shared<MeshBuffer>();

    // Store the vertex buffer.
    new_mesh_buffer->vertex_buffer = vertex_buffer;

    // Yes, there is an index buffer available!
    new_mesh_buffer->index_buffer_available = true;

    // Store the index buffer.
    new_mesh_buffer->index_buffer = index_buffer;

    // Store the number of vertices and indices.
    new_mesh_buffer->number_of_vertices = static_cast<uint32_t>(number_of_vertices);
    new_mesh_buffer->number_of_indices = static_cast<uint32_t>(number_of_indices);

    // Store the internal description of this buffer.
    new_mesh_buffer->description = internal_mesh_buffer_name;

    // Store the output.
    mesh_buffer_output = new_mesh_buffer;

    add_entry(internal_mesh_buffer_name, new_mesh_buffer);

    spdlog::debug("Destroying staging vertex buffer.");

    // Destroy staging buffer.
    vmaDestroyBuffer(vma_allocator, staging_vertex_buffer.buffer, staging_vertex_buffer.allocation);

    spdlog::debug("Destroying staging index buffer.");

    // Destroy staging buffer.
    vmaDestroyBuffer(vma_allocator, staging_index_buffer.buffer, staging_index_buffer.allocation);

    return result;
}

void MeshBufferManager::shutdown_vertex_and_index_buffers() {
    assert(mesh_buffer_manager_initialised);
    assert(device);
    assert(vma_allocator);
    assert(data_transfer_command_pool);

    auto list_of_meshes = get_all_values();

    // Loop through all vertex buffers and release their memoy.
    for (const auto &mesh_buffer : list_of_meshes) {
        spdlog::debug("Destroying vertex buffer for '{}'.", mesh_buffer->description);

        // Destroy vertex buffer.
        vmaDestroyBuffer(vma_allocator, mesh_buffer->vertex_buffer.buffer, mesh_buffer->vertex_buffer.allocation);

        // Destroy index buffer if existent.
        if (mesh_buffer->index_buffer_available) {
            spdlog::debug("Destroying index buffer {}.", mesh_buffer->description);

            // Destroy corresponding index buffer.
            vmaDestroyBuffer(vma_allocator, mesh_buffer->index_buffer.buffer, mesh_buffer->index_buffer.allocation);
        } else {
            spdlog::debug("There is no index buffer for vertex buffer of '{}'", mesh_buffer->description);
        }
    }

    spdlog::debug("Destroying MeshBufferManager command pool.");

    vkDestroyCommandPool(device, data_transfer_command_pool, nullptr);

    spdlog::debug("Clearing list of meshes.");

    list_of_meshes.clear();
}

} // namespace inexor::vulkan_renderer
