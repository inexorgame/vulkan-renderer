#pragma once

#include "inexor/vulkan-renderer/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/world/octree_cpu_data.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::world {

// TODO: VertexType is invalid as long as the vertex layout is not set correctly!!
template <typename UniformBufferObjectType, typename VertexType, typename IndexType = std::uint32_t>
class OctreeGpuData : public GpuDataBase<VertexType, IndexType> {
private:
    std::unique_ptr<wrapper::UniformBuffer<UniformBufferObjectType>> m_uniform_buffer;

    const OctreeCpuData<VertexType, IndexType> &m_cpu_data;

    void setup_rendering_resources(RenderGraph *render_graph) override {
        this->m_vertex_buffer = render_graph->add<BufferResource>("octree vertices", BufferUsage::VERTEX_BUFFER);

        // TODO: This vertex attribute layout must be somehow determined by the template parameter?
        this->m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexType, position))
            ->add_vertex_attribute(VK_FORMAT_R32G32B32_SFLOAT, offsetof(VertexType, color))
            ->template set_element_size<VertexType>()
            ->upload_data(m_cpu_data.vertices());

        this->m_index_buffer = render_graph->add<BufferResource>("octree indices", BufferUsage::INDEX_BUFFER)
                             ->upload_data(m_cpu_data.indices());

        const std::vector<VkDescriptorPoolSize> pool_sizes{{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};

        this->m_descriptor_pool =
            std::make_unique<wrapper::DescriptorPool>(render_graph->device_wrapper(), pool_sizes, "octree");

        // Create an instance of the resource descriptor builder.
        // This allows us to make resource descriptors with the help of a builder pattern.
        wrapper::DescriptorBuilder descriptor_builder(render_graph->device_wrapper(),
                                                      this->m_descriptor_pool->descriptor_pool());

        m_uniform_buffer =
            std::make_unique<wrapper::UniformBuffer<UniformBufferObjectType>>(render_graph->device_wrapper(), "octree");

        this->m_descriptor =
            descriptor_builder.add_uniform_buffer<UniformBufferObjectType>(m_uniform_buffer->buffer()).build("octree");
    }

public:
    OctreeGpuData(RenderGraph *render_graph, const OctreeCpuData<VertexType, IndexType> &cpu_data)
        : GpuDataBase<VertexType, IndexType>(static_cast<std::uint32_t>(cpu_data.vertices().size()),
                                             static_cast<std::uint32_t>(cpu_data.indices().size())),
          m_cpu_data(cpu_data) {

        setup_rendering_resources(render_graph);
    }

    OctreeGpuData() = delete;
    OctreeGpuData(const OctreeGpuData &) = delete;

    OctreeGpuData(OctreeGpuData &&other) noexcept
        : m_cpu_data(other.m_cpu_data), GpuDataBase<VertexType, IndexType>(std::move(other)) {
        m_uniform_buffer = std::exchange(other.m_uniform_buffer, nullptr);
    }

    void update_uniform_buffer(const UniformBufferObjectType *data) {
        assert(data);
        m_uniform_buffer->update(data);
    }
};

} // namespace inexor::vulkan_renderer::world
