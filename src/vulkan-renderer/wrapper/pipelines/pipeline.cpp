#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

GraphicsPipeline::GraphicsPipeline(const Device &device,
                                   std::vector<VkDescriptorSetLayout> descriptor_set_layouts,
                                   std::vector<VkPushConstantRange> push_constant_ranges,
                                   VkGraphicsPipelineCreateInfo pipeline_ci,
                                   std::string name)
    : m_device(device), m_name(std::move(name)) {

    // Create the graphics pipeline layout
    m_pipeline_layout = std::make_unique<PipelineLayout>(m_device, m_name, std::move(descriptor_set_layouts),
                                                         std::move(push_constant_ranges));

    // Set the pipeline layout
    pipeline_ci.layout = m_pipeline_layout->m_pipeline_layout;

    // Then create the graphics pipeline
    if (const auto result =
            vkCreateGraphicsPipelines(m_device.device(), nullptr, 1, &pipeline_ci, nullptr, &m_pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateGraphicsPipelines failed for pipeline " + m_name + " !", result);
    }
    m_device.set_debug_name(m_pipeline, m_name);
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline &&other) noexcept : m_device(other.m_device) {
    m_pipeline = std::exchange(other.m_pipeline, VK_NULL_HANDLE);
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, nullptr);
    m_name = std::move(other.m_name);
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
