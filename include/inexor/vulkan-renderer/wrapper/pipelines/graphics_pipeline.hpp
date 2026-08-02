#pragma once

#include "inexor/vulkan-renderer/tools/make_info.hpp"

#include <volk.h>

#include <memory>
#include <span>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::pipelines {
// Forward declarations
class PipelineLayout;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class RenderGraph;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::descriptors {
// Forward declaration
class PerFrameDescriptorSets;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::wrapper::pipelines {

// TODO: Implement RAII wrapper for ComputePipeline

// Using declaration
using tools::make_info;

/// When creating a graphics pipeline, the lifetime of certain data which is used to create the pipeline must be
/// ensured. In particular, the VkGraphicsPipelineCreateInfo struct must not be stored, however, the memory to which the
/// pointers inside of VkGraphicsPipelineCreateInfo point to must be stored. For example, VkGraphicsPipelineCreateInfo
/// has a member VkPipelineViewportStateCreateInfo, which itself has a pointer that point to VkViewport data, for
/// example. This means we must make sure the lifetime of all data that the pointers point to must be preserved.
/// Initially, we collected all the data to create the graphics pipeline in GraphicsPipelineBuilder, and reset all the
/// data of the builder after the build() method has been called. However, this is wrong, because the lifetime of the
/// data ends with calling reset(). This causes some bugs which are hard to find.
///
/// @TODO: Implement move constructor for GraphicsPipelineSetupData
struct GraphicsPipelineSetupData {
    // This is the underlying data for the create info structures
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages{};
    std::vector<VkVertexInputBindingDescription> vertex_input_binding_descriptions{};
    std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descriptions{};
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachment_states{};
    std::vector<VkViewport> viewports{};
    std::vector<VkRect2D> scissors{};
    std::vector<VkPushConstantRange> push_constant_ranges{};
    std::vector<VkDescriptorSetLayout> descriptor_set_layouts{};
    VkFormat depth_attachment_format{};
    VkFormat stencil_attachment_format{};
    std::vector<VkFormat> color_attachments{};
    std::vector<VkDynamicState> dynamic_states{};
    VkPipelineLayout pipeline_layout{VK_NULL_HANDLE};
    std::vector<std::weak_ptr<wrapper::descriptors::PerFrameDescriptorSets>> associated_descriptor_sets{};

    // These are the create info structures required to fill the VkGraphicsPipelineCreateInfo
    VkPipelineVertexInputStateCreateInfo vertex_input_sci{make_info<VkPipelineVertexInputStateCreateInfo>()};
    VkPipelineInputAssemblyStateCreateInfo input_assembly_sci{make_info<VkPipelineInputAssemblyStateCreateInfo>()};
    VkPipelineTessellationStateCreateInfo tesselation_sci{make_info<VkPipelineTessellationStateCreateInfo>()};
    VkPipelineViewportStateCreateInfo viewport_sci{make_info<VkPipelineViewportStateCreateInfo>()};
    VkPipelineRasterizationStateCreateInfo rasterization_sci{make_info<VkPipelineRasterizationStateCreateInfo>()};
    VkPipelineDepthStencilStateCreateInfo depth_stencil_sci{make_info<VkPipelineDepthStencilStateCreateInfo>()};
    VkPipelineRenderingCreateInfo pipeline_rendering_ci{make_info<VkPipelineRenderingCreateInfo>()};
    VkPipelineMultisampleStateCreateInfo multisample_sci{make_info<VkPipelineMultisampleStateCreateInfo>()};
    VkPipelineColorBlendStateCreateInfo color_blend_sci{make_info<VkPipelineColorBlendStateCreateInfo>()};
    VkPipelineDynamicStateCreateInfo dynamic_states_sci{make_info<VkPipelineDynamicStateCreateInfo>()};
};

/// RAII wrapper for graphics pipelines
class GraphicsPipeline {
private:
    const core::Device &m_device;
    std::string m_name;
    VkPipeline m_pipeline;
    std::unique_ptr<PipelineLayout> m_pipeline_layout;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param setup_data The graphics pipeline setup data
    /// @param name The internal debug name of the graphics pipeline
    GraphicsPipeline(const core::Device &device, GraphicsPipelineSetupData setup_data, std::string name);

    /// Call vkDestroyPipeline
    ~GraphicsPipeline();

    [[nodiscard]] VkPipeline pipeline() const {
        return m_pipeline;
    }

    [[nodiscard]] VkPipelineLayout pipeline_layout() const;
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
