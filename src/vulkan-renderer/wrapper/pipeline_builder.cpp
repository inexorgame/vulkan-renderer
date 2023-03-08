#include "inexor/vulkan-renderer/wrapper/pipeline_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

GraphicsPipelineBuilder::GraphicsPipelineBuilder(const Device &device) : m_device(device) {}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(GraphicsPipelineBuilder &&other) noexcept : m_device(other.m_device) {
    m_vertex_input_sci = std::move(other.m_vertex_input_sci);
    m_input_assembly_sci = std::move(other.m_input_assembly_sci);
    m_tesselation_sci = std::move(other.m_tesselation_sci);
    m_viewport_sci = std::move(other.m_viewport_sci);
    m_rasterization_sci = std::move(m_rasterization_sci);
    m_multisample_sci = std::move(other.m_multisample_sci);
    m_depth_stencil_sci = std::move(other.m_depth_stencil_sci);
    m_color_blend_sci = std::move(other.m_color_blend_sci);
    m_dynamic_states_sci = std::move(other.m_dynamic_states_sci);
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, VK_NULL_HANDLE);
    m_render_pass = std::exchange(other.m_render_pass, VK_NULL_HANDLE);
    m_dynamic_states = std::move(other.m_dynamic_states);
    m_viewports = std::move(other.m_viewports);
    m_scissors = std::move(other.m_scissors);
    m_shader_stages = std::move(other.m_shader_stages);
    m_vertex_input_binding_descriptions = std::move(other.m_vertex_input_binding_descriptions);
    m_vertex_input_attribute_descriptions = std::move(other.m_vertex_input_attribute_descriptions);
    m_color_blend_attachment_states = std::move(other.m_color_blend_attachment_states);
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_shader(const VkPipelineShaderStageCreateInfo &shader_stage) {
    m_shader_stages.push_back(shader_stage);
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::add_color_blend_attachment(const VkPipelineColorBlendAttachmentState &attachment) {
    m_color_blend_attachment_states.push_back(attachment);
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::add_vertex_input_attribute(const VkVertexInputAttributeDescription &description) {
    m_vertex_input_attribute_descriptions.push_back(description);
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::add_vertex_input_binding(const VkVertexInputBindingDescription &description) {
    m_vertex_input_binding_descriptions.push_back(description);
    return *this;
}

std::unique_ptr<GraphicsPipeline> GraphicsPipelineBuilder::build(std::string name) {
    m_vertex_input_sci = make_info<VkPipelineVertexInputStateCreateInfo>({
        .vertexBindingDescriptionCount = static_cast<std::uint32_t>(m_vertex_input_binding_descriptions.size()),
        .pVertexBindingDescriptions = m_vertex_input_binding_descriptions.data(),
        .vertexAttributeDescriptionCount = static_cast<std::uint32_t>(m_vertex_input_attribute_descriptions.size()),
        .pVertexAttributeDescriptions = m_vertex_input_attribute_descriptions.data(),

    });

    m_viewport_sci = make_info<VkPipelineViewportStateCreateInfo>({
        .viewportCount = static_cast<uint32_t>(m_viewports.size()),
        .pViewports = m_viewports.data(),
        .scissorCount = static_cast<uint32_t>(m_scissors.size()),
        .pScissors = m_scissors.data(),
    });

    m_dynamic_states_sci = make_info<VkPipelineDynamicStateCreateInfo>({
        .dynamicStateCount = static_cast<std::uint32_t>(m_dynamic_states.size()),
        .pDynamicStates = m_dynamic_states.data(),
    });

    return std::make_unique<GraphicsPipeline>(m_device,
                                              make_info<VkGraphicsPipelineCreateInfo>({
                                                  .stageCount = static_cast<std::uint32_t>(m_shader_stages.size()),
                                                  .pStages = m_shader_stages.data(),
                                                  .pVertexInputState = &m_vertex_input_sci,
                                                  .pInputAssemblyState = &m_input_assembly_sci,
                                                  .pTessellationState = &m_tesselation_sci,
                                                  .pViewportState = &m_viewport_sci,
                                                  .pRasterizationState = &m_rasterization_sci,
                                                  .pMultisampleState = &m_multisample_sci,
                                                  .pDepthStencilState = &m_depth_stencil_sci,
                                                  .pColorBlendState = &m_color_blend_sci,
                                                  .pDynamicState = &m_dynamic_states_sci,
                                                  .layout = m_pipeline_layout,
                                                  .renderPass = m_render_pass,
                                              }),
                                              std::move(name));
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_color_blend(const VkPipelineColorBlendStateCreateInfo &color_blend) {
    m_color_blend_sci = color_blend;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_color_blend_attachments(
    const std::vector<VkPipelineColorBlendAttachmentState> &attachments) {
    m_color_blend_attachment_states = attachments;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_culling_mode(const VkBool32 culling_enabled) {
    spdlog::warn("Culling is disabled, which could have negative effects on the performance!");
    m_rasterization_sci.cullMode = culling_enabled ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_depth_stencil(const VkPipelineDepthStencilStateCreateInfo &depth_stencil) {
    m_depth_stencil_sci = depth_stencil;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_dynamic_states(const std::vector<VkDynamicState> &dynamic_states) {
    assert(!dynamic_states.empty());
    m_dynamic_states = dynamic_states;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_input_assembly(const VkPipelineInputAssemblyStateCreateInfo &input_assembly) {
    m_input_assembly_sci = input_assembly;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_line_width(const float width) {
    m_rasterization_sci.lineWidth = width;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_pipeline_layout(const VkPipelineLayout layout) {
    assert(layout);
    m_pipeline_layout = layout;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_primitive_topology(const VkPrimitiveTopology topology) {
    m_input_assembly_sci = make_info<VkPipelineInputAssemblyStateCreateInfo>({
        .topology = topology,
    });
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_rasterization(const VkPipelineRasterizationStateCreateInfo &rasterization) {
    m_rasterization_sci = rasterization;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_render_pass(const VkRenderPass render_pass) {
    assert(render_pass);
    m_render_pass = render_pass;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_scissor(const VkRect2D &scissor) {
    m_scissors = {scissor};
    m_viewport_sci.scissorCount = 1;
    m_viewport_sci.pScissors = m_scissors.data();
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_scissors(const std::vector<VkRect2D> &scissors) {
    assert(!scissors.empty());
    m_scissors = scissors;
    m_viewport_sci.scissorCount = static_cast<std::uint32_t>(scissors.size());
    m_viewport_sci.pScissors = scissors.data();
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_shaders(const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages) {
    assert(!shader_stages.empty());
    m_shader_stages = shader_stages;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_tesselation(const std::uint32_t control_points) {
    m_tesselation_sci.patchControlPoints = control_points;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_vertex_input_attributes(
    const std::vector<VkVertexInputAttributeDescription> &descriptions) {
    assert(!descriptions.empty());
    m_vertex_input_attribute_descriptions = descriptions;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_vertex_input_bindings(const std::vector<VkVertexInputBindingDescription> &descriptions) {
    assert(!descriptions.empty());
    m_vertex_input_binding_descriptions = descriptions;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_viewport(const VkViewport &viewport) {
    m_viewports = {viewport};
    m_viewport_sci.viewportCount = 1;
    m_viewport_sci.pViewports = m_viewports.data();
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_viewports(const std::vector<VkViewport> &viewports) {
    assert(!viewports.empty());
    m_viewports = viewports;
    m_viewport_sci.viewportCount = static_cast<std::uint32_t>(m_viewports.size());
    m_viewport_sci.pViewports = m_viewports.data();
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_wireframe(const VkBool32 wireframe) {
    m_rasterization_sci.polygonMode = (wireframe == VK_TRUE) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    return *this;
}

} // namespace inexor::vulkan_renderer::wrapper
