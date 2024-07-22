#pragma once

#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <memory>
#include <volk.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declarations
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declarations
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::pipelines {

// TODO: Implement compute pipelines

/// RAII wrapper for graphics pipelines
class GraphicsPipeline {
    friend class commands::CommandBuffer;
    friend class render_graph::RenderGraph;

private:
    const Device &m_device;
    std::unique_ptr<PipelineLayout> m_pipeline_layout{nullptr};
    VkPipeline m_pipeline{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param descriptor_set_layouts The descriptor set layouts in the pipeline layout
    /// @param push_constant_ranges The push constant ranges in the pipeline layout
    /// @param pipeline_ci The pipeline create info
    /// @param name The internal debug name of the graphics pipeline
    GraphicsPipeline(const Device &device,
                     std::vector<VkDescriptorSetLayout> descriptor_set_layouts,
                     std::vector<VkPushConstantRange> push_constant_ranges,
                     VkGraphicsPipelineCreateInfo pipeline_ci,
                     std::string name);

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&) noexcept;

    /// Call vkDestroyPipeline
    ~GraphicsPipeline();

    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
