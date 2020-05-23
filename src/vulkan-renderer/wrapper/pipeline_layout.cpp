#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

PipelineLayout::PipelineLayout(PipelineLayout &&other) noexcept
    : device(other.device), pipeline_layout(std::exchange(other.pipeline_layout, nullptr)), name(std::move(other.name)) {}

PipelineLayout::PipelineLayout(const VkDevice device, const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts, const std::string &name)
    : device(device), name(name) {
    assert(device);
    assert(!descriptor_set_layouts.empty());
    assert(!name.empty());

    VkPipelineLayoutCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    create_info.setLayoutCount = static_cast<std::uint32_t>(descriptor_set_layouts.size());
    create_info.pSetLayouts = descriptor_set_layouts.data();

    spdlog::debug("Creating pipeline layout {}.", name);

    if (vkCreatePipelineLayout(device, &create_info, nullptr, &pipeline_layout)) {
        throw std::runtime_error("Error: vkCreatePipelineLayout failed for " + name + " !");
    }

    // TODO: Assign an internal name to this pipeline layout using Vulkan debug markers.

    spdlog::debug("Created pipeline layout successfully.");
}

PipelineLayout::~PipelineLayout() {
    vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
