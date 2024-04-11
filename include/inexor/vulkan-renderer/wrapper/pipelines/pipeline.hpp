#pragma once

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

/// RAII wrapper for VkPipeline
// TODO: Compute pipelines
class GraphicsPipeline {
    // The CommandBuffer wrapper needs to access m_pipeline
    friend commands::CommandBuffer;
    friend render_graph::RenderGraph;

private:
    const Device &m_device;
    std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
    std::vector<VkPushConstantRange> m_push_constant_ranges;
    VkPipeline m_pipeline{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// Default constructor is private so that only RenderGraph and CommandBuffer can access it
    /// @param device The device wrapper
    /// @param pipeline_ci The pipeline create info
    /// @param name The internal debug name of the graphics pipeline
    GraphicsPipeline(const Device &device, const VkGraphicsPipelineCreateInfo &pipeline_ci, std::string name);

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&) noexcept;

    /// Call vkDestroyPipeline
    ~GraphicsPipeline();

    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;

    [[nodiscard]] auto &descriptor_set_layouts() const {
        return m_descriptor_set_layouts;
    }

    [[nodiscard]] auto &name() const {
        return m_name;
    }

    [[nodiscard]] auto &push_constant_ranges() const {
        return m_push_constant_ranges;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
