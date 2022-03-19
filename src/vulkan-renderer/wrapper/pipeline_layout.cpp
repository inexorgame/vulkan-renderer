#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

PipelineLayout::PipelineLayout(const Device &device, const VkPipelineLayoutCreateInfo &pipeline_layout_ci,
                               std::string name)
    : m_device(device), m_name(std::move(name)) {

    if (const auto result = vkCreatePipelineLayout(device.device(), &pipeline_layout_ci, nullptr, &m_pipeline_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreatePipelineLayout failed for pipeline layout " + m_name + "!", result);
    }

    // TODO: Assign internal name!
}

PipelineLayout::PipelineLayout(PipelineLayout &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, VK_NULL_HANDLE);
}

PipelineLayout::~PipelineLayout() {
    vkDestroyPipelineLayout(m_device.device(), m_pipeline_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper