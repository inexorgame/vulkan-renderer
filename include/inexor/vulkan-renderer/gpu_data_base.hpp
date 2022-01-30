#pragma once

#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer {

template <typename VertexType, typename IndexType = std::uint32_t>
class GpuDataBase {
private:
    std::string m_name;
    std::uint32_t m_vertex_count{0};
    std::uint32_t m_index_count{0};

    BufferResource *m_vertex_buffer{nullptr};
    BufferResource *m_index_buffer{nullptr};

protected:
    std::vector<VertexType> m_vertices;
    std::vector<IndexType> m_indices;

    // TODO: Add overloaded constructor which takes a vector of vertices/indices

    GpuDataBase(const std::uint32_t vertex_count, const std::uint32_t index_count, std::string name)
        : m_vertex_count(vertex_count), m_index_count(index_count), m_name(name) {}

    /// Overloaded constructor
    /// @note Sometimes we don't know the number of vertices and indices at the beginning,
    /// which is why this overloaded constructor exists.
    GpuDataBase(std::string name) : GpuDataBase(0, 0, name) {}

    void create_vertex_buffer(RenderGraph *render_graph,
                              const std::vector<vk_tools::VertexAttributeLayout> &vertex_attribute_layout) {
        m_vertex_buffer = render_graph->add<BufferResource>(m_name, BufferUsage::VERTEX_BUFFER)
                              ->set_vertex_attribute_layout<VertexType>(vertex_attribute_layout)
                              ->upload_data(m_vertices);
    }

    void create_vertex_buffer(RenderGraph *render_graph) {
        create_vertex_buffer(render_graph, VertexType::vertex_attribute_layout());
    }

    void create_index_buffer(RenderGraph *render_graph) {
        m_index_buffer = render_graph->add<BufferResource>(m_name, BufferUsage::INDEX_BUFFER)->upload_data(m_indices);
    }

public:
    GpuDataBase(GpuDataBase &&other) noexcept {
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
};

} // namespace inexor::vulkan_renderer
