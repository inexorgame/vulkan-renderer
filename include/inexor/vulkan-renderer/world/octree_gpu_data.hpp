#pragma once

#include "inexor/vulkan-renderer/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/world/octree_cpu_data.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::world {

template <typename UniformBufferType, typename VertexType, typename IndexType = std::uint32_t>
class OctreeGpuData : public GpuDataBase<VertexType, IndexType> {
private:
    std::unique_ptr<wrapper::UniformBuffer<UniformBufferType>> m_uniform_buffer;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    void setup_rendering_resources(RenderGraph *render_graph, const OctreeCpuData<VertexType, IndexType> &cpu_data) {
        assert(render_graph);

        this->create_vertex_buffer(render_graph);
        this->create_index_buffer(render_graph);

        m_uniform_buffer =
            std::make_unique<wrapper::UniformBuffer<UniformBufferType>>(render_graph->device_wrapper(), "octree");

        auto builder = wrapper::DescriptorBuilder(render_graph->device_wrapper());

        m_descriptor = builder.add_uniform_buffer(*m_uniform_buffer).build("octree");
    }

public:
    OctreeGpuData(RenderGraph *render_graph, const OctreeCpuData<VertexType, IndexType> &cpu_data, std::string name)
        : GpuDataBase<VertexType, IndexType>(static_cast<std::uint32_t>(cpu_data.vertices().size()),
                                             static_cast<std::uint32_t>(cpu_data.indices().size()), name) {
        setup_rendering_resources(render_graph, cpu_data);
    }

    OctreeGpuData(const OctreeGpuData &) = delete;

    OctreeGpuData(OctreeGpuData &&other) noexcept : GpuDataBase<VertexType, IndexType>(std::move(other)) {
        m_uniform_buffer = std::exchange(other.m_uniform_buffer, nullptr);
        m_descriptor = std::exchange(other.m_descriptor, nullptr);
    }

    OctreeGpuData &operator=(const OctreeGpuData &) = delete;
    OctreeGpuData &operator=(OctreeGpuData &&) noexcept = default;

    void update_uniform_buffer(const UniformBufferType *data) {
        m_uniform_buffer->update(data);
    }

    [[nodiscard]] VkDescriptorSetLayout descriptor_set_layout() const {
        return m_descriptor->descriptor_set_layout;
    }

    [[nodiscard]] VkDescriptorSet descriptor_set() const {
        return m_descriptor->descriptor_set;
    }
};

} // namespace inexor::vulkan_renderer::world
