#pragma once

#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <volk.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::pipelines {
class PipelineCache;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::pipelines {

// TODO: Implement RAII wrapper for ComputePipeline

/// RAII wrapper for graphics pipelines
class GraphicsPipeline {
    friend class commands::CommandBuffer;
    friend class render_graph::RenderGraph;

private:
    const Device &m_device;
    std::string m_name;

    VkPipeline m_pipeline;
    std::unique_ptr<PipelineLayout> m_pipeline_layout;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param pipeline_cache The Vulkan pipeline cache
    /// @param descriptor_set_layouts The descriptor set layouts in the pipeline layout
    /// @param push_constant_ranges The push constant ranges in the pipeline layout
    /// @param pipeline_ci The pipeline create info
    /// @param name The internal debug name of the graphics pipeline
    GraphicsPipeline(const Device &device, const PipelineCache &pipeline_cache,
                     std::span<const VkDescriptorSetLayout> descriptor_set_layouts,
                     std::span<const VkPushConstantRange> push_constant_ranges,
                     VkGraphicsPipelineCreateInfo pipeline_ci, std::string name);

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&) noexcept;

    /// Call vkDestroyPipeline
    ~GraphicsPipeline();

    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;

    [[nodicsard]] auto pipeline() const {
        return m_pipeline;
    }

    [[nodicsard]] auto pipeline_layout() const {
        return m_pipeline_layout->m_pipeline_layout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
