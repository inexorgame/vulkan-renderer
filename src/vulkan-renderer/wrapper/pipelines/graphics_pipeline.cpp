#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"

#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

GraphicsPipeline::GraphicsPipeline(const Device &device, const PipelineCache &pipeline_cache,
                                   std::span<const VkDescriptorSetLayout> descriptor_set_layouts,
                                   std::span<const VkPushConstantRange> push_constant_ranges,
                                   GraphicsPipelineSetupData pipeline_setup_data, std::string name)
    : m_device(device), m_pipeline_setup_data(std::move(pipeline_setup_data)), m_name(std::move(name)) {

    // @TODO Expose the pipeline layout as parameter
    m_pipeline_layout = std::make_unique<PipelineLayout>(m_device, m_name, std::move(descriptor_set_layouts),
                                                         std::move(push_constant_ranges));

    const auto pipeline_ci = make_info<VkGraphicsPipelineCreateInfo>({
        .layout = m_pipeline_layout->pipeline_layout(),
    });

    if (const auto result = vkCreateGraphicsPipelines(m_device.device(), pipeline_cache.m_pipeline_cache, 1,
                                                      &pipeline_ci, nullptr, &m_pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateGraphicsPipelines failed!", result, m_name);
    }
    m_device.set_debug_name(m_pipeline, m_name);
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline &&other) noexcept : m_device(other.m_device) {
    // TODO: Check me!
    m_pipeline = std::exchange(other.m_pipeline, VK_NULL_HANDLE);
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, nullptr);
    m_name = std::move(other.m_name);
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
}

VkPipelineLayout GraphicsPipeline::pipeline_layout() const {
    return m_pipeline_layout->pipeline_layout();
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
