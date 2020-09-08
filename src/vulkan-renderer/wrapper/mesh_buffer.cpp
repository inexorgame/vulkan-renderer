#include "inexor/vulkan-renderer/wrapper/mesh_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {
MeshBuffer::MeshBuffer(MeshBuffer &&other) noexcept
    : m_name(std::move(other.m_name)), m_vertex_buffer(std::move(other.m_vertex_buffer)),
      m_index_buffer(std::exchange(other.m_index_buffer, std::nullopt)),
      m_number_of_vertices(other.m_number_of_vertices), m_number_of_indices(other.m_number_of_indices),
      m_device(other.m_device) {}

MeshBuffer::MeshBuffer(const Device &device, const std::string &name, const VkDeviceSize size_of_vertex_structure,
                       const std::size_t number_of_vertices, void *vertices, const VkDeviceSize size_of_index_structure,
                       const std::size_t number_of_indices, void *indices)

    // It's no problem to create the vertex buffer and index buffer before the corresponding staging buffers are
    // created!.
    : m_vertex_buffer(device, name, size_of_vertex_structure * number_of_vertices,
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY),
      m_index_buffer(std::make_optional<GPUMemoryBuffer>(
          device, name, size_of_index_structure * number_of_indices,
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY)),
      m_device(device) {
    assert(device.device());
    assert(device.allocator());
    assert(!name.empty());
    assert(size_of_vertex_structure > 0);

    std::size_t vertex_buffer_size = size_of_vertex_structure * number_of_vertices;
    std::size_t index_buffer_size = size_of_index_structure * number_of_indices;

    spdlog::debug("Creating vertex buffer of size {} for mesh {}.", vertex_buffer_size, name);
    spdlog::debug("Creating index buffer of size {} for mesh {}.", index_buffer_size, name);

    // Not using an index buffer can decrease performance drastically!
    if (index_buffer_size == 0) {
        spdlog::warn("Size of index buffer is 0!");
        spdlog::warn(
            "Always use an index buffer if possible! Not using an index buffer decreases performance drastically!");
    }

    VkDeviceSize vertices_memory_size = number_of_vertices * size_of_vertex_structure;

    StagingBuffer staging_buffer_for_vertices(m_device, name, vertex_buffer_size, vertices, vertices_memory_size);

    staging_buffer_for_vertices.upload_data_to_gpu(m_vertex_buffer);

    if (number_of_indices > 0) {
        VkDeviceSize indices_memory_size = number_of_indices * size_of_index_structure;

        StagingBuffer staging_buffer_for_indices(m_device, name, index_buffer_size, indices, indices_memory_size);

        staging_buffer_for_indices.upload_data_to_gpu(*m_index_buffer);
    } else {
        spdlog::warn("No index buffer created for mesh {}", name);
    }
}

MeshBuffer::MeshBuffer(const Device &device, const std::string &name, const VkDeviceSize size_of_vertex_structure,
                       const std::size_t number_of_vertices, const VkDeviceSize size_of_index_structure,
                       const std::size_t number_of_indices)

    : m_vertex_buffer(device, name, size_of_vertex_structure * number_of_vertices,
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY),
      m_index_buffer(std::make_optional<GPUMemoryBuffer>(
          device, name, size_of_index_structure * number_of_indices,
          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY)),
      m_device(device) {}

MeshBuffer::MeshBuffer(const Device &device, const std::string &name, const VkDeviceSize size_of_vertex_structure,
                       const std::size_t number_of_vertices, void *vertices)
    // It's no problem to create the vertex buffer and index buffer before the corresponding staging buffers are
    // created!.
    : m_vertex_buffer(device, name, size_of_vertex_structure * number_of_vertices,
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY),
      m_index_buffer(std::nullopt), m_number_of_vertices(static_cast<std::uint32_t>(number_of_vertices)),
      m_number_of_indices(static_cast<std::uint32_t>(m_number_of_indices)), m_device(device) {
    assert(device.device());
    assert(device.allocator());
    assert(!name.empty());
    assert(size_of_vertex_structure > 0);

    std::size_t size_of_vertex_buffer = size_of_vertex_structure * number_of_vertices;

    spdlog::debug("Creating vertex buffer of size {} for mesh {}.", size_of_vertex_buffer, name);

    // Not using an index buffer can decrease performance drastically!
    spdlog::warn("Creating a vertex buffer without an index buffer!");
    spdlog::warn("Always use an index buffer if possible. The performance will decrease drastically otherwise!");

    VkDeviceSize vertices_memory_size = size_of_vertex_structure * number_of_vertices;

    StagingBuffer staging_buffer_for_vertices(m_device, name, size_of_vertex_buffer, vertices, vertices_memory_size);

    staging_buffer_for_vertices.upload_data_to_gpu(m_vertex_buffer);
}

MeshBuffer::MeshBuffer(const Device &device, const std::string &name, const VkDeviceSize size_of_vertex_structure,
                       const std::size_t number_of_vertices)

    : m_vertex_buffer(device, name, size_of_vertex_structure * number_of_vertices,
                      VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY),
      m_index_buffer(std::nullopt), m_number_of_indices(static_cast<std::uint32_t>(m_number_of_indices)),
      m_device(device) {}

} // namespace inexor::vulkan_renderer::wrapper
