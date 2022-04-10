#include "inexor/vulkan-renderer/wrapper/graphics_pipeline_builder.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

GraphicsPipelineBuilder::GraphicsPipelineBuilder(const Device &device) : m_device(device) {
    // Set viewport count and scissor count to 1 by default, so only when more than one viewport or scissor is required,
    // set_viewport_count and set_scissor_count must be called explicitely.
    m_viewport_sci.viewportCount = 1;
    m_viewport_sci.scissorCount = 1;
}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(GraphicsPipelineBuilder &&other) noexcept : m_device(other.m_device) {}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_dynamic_states(const std::vector<VkDynamicState> &dynamic_states) {
    m_dynamic_states_ci = make_info(dynamic_states);
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_scissor_count(const std::uint32_t scissor_count) {
    m_viewport_sci.scissorCount = scissor_count;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_viewport_count(const std::uint32_t viewport_count) {
    m_viewport_sci.viewportCount = viewport_count;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_color_blend_attachments(
    const std::vector<VkPipelineColorBlendAttachmentState> &attachments) {
    m_color_blend_sci = make_info(attachments);
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_vertex_input_attributes(
    const std::vector<VkVertexInputAttributeDescription> &vertex_input_attributes) {
    m_vertex_input_attributes = vertex_input_attributes;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_vertex_input_bindings(
    const std::vector<VkVertexInputBindingDescription> &vertex_input_bindings) {
    m_vertex_input_bindings = vertex_input_bindings;
    return *this;
}

GraphicsPipeline GraphicsPipelineBuilder::build(const PipelineLayout &m_pipeline_layout, const RenderPass &m_renderpass,
                                                const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages,
                                                std::string name) {
    assert(!shader_stages.empty());

    const auto vertex_input_sci = make_info(m_vertex_input_bindings, m_vertex_input_attributes);

    GraphicsPipeline pipeline(m_device,
                              make_info(m_pipeline_layout.pipeline_layout(), m_renderpass.renderpass(), shader_stages,
                                        &vertex_input_sci, &m_input_assembly_sci, &m_viewport_sci, &m_rasterization_sci,
                                        &m_multisample_sci, &m_depth_stencil_sci, &m_color_blend_sci,
                                        &m_dynamic_states_ci),
                              std::move(name));

    return std::move(pipeline);
}

} // namespace inexor::vulkan_renderer::wrapper
