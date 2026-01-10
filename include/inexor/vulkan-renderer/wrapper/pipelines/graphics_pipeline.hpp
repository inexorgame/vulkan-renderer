#pragma once

#include <volk.h>

#include <memory>
#include <span>
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
class PipelineLayout;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

// Forward declaration
using inexor::vulkan_renderer::wrapper::pipelines::PipelineLayout;

namespace inexor::vulkan_renderer::wrapper::pipelines {

// TODO: Implement RAII wrapper for ComputePipeline

/// When creating a graphics pipeline, the lifetime of certain data which is used to create the pipeline must be
/// ensured. In particular, the VkGraphicsPipelineCreateInfo struct must not be stored, however, the memory to which the
/// pointers inside of VkGraphicsPipelineCreateInfo point to must be stored. For example, VkGraphicsPipelineCreateInfo
/// has a member VkPipelineViewportStateCreateInfo, which itself has a pointer that point to VkViewport data, for
/// example. This means we must make sure the lifetime of all data that the pointers point to must be preserved.
/// Initially, we collected all the data to create the graphics pipeline in GraphicsPipelineBuilder, and reset all the
/// data of the builder after the build() method has been called. However, this is wrong, because the lifetime of the
/// data ends with calling reset(). This causes some bugs which are hard to find.
struct GraphicsPipelineSetupData {
    // @TODO Add whatever we need here!
    // This is the underlying data for the create info structures
    std::vector<VkPipelineShaderStageCreateInfo> m_shader_stages;
    std::vector<VkVertexInputBindingDescription> m_vertex_input_binding_descriptions;
    std::vector<VkVertexInputAttributeDescription> m_vertex_input_attribute_descriptions;
    std::vector<VkPipelineColorBlendAttachmentState> m_color_blend_attachment_states;
    std::vector<VkViewport> m_viewports;
    std::vector<VkRect2D> m_scissors;
    std::vector<VkPushConstantRange> m_push_constant_ranges;
    std::vector<VkDescriptorSetLayout> m_descriptor_set_layouts;
    VkRenderPass m_render_pass;
    VkFormat m_depth_attachment_format;
    VkFormat m_stencil_attachment_format;
    std::vector<VkFormat> m_color_attachments;
    std::vector<VkDynamicState> m_dynamic_states;
    VkPipelineLayout m_pipeline_layout;

    // These are the create info structures required to fill the VkGraphicsPipelineCreateInfo
    VkPipelineVertexInputStateCreateInfo m_vertex_input_sci;
    VkPipelineInputAssemblyStateCreateInfo m_input_assembly_sci;
    VkPipelineTessellationStateCreateInfo m_tesselation_sci;
    VkPipelineViewportStateCreateInfo m_viewport_sci;
    VkPipelineRasterizationStateCreateInfo m_rasterization_sci;
    VkPipelineDepthStencilStateCreateInfo m_depth_stencil_sci;
    VkPipelineRenderingCreateInfo m_pipeline_rendering_ci;
    VkPipelineMultisampleStateCreateInfo m_multisample_sci;
    VkPipelineColorBlendStateCreateInfo m_color_blend_sci;
    VkPipelineDynamicStateCreateInfo m_dynamic_states_sci;

    // @TODO: Implement move constructor for GraphicsPipelineSetupData?
};

/// RAII wrapper for graphics pipelines
class GraphicsPipeline {
    friend class commands::CommandBuffer;
    friend class render_graph::RenderGraph;

private:
    const Device &m_device;
    std::string m_name;
    GraphicsPipelineSetupData m_pipeline_setup_data;

    VkPipeline m_pipeline;
    std::unique_ptr<PipelineLayout> m_pipeline_layout;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param pipeline_cache The Vulkan pipeline cache
    /// @param descriptor_set_layouts The descriptor set layouts in the pipeline layout
    /// @param push_constant_ranges The push constant ranges in the pipeline layout
    /// @param pipeline_setup_data The graphics pipeline setup data
    /// @param name The internal debug name of the graphics pipeline
    GraphicsPipeline(const Device &device, const PipelineCache &pipeline_cache,
                     std::span<const VkDescriptorSetLayout> descriptor_set_layouts,
                     std::span<const VkPushConstantRange> push_constant_ranges,
                     GraphicsPipelineSetupData pipeline_setup_data, std::string name);

    GraphicsPipeline(const GraphicsPipeline &) = delete;
    GraphicsPipeline(GraphicsPipeline &&) noexcept;

    /// Call vkDestroyPipeline
    ~GraphicsPipeline();

    GraphicsPipeline &operator=(const GraphicsPipeline &) = delete;
    GraphicsPipeline &operator=(GraphicsPipeline &&) = delete;

    [[nodiscard]] auto pipeline() const {
        return m_pipeline;
    }

    [[nodiscard]] VkPipelineLayout pipeline_layout() const;
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
