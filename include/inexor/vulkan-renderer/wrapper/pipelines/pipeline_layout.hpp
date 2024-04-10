#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <span>

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::pipelines {

/// RAII wrapper class for VkPipelineLayout
class PipelineLayout {
private:
    friend render_graph::RenderGraph;

    const Device &m_device;
    std::string m_name;
    VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};

    /// Default constructor is private so that only RenderGraph can access it
    /// @param device The device wrapper
    /// @param descriptor_set_layouts The descriptor set layouts of the pipeline layout
    /// @param push_constant_ranges The push constant ranges of the pipeline layout
    /// @param name The name of the pipeline layout
    PipelineLayout(const Device &device, std::span<const VkDescriptorSetLayout> descriptor_set_layouts,
                   std::span<const VkPushConstantRange> push_constant_ranges, std::string name);

public:
    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout(PipelineLayout &&) noexcept;

    /// Call vkDestroyPipelineLayout
    ~PipelineLayout();

    PipelineLayout &operator=(const PipelineLayout &) = delete;
    PipelineLayout &operator=(PipelineLayout &&other) noexcept = delete;

    // TODO: Switch from get method to friend class
    [[nodiscard]] VkPipelineLayout pipeline_layout() const noexcept {
        return m_pipeline_layout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
