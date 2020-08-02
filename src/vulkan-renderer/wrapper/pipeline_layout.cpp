#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

PipelineLayout::PipelineLayout(PipelineLayout &&other) noexcept
    : m_device(other.m_device), m_pipeline_layout(std::exchange(other.m_pipeline_layout, nullptr)),
      m_name(std::move(other.m_name)) {}

PipelineLayout::PipelineLayout(const VkDevice device, const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts,
                               const std::string &name)
    : m_device(device), m_name(name) {
    assert(device);
    assert(!descriptor_set_layouts.empty());
    assert(!name.empty());

    auto pipeline_layout_ci = make_info<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = static_cast<std::uint32_t>(descriptor_set_layouts.size());
    pipeline_layout_ci.pSetLayouts = descriptor_set_layouts.data();

    spdlog::debug("Creating pipeline layout {}.", name);

    if (vkCreatePipelineLayout(device, &pipeline_layout_ci, nullptr, &m_pipeline_layout)) {
        throw std::runtime_error("Error: vkCreatePipelineLayout failed for " + name + " !");
    }

    // TODO: Assign an internal name to this pipeline layout using Vulkan debug markers.

    spdlog::debug("Created pipeline layout successfully.");
}

PipelineLayout::~PipelineLayout() {
    spdlog::trace("Destroying pipeline layout {}.", m_name);
    vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
