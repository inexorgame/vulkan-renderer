#pragma once

#include "inexor/vulkan-renderer/vk_tools/vert_attr_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer {

// TODO: Use C++20 concepts to allow only for supported index data types!

/// This wrapper class for gpu data stores a vector of vertices and indices, and the vertex and index buffer.
/// @tparam VertexType The data type of the vertices
/// @tparam IndexType The data type of the indices
template <typename VertexType, typename IndexType = std::uint32_t>
class GpuDataBase {
private:
    std::string m_name;

    const std::vector<vk_tools::VertexAttributeLayout> m_vertex_attribute_layout;

    VkBuffer m_vertex_buffer{VK_NULL_HANDLE};
    VkBuffer m_index_buffer{VK_NULL_HANDLE};

    VmaAllocationCreateInfo m_vertex_buffer_aci{};
    VmaAllocationCreateInfo m_index_buffer_aci{};

    VmaAllocationInfo m_vertex_buffer_ai{};
    VmaAllocationInfo m_index_buffer_ai{};

    VmaAllocation m_vertex_buffer_alloc{};
    VmaAllocation m_index_buffer_alloc{};

    void create_vertex_buffer(const std::vector<VertexType> &vertices) {
        m_vertex_buffer_aci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        m_vertex_buffer_aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        auto buffer_ci = wrapper::make_info<VkBufferCreateInfo>();
        buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_ci.size = sizeof(VertexType) * vertices.size();
        buffer_ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

        m_vertex_buffer =
            m_device.create_buffer(buffer_ci, m_vertex_buffer_aci, &m_vertex_buffer_alloc, &m_vertex_buffer_ai);
    }

    void destroy_vertex_buffer() {
        m_device.destroy_buffer(m_vertex_buffer, m_vertex_buffer_alloc);
    }

    void create_index_buffer(const std::vector<IndexType> &indices) {
        m_index_buffer_aci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
        m_index_buffer_aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

        auto buffer_ci = wrapper::make_info<VkBufferCreateInfo>();
        buffer_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        buffer_ci.size = sizeof(IndexType) * indices.size();
        buffer_ci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;

        m_index_buffer =
            m_device.create_buffer(buffer_ci, m_index_buffer_aci, &m_index_buffer_alloc, &m_index_buffer_ai);
    }

    void destroy_index_buffer() {
        m_device.destroy_buffer(m_index_buffer, m_index_buffer_alloc);
    }

protected:
    const wrapper::Device &m_device;

    std::vector<VertexType> m_vertices{};
    std::vector<IndexType> m_indices{};

    /// Set the vertex attribute layout, but do not provide vertices or indices yet
    GpuDataBase(const wrapper::Device &device,
                const std::vector<vk_tools::VertexAttributeLayout> &vertex_attribute_layout, std::string name)
        : m_device(device), m_name(std::move(name)), m_vertex_attribute_layout(vertex_attribute_layout) {}

    /// Set the vertex attribute layout and the vertices and indices
    GpuDataBase(const wrapper::Device &device,
                const std::vector<vk_tools::VertexAttributeLayout> &vertex_attribute_layout,
                const std::vector<VertexType> &vertices, const std::vector<IndexType> &indices, std::string name)
        : GpuDataBase(device, vertex_attribute_layout, name), m_vertices(vertices), m_indices(indices) {}

    GpuDataBase(GpuDataBase &&other) noexcept : m_device(other.m_device) {
        m_name = std::move(other.m_name);
        m_vertices = std::move(other.m_vertices);
        m_indices = std::move(other.m_indices);
        m_vertex_buffer = std::exchange(other.m_vertex_buffer, VK_NULL_HANDLE);
        m_index_buffer = std::exchange(other.m_index_buffer, VK_NULL_HANDLE);
    }

    void create_vertex_buffer() {
        create_vertex_buffer(m_vertices);
    }

    void create_index_buffer() {
        create_index_buffer(m_indices);
    }

public:
    // TODO: Change this template class so it always allocates a little more memory than needed.

    // TODO: Accept a std::span once we use C++20 here.
    void update_vertices(const std::vector<VertexType> &vertices) {
        m_vertices = vertices;

        // If the new vertices take up more memory than we have,
        // destroy the old vertex buffer and create a new one with matching size.
        if (vertices.size() > m_vertex_buffer_ai.size) {
            destroy_vertex_buffer();
        }

        // If not vertex buffer is allocated, create a new one with the size of m_vertices.
        // This nicely deals with both the case of
        //   1) the old vertex buffer being too small (destroy the old one and create a new one here)
        //   2) having no vertex buffer at all allocated in the beginning (create one).
        if (!m_vertex_buffer_ai.pMappedData) {
            create_vertex_buffer();
        }

        std::memcpy(m_vertex_buffer_ai.pMappedData, vertices.data(), vertices.size());
    }

    // TODO: Accept a std::span once we use C++20 here.
    void update_indices(const std::vector<IndexType> &indices) {
        m_indices = indices;

        // If the new indices take up more memory than we have,
        // destroy the old index buffer and create a new one with matching size.
        if (indices.size() > m_index_buffer_ai.size) {
            destroy_index_buffer();
        }

        // If not index buffer is allocated, create a new one with the size of m_indices.
        // This nicely deals with both the case of
        //   1) the old index buffer being too small (destroy the old one and create a new one here)
        //   2) having no index buffer at all allocated in the beginning (create one).
        if (!m_index_buffer_ai.pMappedData) {
            create_index_buffer();
        }

        std::memcpy(m_index_buffer_ai.pMappedData, indices.data(), indices.size());
    }

    [[nodiscard]] VkBuffer vertex_buffer() const {
        return m_vertex_buffer;
    }

    [[nodiscard]] VkBuffer index_buffer() const {
        return m_index_buffer;
    }

    [[nodiscard]] bool has_index_buffer() const {
        return m_index_buffer != nullptr;
    }

    [[nodiscard]] std::size_t vertex_count() const {
        return m_vertices.size();
    }

    [[nodiscard]] std::size_t index_count() const {
        return m_indices.size();
    }

    [[nodiscard]] const std::vector<VertexType> &vertices() const {
        return m_vertices;
    }

    [[nodiscard]] const std::vector<IndexType> &indices() const {
        return m_indices;
    }
};

} // namespace inexor::vulkan_renderer
