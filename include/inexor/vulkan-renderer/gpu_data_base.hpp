#pragma once

#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace inexor::vulkan_renderer {

/// A template base class for gpu data of rendering
/// This class was created to encapsulate everything that is required to set up the rendering of data using the
/// rendergraph.
///
template <typename VertexType, typename IndexType = std::uint32_t>
class GpuDataBase {
protected:
    /// The rendergraph buffer resource of the vertices
    BufferResource *m_vertex_buffer{nullptr};

    /// The rendergraph buffer resource of the indices
    BufferResource *m_index_buffer{nullptr};

    /// The number of vertices in the vertex buffer
    std::uint32_t m_vertex_count{0};

    /// The number of indices in the index buffer
    std::uint32_t m_index_count{0};

    std::vector<VertexType> m_vertices;
    std::vector<IndexType> m_indices;

    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    /// Default constructor
    ///
    ///
    GpuDataBase(const std::uint32_t vertex_count, const std::uint32_t index_count)
        : m_vertex_count(vertex_count), m_index_count(index_count) {}

    /// Overloaded constructor
    /// @note Sometimes we don't know the number of vertices and indices at the beginning,
    /// which is why this overloaded constructor exists.
    GpuDataBase() : GpuDataBase(0, 0) {}

public:
    GpuDataBase(GpuDataBase &&other) noexcept {
        m_descriptor = std::move(other.m_descriptor);
        m_index_buffer = std::exchange(other.m_index_buffer, nullptr);
        m_vertex_buffer = std::exchange(other.m_vertex_buffer, nullptr);
        m_vertex_count = other.m_vertex_count;
        m_index_count = other.m_index_count;
    }

    void update_indices(const std::vector<IndexType> &indices) {
        m_index_buffer->upload_data<IndexType>(indices);
        m_index_count = static_cast<std::uint32_t>(indices.size());
    }

    void update_vertices(const std::vector<VertexType> &vertices) {
        m_vertex_buffer->upload_data<VertexType>(vertices);
        m_vertex_count = static_cast<std::uint32_t>(vertices.size());
    }

    [[nodiscard]] const auto *vertex_buffer() const {
        return m_vertex_buffer;
    }

    [[nodiscard]] const auto *index_buffer() const {
        return m_index_buffer;
    }

    [[nodiscard]] std::uint32_t vertex_count() const {
        return m_vertex_count;
    }

    [[nodiscard]] std::uint32_t index_count() const {
        return m_index_count;
    }

    [[nodiscard]] const auto &vertices() const {
        return m_vertices;
    }

    [[nodiscard]] const auto &indices() const {
        return m_indices;
    }

    [[nodiscard]] const auto descriptor_set() const {
        return m_descriptor->descriptor_set();
    }

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor->descriptor_set_layout();
    }
};

} // namespace inexor::vulkan_renderer
