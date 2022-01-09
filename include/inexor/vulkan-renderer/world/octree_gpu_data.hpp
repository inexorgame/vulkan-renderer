#pragma once

#include "inexor/vulkan-renderer/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/standard_ubo.hpp"
#include "inexor/vulkan-renderer/world/octree_cpu_data.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::world {

// TODO: VertexType is invalid as long as the vertex layout is not set correctly!!
template <typename UniformBufferType, typename VertexType, typename IndexType = std::uint32_t>
class OctreeGpuData : public GpuDataBase<VertexType, IndexType> {
private:
    std::unique_ptr<wrapper::UniformBuffer<UniformBufferType>> m_uniform_buffer;

    void setup_rendering_resources(RenderGraph *render_graph, const OctreeCpuData<VertexType, IndexType> &cpu_data) {
        assert(render_graph);
        assert(!cpu_data.vertices().empty());
        assert(!cpu_data.indices().empty());

        // TODO: Unify into ->add_vertex_buffer(), ->add_index_buffer()...
        this->m_vertex_buffer = render_graph->add<BufferResource>("octree vertices", BufferUsage::VERTEX_BUFFER)
                                    ->set_vertex_attribute_layout<VertexType>(VertexType::vertex_attribute_layout())
                                    ->upload_data(cpu_data.vertices());

        this->m_index_buffer = render_graph->add<BufferResource>("octree indices", BufferUsage::INDEX_BUFFER)
                                   ->upload_data(cpu_data.indices());

        m_uniform_buffer =
            std::make_unique<wrapper::UniformBuffer<UniformBufferType>>(render_graph->device_wrapper(), "octree");

        const std::vector<VkDescriptorPoolSize> pool_sizes = {{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1}};

        VkDescriptorPoolCreateInfo desc_pool_ci{};
        desc_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        desc_pool_ci.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
        desc_pool_ci.pPoolSizes = pool_sizes.data();
        desc_pool_ci.maxSets = 1;

        if (const auto result =
                vkCreateDescriptorPool(render_graph->device(), &desc_pool_ci, nullptr, &m_descriptor_pool);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkCreateDescriptorPool failed!", result);
        }

        const std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings = {
            {0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
        };

        VkDescriptorSetLayoutCreateInfo desc_set_layout_ci{};
        desc_set_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        desc_set_layout_ci.pBindings = set_layout_bindings.data();
        desc_set_layout_ci.bindingCount = static_cast<uint32_t>(set_layout_bindings.size());

        if (const auto result = vkCreateDescriptorSetLayout(render_graph->device(), &desc_set_layout_ci, nullptr,
                                                            &m_descriptor_set_layout);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkCreateDescriptorSetLayout failed!", result);
        }

        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = m_descriptor_pool;
        descriptorSetAllocInfo.pSetLayouts = &m_descriptor_set_layout;
        descriptorSetAllocInfo.descriptorSetCount = 1;

        if (const auto result =
                vkAllocateDescriptorSets(render_graph->device(), &descriptorSetAllocInfo, &m_descriptor_set);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vkAllocateDescriptorSets failed!", result);
        }

        VkWriteDescriptorSet write_desc_set{};
        write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write_desc_set.descriptorCount = 1;
        write_desc_set.dstSet = m_descriptor_set;
        write_desc_set.dstBinding = 0;
        write_desc_set.pBufferInfo = &m_uniform_buffer->m_desc_buffer_info;

        vkUpdateDescriptorSets(render_graph->device(), 1, &write_desc_set, 0, 0);
    }

public:
    VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};
    VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};
    VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};

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

    void update_uniform_buffer(const UniformBufferType *data) {
        assert(data);
        m_uniform_buffer->update(data);
    }
};

} // namespace inexor::vulkan_renderer::world
