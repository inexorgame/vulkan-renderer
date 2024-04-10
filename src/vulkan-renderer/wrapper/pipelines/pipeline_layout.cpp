#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

PipelineLayout::PipelineLayout(const Device &device,
                               const std::span<const VkDescriptorSetLayout> descriptor_set_layouts,
                               const std::span<const VkPushConstantRange> push_constant_ranges, std::string name)
    : m_device(device), m_name(std::move(name)) {

    const auto pipeline_layout_ci = wrapper::make_info<VkPipelineLayoutCreateInfo>({
        .setLayoutCount = static_cast<std::uint32_t>(descriptor_set_layouts.size()),
        .pSetLayouts = descriptor_set_layouts.data(),
        .pushConstantRangeCount = static_cast<std::uint32_t>(push_constant_ranges.size()),
        .pPushConstantRanges = push_constant_ranges.data(),
    });

    if (const auto result = vkCreatePipelineLayout(m_device.device(), &pipeline_layout_ci, nullptr, &m_pipeline_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreatePipelineLayout failed for pipeline layout " + m_name + "!", result);
    }
    // Assign an internal debug name to this pipeline layout using Vulkan debug utils (VK_EXT_debug_utils)
    m_device.set_debug_utils_object_name(VK_OBJECT_TYPE_PIPELINE_LAYOUT,
                                         reinterpret_cast<std::uint64_t>(m_pipeline_layout), m_name);
}

PipelineLayout::PipelineLayout(PipelineLayout &&other) noexcept : m_device(other.m_device) {
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

PipelineLayout::~PipelineLayout() {
    vkDestroyPipelineLayout(m_device.device(), m_pipeline_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
