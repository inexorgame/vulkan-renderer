#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

PipelineLayout::PipelineLayout(const Device &device, const VkPipelineLayoutCreateInfo &pipeline_layout_ci,
                               std::string name)
    : m_device(device), m_name(std::move(name)) {
    m_device.create_pipeline_layout(pipeline_layout_ci, &m_pipeline_layout, m_name);
}

PipelineLayout::PipelineLayout(const Device &device, const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts,
                               const std::vector<VkPushConstantRange> &push_constant_ranges, std::string name)
    : PipelineLayout(device,
                     wrapper::make_info<VkPipelineLayoutCreateInfo>({
                         .setLayoutCount = static_cast<std::uint32_t>(descriptor_set_layouts.size()),
                         .pSetLayouts = descriptor_set_layouts.data(),
                         .pushConstantRangeCount = static_cast<std::uint32_t>(push_constant_ranges.size()),
                         .pPushConstantRanges = push_constant_ranges.data(),
                     }),
                     std::move(name)) {}

PipelineLayout::PipelineLayout(PipelineLayout &&other) noexcept : m_device(other.m_device) {
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

PipelineLayout::~PipelineLayout() {
    vkDestroyPipelineLayout(m_device.device(), m_pipeline_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
