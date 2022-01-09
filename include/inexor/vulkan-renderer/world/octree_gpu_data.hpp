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

    void setup_rendering_resources(RenderGraph *render_graph, const OctreeCpuData<VertexType, IndexType> &cpu_data) {

        this->m_vertex_buffer = render_graph->add<BufferResource>("octree vertices", BufferUsage::VERTEX_BUFFER)
                                    ->set_vertex_attribute_layout<VertexType>(VertexType::vertex_attribute_layout())
                                    ->upload_data(cpu_data.vertices());

        this->m_index_buffer = render_graph->add<BufferResource>("octree indices", BufferUsage::INDEX_BUFFER)
                                   ->upload_data(cpu_data.indices());

        // Create an instance of the resource descriptor builder.
        // This allows us to make resource descriptors with the help of a builder pattern.
        wrapper::DescriptorBuilder descriptor_builder(render_graph->device_wrapper());

        m_uniform_buffer =
            std::make_unique<wrapper::UniformBuffer<UniformBufferObjectType>>(render_graph->device_wrapper(), "octree");

        this->m_descriptor =
            descriptor_builder.add_uniform_buffer<UniformBufferObjectType>(m_uniform_buffer->buffer()).build("octree");
    }

public:
    OctreeGpuData(RenderGraph *render_graph, const OctreeCpuData<VertexType, IndexType> &cpu_data)
        : GpuDataBase<VertexType, IndexType>(static_cast<std::uint32_t>(cpu_data.vertices().size()),
                                             static_cast<std::uint32_t>(cpu_data.indices().size())) {
        setup_rendering_resources(render_graph, cpu_data);
    }

    OctreeGpuData(const OctreeGpuData &) = delete;

    OctreeGpuData(OctreeGpuData &&other) noexcept : GpuDataBase<VertexType, IndexType>(std::move(other)) {
        m_uniform_buffer = std::exchange(other.m_uniform_buffer, nullptr);
    }

    OctreeGpuData &operator=(const OctreeGpuData &) = delete;
    OctreeGpuData &operator=(OctreeGpuData &&) noexcept = default;

    void update_uniform_buffer(const UniformBufferObjectType *data) {
        assert(data);
        m_uniform_buffer->update(data);
    }
};

} // namespace inexor::vulkan_renderer::world
