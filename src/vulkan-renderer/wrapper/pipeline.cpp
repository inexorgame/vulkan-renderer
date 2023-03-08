#include "inexor/vulkan-renderer/wrapper/pipeline.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

GraphicsPipeline::GraphicsPipeline(const Device &device, const VkGraphicsPipelineCreateInfo &pipeline_ci,
                                   std::string name)
    : m_device(device), m_name(std::move(name)) {
    m_device.create_graphics_pipeline(pipeline_ci, &m_pipeline, m_name);
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline &&other) noexcept : m_device(other.m_device) {
    m_pipeline = std::exchange(other.m_pipeline, nullptr);
    m_name = std::move(other.m_name);
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
