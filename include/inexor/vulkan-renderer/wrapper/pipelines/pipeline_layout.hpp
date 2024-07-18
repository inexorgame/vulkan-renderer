#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <span>

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::pipelines {

// Using declaration
using render_graph::RenderGraph;

/// RAII wrapper class for VkPipelineLayout
class PipelineLayout {
private:
    // Rendergraph needs to read from m_pipeline_layout
    friend RenderGraph;

    const Device &m_device;
    std::string m_name;

    // There is no get method for this because only rendergraph needs to access it through friend class
    VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};

    /// Call vkCreatePipelineLayout
    /// @note The constructor is private because only friend class RenderGraph needs access to it
    /// @param device The device wrapper
    /// @param desc_set_layouts The descriptor set layouts of the pipeline layout
    /// @param push_constant_ranges The push constant ranges of the pipeline layout
    /// @param name The name of the pipeline layout
    PipelineLayout(const Device &device,
                   std::span<const VkDescriptorSetLayout> desc_set_layouts,
                   std::span<const VkPushConstantRange> push_constant_ranges,
                   std::string name);

public:
    PipelineLayout(const PipelineLayout &) = delete;
    PipelineLayout(PipelineLayout &&) noexcept;

    /// Call vkDestroyPipelineLayout
    ~PipelineLayout();

    PipelineLayout &operator=(const PipelineLayout &) = delete;
    PipelineLayout &operator=(PipelineLayout &&other) noexcept = delete;
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
