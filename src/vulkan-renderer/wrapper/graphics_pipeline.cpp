#include "inexor/vulkan-renderer/wrapper/graphics_pipeline.hpp"

#include "inexor/vulkan-renderer/exception.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

GraphicsPipeline::GraphicsPipeline(const Device &device, const VkGraphicsPipelineCreateInfo &pipeline_ci,
                                   std::string name)
    : m_device(device), m_name(std::move(name)) {
    assert(!m_name.empty());

    if (const auto result =
            vkCreateGraphicsPipelines(m_device.device(), nullptr, 1, &pipeline_ci, nullptr, &m_pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateGraphicsPipeline failed for graphics pipeline " + m_name + "!", result);
    }

    // TODO: Assign internal debug name!
}

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_pipeline = std::exchange(other.m_pipeline, VK_NULL_HANDLE);
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper