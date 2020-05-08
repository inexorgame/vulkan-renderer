#include "inexor/vulkan-renderer/mesh_buffer.hpp"

namespace inexor::vulkan_renderer {
MeshBuffer::MeshBuffer(MeshBuffer &&other) noexcept
    : name(std::move(other.name)), vertex_buffer(std::move(other.vertex_buffer)), index_buffer(std::move(other.index_buffer)),
      number_of_vertices(other.number_of_vertices), number_of_indices(other.number_of_indices), copy_command_buffer(std::move(other.copy_command_buffer)) {}

MeshBuffer::MeshBuffer(const VkDevice device, VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index,
                       const VmaAllocator vma_allocator, std::string name, const VkDeviceSize size_of_vertex_structure, const std::uint32_t number_of_vertices,
                       void *vertices, const VkDeviceSize size_of_index_structure, const std::uint32_t number_of_indices, void *indices)

    // It's no problem to create the vertex buffer and index buffer before the corresponding staging buffers are created!.
    : vertex_buffer(device, vma_allocator, name, size_of_vertex_structure * number_of_vertices,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY),
      index_buffer(GPUMemoryBuffer(device, vma_allocator, name, size_of_index_structure * number_of_vertices,
                                   VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY)),
      copy_command_buffer(device, data_transfer_queue, data_transfer_queue_family_index) {
    assert(device);
    assert(vma_allocator);
    assert(!name.empty());
    assert(size_of_vertex_structure > 0);

    std::size_t vertex_buffer_size = size_of_vertex_structure * number_of_vertices;
    std::size_t index_buffer_size = size_of_index_structure * number_of_indices;

    spdlog::debug("Creating vertex buffer of size {} for mesh {}.", vertex_buffer_size, name);
    spdlog::debug("Creating index buffer of size {} for mesh {}.", index_buffer_size, name);

    // Not using an index buffer can decrease performance drastically!
    if (index_buffer_size == 0) {
        spdlog::warn("Size of index buffer is 0!");
        spdlog::warn("Always use an index buffer if possible! Not using an index buffer decreases performance drastically!");
    }

    VkDeviceSize vertices_memory_size = number_of_vertices * size_of_vertex_structure;

    StagingBuffer staging_buffer_for_vertices(device, vma_allocator, copy_command_buffer.get_command_buffer(), data_transfer_queue,
                                              data_transfer_queue_family_index, name, vertex_buffer_size, vertices, vertices_memory_size);

    staging_buffer_for_vertices.upload_data_to_gpu(vertex_buffer);

    if (number_of_indices > 0) {
        VkDeviceSize indices_memory_size = number_of_indices * size_of_index_structure;

        StagingBuffer staging_buffer_for_indices(device, vma_allocator, copy_command_buffer.get_command_buffer(), data_transfer_queue,
                                                 data_transfer_queue_family_index, name, index_buffer_size, indices, indices_memory_size);

        staging_buffer_for_indices.upload_data_to_gpu(index_buffer.value());
    } else {
        spdlog::warn("No index buffer created for mesh {}", name);
    }
}

MeshBuffer::MeshBuffer(const VkDevice device, VkQueue data_transfer_queue, const std::uint32_t data_transfer_queue_family_index,
                       const VmaAllocator vma_allocator, std::string name, const VkDeviceSize size_of_vertex_structure, const std::uint32_t number_of_vertices,
                       void *vertices)
    // It's no problem to create the vertex buffer and index buffer before the corresponding staging buffers are created!.
    : vertex_buffer(device, vma_allocator, name, size_of_vertex_structure * number_of_vertices,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY),
      index_buffer(std::nullopt), copy_command_buffer(device, data_transfer_queue, data_transfer_queue_family_index), number_of_vertices(number_of_vertices),
      number_of_indices(number_of_indices) {
    assert(device);
    assert(vma_allocator);
    assert(!name.empty());
    assert(size_of_vertex_structure > 0);

    std::size_t vertex_buffer_size = size_of_vertex_structure * number_of_vertices;

    spdlog::debug("Creating vertex buffer of size {} for mesh {}.", vertex_buffer_size, name);

    // Not using an index buffer can decrease performance drastically!
    spdlog::warn("Creating a vertex buffer without an index buffer!");
    spdlog::warn("Always use an index buffer if possible. The performance will decrease drastically otherwise!");

    VkDeviceSize vertices_memory_size = number_of_vertices * size_of_vertex_structure;

    StagingBuffer staging_buffer_for_vertices(device, vma_allocator, copy_command_buffer.get_command_buffer(), data_transfer_queue,
                                              data_transfer_queue_family_index, name, vertex_buffer_size, vertices, vertices_memory_size);

    staging_buffer_for_vertices.upload_data_to_gpu(vertex_buffer);
}

MeshBuffer::~MeshBuffer() {}

} // namespace inexor::vulkan_renderer
