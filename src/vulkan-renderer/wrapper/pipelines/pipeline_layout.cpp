#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

PipelineLayout::PipelineLayout(const Device &device, std::string name,
                               const std::span<const VkDescriptorSetLayout> descriptor_set_layouts,
                               const std::span<const VkPushConstantRange> push_constant_ranges)
    : m_device(device), m_name(std::move(name)) {
    if (m_name.empty()) {
        throw InexorException("Error: Parameter 'name' is an emtpy string!");
    }

    const auto pipeline_layout_ci = wrapper::make_info<VkPipelineLayoutCreateInfo>({
        .setLayoutCount = static_cast<std::uint32_t>(descriptor_set_layouts.size()),
        .pSetLayouts = descriptor_set_layouts.data(),
        .pushConstantRangeCount = static_cast<std::uint32_t>(push_constant_ranges.size()),
        .pPushConstantRanges = push_constant_ranges.data(),
    });

    if (const auto result = vkCreatePipelineLayout(m_device.device(), &pipeline_layout_ci, nullptr, &m_pipeline_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreatePipelineLayout failed!", result, m_name);
    }
    m_device.set_debug_name(m_pipeline_layout, m_name);
}

PipelineLayout::PipelineLayout(PipelineLayout &&other) noexcept : m_device(other.m_device) {
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

PipelineLayout::~PipelineLayout() {
    vkDestroyPipelineLayout(m_device.device(), m_pipeline_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
