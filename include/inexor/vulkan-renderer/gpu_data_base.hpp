#pragma once

#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace inexor::vulkan_renderer {

template <typename VertexType, typename IndexType = std::uint32_t>
class GpuDataBase {
protected:
    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    BufferResource *m_index_buffer{nullptr};
    BufferResource *m_vertex_buffer{nullptr};

    std::uint32_t m_vertex_count{0};
    std::uint32_t m_index_count{0};

    virtual void setup_rendering_resources(RenderGraph *render_graph) = 0;

    GpuDataBase(const std::uint32_t vertex_count, const std::uint32_t index_count)
        : m_vertex_count(vertex_count), m_index_count(index_count) {}

public:
    GpuDataBase(GpuDataBase &&other) noexcept {
        m_descriptor_pool = std::exchange(other.m_descriptor_pool, nullptr);
        m_descriptor = std::exchange(other.m_descriptor, nullptr);
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

    [[nodiscard]] auto vertex_count() const {
        return m_vertex_count;
    }

    [[nodiscard]] auto index_count() const {
        return m_index_count;
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor->descriptor_set();
    }

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor->descriptor_set_layout();
    }
};

} // namespace inexor::vulkan_renderer
