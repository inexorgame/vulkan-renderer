#pragma once

#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/staging_buffer.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for mesh buffers.
/// In Inexor engine, a mesh buffer is a vertex buffer with a corresponding index buffer.
/// The following example shows how to create a mesh buffer with a custom vertex structure:
/// @code
/// struct ModelVertex {
///    glm::vec3 position{0.0f, 0.0f, 0.0f};
///    glm::vec3 color{0.0f, 0.0f, 0.0f};
///    glm::vec3 normal{0.0f, 0.0f, 0.0f};
///    glm::vec2 uv{0.0f, 0.0f};
/// };
///
/// MeshBuffer<ModelVertex, std::uint32_t> myMeshBufferA(1024); // Create a mesh buffer of 1024 vertices
/// MeshBuffer<ModelVertex, std::uint32_t> myMeshBufferB(1024, 128); // Create a mesh buffer of 1024 vertices and 128
/// indices.
///
/// @tparam VertexType The vertex type.
/// @tparam IndexType The index type, usually uint32_t.
/// @note IndexType can also be uint16_t in some cases, though that allows a lower number of indices.
template <typename VertexType, typename IndexType = std::uint32_t>
class MeshBuffer {
    const Device &m_device;
    std::string m_name;

    GPUMemoryBuffer m_vertex_buffer;

    std::optional<GPUMemoryBuffer> m_index_buffer{std::nullopt};

    std::uint32_t m_number_of_vertices{0};
    std::uint32_t m_number_of_indices{0};

public:
    /// @brief Creates a mesh buffer of type VertexType with a corresponding index buffer of type IndexType by
    /// specifying the number of vertices and indices but without already specifying any vertex data or index data.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal name of the mesh buffer, must not be empty.
    /// @param vertex_count The number of vertices, must be greater than 0.
    /// @param index_count The number of indices, must be greater than 0.
    MeshBuffer(const Device &device, const std::string &name, const std::size_t vertex_count,
               const std::size_t index_count)
        // It's no problem to create the vertex buffer and index buffer before the corresponding staging buffers are
        // created!.
        : m_vertex_buffer(device, name, sizeof(VertexType) * vertex_count,
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_ONLY),
          m_index_buffer(std::make_optional<GPUMemoryBuffer>(
              device, name, sizeof(IndexType) * index_count,
              VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_ONLY)),
          m_device(device) {
        assert(device.device());
        assert(device.allocator());
        assert(!name.empty());
        assert(vertex_count > 0);
        assert(index_count > 0);

        std::size_t vertex_buffer_size = sizeof(VertexType) * vertex_count;
        std::size_t index_buffer_size = sizeof(IndexType) * index_count;

        spdlog::debug("Creating vertex buffer of size {} for mesh {}.", vertex_buffer_size, name);
        spdlog::debug("Creating index buffer of size {} for mesh {}.", index_buffer_size, name);
    }

    /// @brief Creates a mesh buffer of type VertexType without a corresponding index buffer by specifying the number of
    /// vertices but without already specifying any vertex data.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal name of the mesh buffer, must not be empty.
    /// @param vertex_count The number of vertices, must be greater than 0.
    MeshBuffer(const Device &device, const std::string &name, const std::size_t vertex_count)
        // It's no problem to create the vertex buffer and index buffer before the corresponding staging buffers are
        // created!.
        : m_vertex_buffer(device, name, sizeof(VertexType) * vertex_count,
                          VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VMA_MEMORY_USAGE_CPU_ONLY),
          m_index_buffer(std::nullopt), m_number_of_vertices(vertex_count), m_number_of_indices(0), m_device(device) {

        assert(device.device());
        assert(device.allocator());
        assert(!name.empty());

        std::size_t vertex_buffer_size = sizeof(VertexType) * vertex_count;

        spdlog::debug("Creating vertex buffer of size {} for mesh {}.", vertex_buffer_size, name);
    }

    /// @brief Constructs a mesh buffer of type VertexType with a corresponding index buffer of type IndexType.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal name of the mesh buffer.
    /// @param vertices A const std::vector of vertices of type VertexType.
    /// @param indices A const std::vector of indices of type IndexType.
    MeshBuffer(const Device &device, const std::string &name, std::vector<VertexType> &vertices,
               std::vector<IndexType> &indices)
        : MeshBuffer(device, name, vertices.size(), indices.size()) {
        std::size_t vertex_buffer_size = sizeof(VertexType) * vertices.size();
        std::size_t index_buffer_size = sizeof(IndexType) * indices.size();

        spdlog::debug("Creating vertex buffer of size {} for mesh {}.", vertex_buffer_size, name);
        spdlog::debug("Creating index buffer of size {} for mesh {}.", index_buffer_size, name);

        // Not using an index buffer can decrease performance drastically!
        if (index_buffer_size == 0) {
            spdlog::warn("Size of index buffer is 0! Always use an index buffer if possible!");
            spdlog::warn("Not using an index buffer decreases performance drastically!");
        }

        StagingBuffer staging_buffer_for_vertices(m_device, name, vertex_buffer_size, vertices.data(),
                                                  vertex_buffer_size);

        staging_buffer_for_vertices.upload_data_to_gpu(m_vertex_buffer);

        if (!indices.empty()) {
            VkDeviceSize indices_memory_size = sizeof(IndexType) * indices.size();

            StagingBuffer staging_buffer_for_indices(m_device, name, index_buffer_size,
                                                     static_cast<void *>(indices.data()), indices_memory_size);

            staging_buffer_for_indices.upload_data_to_gpu(*m_index_buffer);
        } else {
            spdlog::warn("No index buffer created for mesh {}!", name);
        }

        update_vertices(vertices);
        update_indices(indices);
    }

    /// @brief Constructs a mesh buffer of type VertexType without an index buffer.
    /// @warning Not using an index buffer will decrease performance drastically!
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal name of the mesh buffer.
    /// @param vertices A const std::vector of vertices of type VertexType.
    MeshBuffer(const Device &device, const std::string &name, const std::vector<VertexType> &vertices)
        : MeshBuffer(device, name, vertices.size()) {
        std::size_t size_of_vertex_buffer = sizeof(VertexType) * vertices.size();

        spdlog::debug("Creating vertex buffer of size {} for mesh {}.", size_of_vertex_buffer, name);

        // Not using an index buffer can decrease performance drastically!
        spdlog::warn("Creating a vertex buffer without an index buffer!");
        spdlog::warn("Always use an index buffer if possible. The performance will decrease drastically otherwise!");

        VkDeviceSize vertices_memory_size = sizeof(VertexType) * vertices.size();

        StagingBuffer staging_buffer_for_vertices(m_device, name, size_of_vertex_buffer, vertices,
                                                  vertices_memory_size);

        staging_buffer_for_vertices.upload_data_to_gpu(m_vertex_buffer);

        update_vertices(vertices);
    }

    MeshBuffer(const MeshBuffer &) = delete;

    MeshBuffer(MeshBuffer &&other) noexcept
        : m_name(std::move(other.m_name)), m_vertex_buffer(std::move(other.m_vertex_buffer)),
          m_index_buffer(std::exchange(other.m_index_buffer, std::nullopt)),
          m_number_of_vertices(other.m_number_of_vertices), m_number_of_indices(other.m_number_of_indices),
          m_device(other.m_device) {}

    ~MeshBuffer() = default;

    MeshBuffer &operator=(const MeshBuffer &) = delete;
    MeshBuffer &operator=(MeshBuffer &&) = delete;

    [[nodiscard]] VkBuffer get_vertex_buffer() const {
        return m_vertex_buffer.buffer();
    }

    [[nodiscard]] bool has_index_buffer() const {
        return m_index_buffer.has_value();
    }

    [[nodiscard]] VkBuffer get_index_buffer() const {
        return m_index_buffer.value().buffer();
    }

    [[nodiscard]] std::uint32_t get_vertex_count() const {
        return m_number_of_vertices;
    }

    [[nodiscard]] std::uint32_t get_index_count() const {
        return m_number_of_indices;
    }

    [[nodiscard]] auto get_vertex_buffer_address() const {
        return m_vertex_buffer.allocation_info().pMappedData;
    }

    [[nodiscard]] auto get_index_buffer_address() const {
        if (!m_index_buffer) {
            throw std::runtime_error(std::string("Error: No index buffer for mesh " + m_name + "!"));
        }

        return m_index_buffer.value().allocation_info().pMappedData;
    }

    void update_vertices(const std::vector<VertexType> &vertices) {
        std::memcpy(m_vertex_buffer.allocation_info().pMappedData, vertices.data(),
                    sizeof(VertexType) * vertices.size());
    }

    void update_indices(const std::vector<IndexType> &indices) {
        if (!m_index_buffer) {
            throw std::runtime_error(std::string("Error: No index buffer for mesh " + m_name + "!"));
        }

        std::memcpy(m_index_buffer.value().allocation_info().pMappedData, indices.data(),
                    sizeof(IndexType) * indices.size());
    }
};

} // namespace inexor::vulkan_renderer::wrapper
