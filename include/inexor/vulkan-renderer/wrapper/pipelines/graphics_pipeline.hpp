#pragma once

#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <volk.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::pipelines {

// TODO: Implement RAII wrapper ComputePipeline for compute pipelines!

/// RAII wrapper for graphics pipelines
class GraphicsPipeline {
    friend render_graph::RenderGraph;

private:
    const Device &m_device;
    std::string m_name;
    VkPipeline m_pipeline{VK_NULL_HANDLE};
    std::unique_ptr<PipelineLayout> m_pipeline_layout;
    VkGraphicsPipelineCreateInfo m_pipeline_ci;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param descriptor_set_layouts The descriptor set layouts in the pipeline layout
    /// @param push_constant_ranges The push constant ranges in the pipeline layout
    /// @param pipeline_ci The pipeline create info
    /// @param name The internal debug name of the graphics pipeline
    GraphicsPipeline(const Device &device,
                     std::span<const VkDescriptorSetLayout> descriptor_set_layouts,
                     std::span<const VkPushConstantRange> push_constant_ranges,
                     VkGraphicsPipelineCreateInfo pipeline_ci,
                     std::string name);

    /// Call vkDestroyPipeline
    /// @note We batch pipeline creation into one call to vkCreateGraphicsPipelines in rendergraph, but the actual
    /// VkPipeline handle is still handled by the GraphicsPipeline wrapper, meaning that it will be destroyed in the
    /// destructor, and not by rendergraph!
    ~GraphicsPipeline();
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
