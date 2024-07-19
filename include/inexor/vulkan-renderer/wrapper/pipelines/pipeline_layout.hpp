#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <span>

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::pipelines {

// Forward declaration
class GraphicsPipeline;

// Using declaration
using commands::CommandBuffer;
using render_graph::RenderGraph;

/// RAII wrapper class for VkPipelineLayout
class PipelineLayout {
    friend class RenderGraph;
    friend class GraphicsPipeline;
    friend class CommandBuffer;

private:
    const Device &m_device;
    std::string m_name;

    // There is no get method for this because only rendergraph needs to access it through friend class
    VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};

public:
    /// Call vkCreatePipelineLayout
    /// @note The constructor is private because only friend class RenderGraph needs access to it
    /// @param device The device wrapper
    /// @param name The name of the pipeline layout
    /// @param descriptor_set_layouts The descriptor set layouts of the pipeline layout
    /// @param push_constant_ranges The push constant ranges of the pipeline layout
    PipelineLayout(const Device &device,
                   std::string name,
                   std::span<const VkDescriptorSetLayout> descriptor_set_layouts,
                   std::span<const VkPushConstantRange> push_constant_ranges);

    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout(PipelineLayout &&) noexcept;

    /// Call vkDestroyPipelineLayout
    ~PipelineLayout();

    PipelineLayout &operator=(const PipelineLayout &) = delete;
    PipelineLayout &operator=(PipelineLayout &&other) noexcept = delete;
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
