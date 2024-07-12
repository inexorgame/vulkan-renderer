#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

PipelineLayout::PipelineLayout(const Device &device,
                               const std::span<const VkDescriptorSetLayout> desc_set_layouts,
                               const std::span<const VkPushConstantRange> push_constant_ranges,
                               std::string name)
    : m_device(device), m_name(std::move(name)) {
    if (m_name.empty()) {
        throw std::invalid_argument("[PipelineLayout::PipelineLayout] Error: Parameter 'name' is emtpy!");
    }


}

PipelineLayout::PipelineLayout(PipelineLayout &&other) noexcept : m_device(other.m_device) {
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

PipelineLayout::~PipelineLayout() {
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
