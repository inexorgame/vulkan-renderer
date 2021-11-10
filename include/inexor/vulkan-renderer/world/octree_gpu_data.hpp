#pragma once

#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/world/octree_cpu_data.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::world {

// TODO: Maybe implement a RenderingData template base class?
// The descriptor poll for example could be in the base class
// TODO: VertexType is invalid as long as the vertex layout is not set correctly!!
template <typename VertexType, typename IndexType, typename UniformBufferObjectType>
class OctreeGPUData {
private:
    std::unique_ptr<wrapper::DescriptorPool> m_descriptor_pool;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;
    std::unique_ptr<wrapper::UniformBuffer<UniformBufferObjectType>> m_uniform_buffer;

    BufferResource *m_index_buffer{nullptr};
    BufferResource *m_vertex_buffer{nullptr};

    std::uint32_t m_vertex_count;
    std::uint32_t m_index_count;

    void setup_rendering_resources(RenderGraph *render_graph, const std::vector<VertexType> &vertices,
                                   const std::vector<IndexType> &indices) {

        m_vertex_buffer = render_graph->add<BufferResource>("octree vertices", BufferUsage::VERTEX_BUFFER);

        // TODO: Strictly speaking this vertex attribute layout must be somehow determined by the template parameter?
        m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexType, position))
            ->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexType, color))
            ->set_element_size<VertexType>()
            ->upload_data(vertices);

        m_index_buffer =
            render_graph->add<BufferResource>("octree indices", BufferUsage::INDEX_BUFFER)->upload_data(indices);

        const std::vector<VkDescriptorPoolSize> pool_sizes{{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};

        m_descriptor_pool =
            std::make_unique<wrapper::DescriptorPool>(render_graph->device_wrapper(), pool_sizes, "octree");

        // Create an instance of the resource descriptor builder.
        // This allows us to make resource descriptors with the help of a builder pattern.
        wrapper::DescriptorBuilder descriptor_builder(render_graph->device_wrapper(),
                                                      m_descriptor_pool->descriptor_pool());

        m_uniform_buffer =
            std::make_unique<wrapper::UniformBuffer<UniformBufferObjectType>>(render_graph->device_wrapper(), "octree");

        m_descriptor =
            descriptor_builder.add_uniform_buffer<UniformBufferObjectType>(m_uniform_buffer->buffer()).build("octree");
    }

public:
    OctreeGPUData(RenderGraph *render_graph, const OctreeCPUData<VertexType, IndexType> &cpu_data) {
        setup_rendering_resources(render_graph, cpu_data.vertices(), cpu_data.indices());
        m_vertex_count = static_cast<std::uint32_t>(cpu_data.vertices().size());
        m_index_count = static_cast<std::uint32_t>(cpu_data.indices().size());
    }

    OctreeGPUData() = delete;
    OctreeGPUData(const OctreeGPUData &) = delete;

    OctreeGPUData(OctreeGPUData &&other) noexcept {
        m_descriptor_pool = std::move(other.m_descriptor_pool);
        m_descriptor = std::move(other.m_descriptor);
        m_uniform_buffer = std::move(other.m_uniform_buffer);
        m_vertex_buffer = std::exchange(other.m_vertex_buffer, nullptr);
        m_index_buffer = std::exchange(other.m_index_buffer, nullptr);
        m_vertex_count = other.m_vertex_count;
        m_index_count = other.m_index_count;
    }

    ~OctreeGPUData() = default;

    OctreeGPUData &operator=(const OctreeGPUData &) = delete;
    OctreeGPUData &operator=(OctreeGPUData &&) = default;

    void update_octree(const OctreeCPUData<VertexType, IndexType> &cpu_data) {
        m_vertex_buffer->upload_data<VertexType>(cpu_data.vertices());
        m_index_buffer->upload_data<IndexType>(cpu_data.indices());
        m_vertex_count = static_cast<std::uint32_t>(cpu_data.vertex_count());
        m_index_count = static_cast<std::uint32_t>(cpu_data.index_count());
    }

    void update_ubo(const UniformBufferObjectType *data) {
        m_uniform_buffer->update(data);
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

} // namespace inexor::vulkan_renderer::world
