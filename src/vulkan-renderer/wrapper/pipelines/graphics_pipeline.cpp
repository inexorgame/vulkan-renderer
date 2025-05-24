#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

GraphicsPipeline::GraphicsPipeline(const Device &device,
                                   const std::span<const VkDescriptorSetLayout> descriptor_set_layouts,
                                   const std::span<const VkPushConstantRange> push_constant_ranges,
                                   VkGraphicsPipelineCreateInfo pipeline_ci,
                                   std::string name)
    : m_device(device), m_name(std::move(name)), m_pipeline_ci(std::move(pipeline_ci)) {

    // We can already create the pipeline layout, but the actual graphics pipelines will all be created in rendergraph
    // in one batched call to vkCreateGraphicsPipelines.
    m_pipeline_layout =
        std::make_unique<PipelineLayout>(m_device, m_name, descriptor_set_layouts, push_constant_ranges);

    // Store the pipeline layout
    m_pipeline_ci.layout = m_pipeline_layout->m_pipeline_layout;
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
