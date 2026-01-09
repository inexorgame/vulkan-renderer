#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

// Using declaration
using wrapper::InexorException;

GraphicsPipelineBuilder::GraphicsPipelineBuilder(const Device &device, const PipelineCache &pipeline_cache)
    : m_device(device), m_pipeline_cache(pipeline_cache) {
    reset();
}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(GraphicsPipelineBuilder &&other) noexcept
    : m_device(other.m_device), m_pipeline_cache(other.m_pipeline_cache) {
    // TODO: Check me!
    m_pipeline_rendering_ci = std::move(other.m_pipeline_rendering_ci);
    m_color_attachments = std::move(other.m_color_attachments);
    m_depth_attachment_format = other.m_depth_attachment_format;
    m_stencil_attachment_format = other.m_stencil_attachment_format;
    m_shader_stages = std::move(other.m_shader_stages);
    m_vertex_input_binding_descriptions = std::move(other.m_vertex_input_binding_descriptions);
    m_vertex_input_attribute_descriptions = std::move(other.m_vertex_input_attribute_descriptions);
    m_vertex_input_sci = std::move(other.m_vertex_input_sci);
    m_input_assembly_sci = std::move(other.m_input_assembly_sci);
    m_tesselation_sci = std::move(other.m_tesselation_sci);
    m_viewport_sci = std::move(other.m_viewport_sci);
    m_viewports = std::move(other.m_viewports);
    m_scissors = std::move(other.m_scissors);
    m_rasterization_sci = std::move(m_rasterization_sci);
    m_multisample_sci = std::move(other.m_multisample_sci);
    m_depth_stencil_sci = std::move(other.m_depth_stencil_sci);
    m_color_blend_sci = std::move(other.m_color_blend_sci);
    m_dynamic_states = std::move(other.m_dynamic_states);
    m_dynamic_states_sci = std::move(other.m_dynamic_states_sci);
    m_pipeline_layout = std::exchange(other.m_pipeline_layout, VK_NULL_HANDLE);
    m_color_blend_attachment_states = std::move(other.m_color_blend_attachment_states);
}

std::shared_ptr<GraphicsPipeline> GraphicsPipelineBuilder::build(std::string name, bool use_dynamic_rendering) {
    if (name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
    // NOTE: Inside of GraphicsPipelineBuilder, we carry out no error checks when it comes to the data which is used to
    // build the graphics pipeline. This is because validation of this data is job of the validation layers, and not the
    // job of GraphicsPipelineBuilder. We should not mimic the behavious of validation layers here.

    if (use_dynamic_rendering) {
        m_pipeline_rendering_ci = make_info<VkPipelineRenderingCreateInfo>({
            // TODO: Support multiview rendering and expose viewMask parameter
            .colorAttachmentCount = static_cast<std::uint32_t>(m_color_attachments.size()),
            .pColorAttachmentFormats = m_color_attachments.data(),
            .depthAttachmentFormat = m_depth_attachment_format,
            .stencilAttachmentFormat = m_stencil_attachment_format,
        });
    }

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

    m_color_blend_sci = wrapper::make_info<VkPipelineColorBlendStateCreateInfo>({
        .attachmentCount = static_cast<std::uint32_t>(m_color_blend_attachment_states.size()),
        .pAttachments = m_color_blend_attachment_states.data(),
    });

    m_dynamic_states_sci = make_info<VkPipelineDynamicStateCreateInfo>({
        .dynamicStateCount = static_cast<std::uint32_t>(m_dynamic_states.size()),
        .pDynamicStates = m_dynamic_states.data(),
    });

    // @TODO Fix this once we move away from renderpasses!
    auto pipeline_ci = make_info<VkGraphicsPipelineCreateInfo>({
        // NOTE: This is one of those rare cases where pNext is actually not nullptr!
        .pNext = (use_dynamic_rendering) ? &m_pipeline_rendering_ci : nullptr,
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
        // @TODO Make this VK_NULL_HANDLE and use dynamic rendering!
        .renderPass = (use_dynamic_rendering) ? VK_NULL_HANDLE : m_render_pass,
    });

    Sleep(2000);

    auto graphics_pipeline =
        std::make_shared<GraphicsPipeline>(m_device, m_pipeline_cache, m_descriptor_set_layouts, m_push_constant_ranges,
                                           std::move(pipeline_ci), std::move(name));

    // NOTE: We reset the data of the builder here so it can be re-used
    reset();

    // Return the graphics pipeline we created
    return graphics_pipeline;
}

void GraphicsPipelineBuilder::reset() {
    m_pipeline_rendering_ci = make_info<VkPipelineRenderingCreateInfo>();
    m_color_attachments.clear();
    m_depth_attachment_format = VK_FORMAT_UNDEFINED;
    m_stencil_attachment_format = VK_FORMAT_UNDEFINED;

    m_shader_stages.clear();
    m_vertex_input_binding_descriptions.clear();
    m_vertex_input_attribute_descriptions.clear();
    m_vertex_input_sci = make_info<VkPipelineVertexInputStateCreateInfo>();

    m_input_assembly_sci = make_info<VkPipelineInputAssemblyStateCreateInfo>({
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    });

    m_tesselation_sci = make_info<VkPipelineTessellationStateCreateInfo>();

    m_viewports.clear();
    m_scissors.clear();

    m_viewport_sci = make_info<VkPipelineViewportStateCreateInfo>();

    m_rasterization_sci = make_info<VkPipelineRasterizationStateCreateInfo>({
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .lineWidth = 1.0f,
    });

    m_multisample_sci = make_info<VkPipelineMultisampleStateCreateInfo>({
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
    });

    m_depth_stencil_sci = make_info<VkPipelineDepthStencilStateCreateInfo>();
    m_color_blend_sci = make_info<VkPipelineColorBlendStateCreateInfo>();

    m_dynamic_states.clear();
    m_dynamic_states_sci = make_info<VkPipelineDynamicStateCreateInfo>();

    m_pipeline_layout = VK_NULL_HANDLE;
    m_color_blend_attachment_states.clear();

    m_push_constant_ranges.clear();
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_color_attachment_format(const VkFormat format) {
    m_color_attachments.push_back(format);
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::add_color_blend_attachment(const VkPipelineColorBlendAttachmentState &attachment) {
    m_color_blend_attachment_states.push_back(attachment);
    return *this;
}

/// Add the default color blend attachment
/// @return A reference to the dereferenced this pointer (allows method calls to be chained)
[[nodiscard]] GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_default_color_blend_attachment() {
    return add_color_blend_attachment({
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    });
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_push_constant_range(const VkShaderStageFlags shader_stage,
                                                                          const std::uint32_t size,
                                                                          const std::uint32_t offset) {
    m_push_constant_ranges.emplace_back(VkPushConstantRange{
        .stageFlags = shader_stage,
        .offset = offset,
        .size = size,
    });
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_shader(std::weak_ptr<Shader> shader) {
    m_shader_stages.emplace_back(wrapper::make_info<VkPipelineShaderStageCreateInfo>({
        .stage = shader.lock()->shader_stage(),
        .module = shader.lock()->shader_module(),
        .pName = shader.lock()->entry_point().c_str(),
    }));
    return *this;
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
    if (culling_enabled == VK_FALSE) {
        spdlog::warn("Culling is disabled, which could have negative effects on the performance!");
    }
    m_rasterization_sci.cullMode = culling_enabled == VK_TRUE ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_depth_attachment_format(const VkFormat format) {
    m_depth_attachment_format = format;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_depth_stencil(const VkPipelineDepthStencilStateCreateInfo &depth_stencil) {
    m_depth_stencil_sci = depth_stencil;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_stencil_attachment_format(const VkFormat format) {
    m_stencil_attachment_format = format;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_descriptor_set_layout(const VkDescriptorSetLayout descriptor_set_layout) {
    assert(descriptor_set_layout);
    m_descriptor_set_layouts = {descriptor_set_layout};
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_descriptor_set_layouts(std::vector<VkDescriptorSetLayout> descriptor_set_layouts) {
    assert(!descriptor_set_layouts.empty());
    m_descriptor_set_layouts = std::move(descriptor_set_layouts);
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

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_multisampling(const VkSampleCountFlagBits sample_count,
                                                                    const std::optional<float> min_sample_shading) {
    m_multisample_sci.rasterizationSamples = sample_count;
    if (min_sample_shading) {
        m_multisample_sci.minSampleShading = min_sample_shading.value();
    }
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_pipeline_layout(const VkPipelineLayout layout) {
    assert(layout);
    m_pipeline_layout = layout;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_push_constant_ranges(std::vector<VkPushConstantRange> push_constant_ranges) {
    m_push_constant_ranges = std::move(push_constant_ranges);
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_primitive_topology(const VkPrimitiveTopology topology) {
    m_input_assembly_sci.topology = topology;
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_rasterization(const VkPipelineRasterizationStateCreateInfo &rasterization) {
    m_rasterization_sci = rasterization;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_render_pass(const VkRenderPass &render_pass) {
    m_render_pass = render_pass;
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_scissor(const VkRect2D &scissor) {
    m_scissors = {scissor};
    m_viewport_sci.scissorCount = 1;
    m_viewport_sci.pScissors = m_scissors.data();
    return *this;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_scissor(const VkExtent2D &extent) {
    return set_scissor({
        // Convert VkExtent2D to VkRect2D
        .extent = extent,
    });
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_shaders(std::vector<VkPipelineShaderStageCreateInfo> shaders) {
    assert(!shaders.empty());
    m_shader_stages = std::move(shaders);
    return *this;
}

GraphicsPipelineBuilder &
GraphicsPipelineBuilder::set_tesselation_control_point_count(const std::uint32_t control_point_count) {
    m_tesselation_sci.patchControlPoints = control_point_count;
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

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_viewport(const VkExtent2D &extent) {
    return set_viewport({
        // Convert VkExtent2D to VkViewport
        .width = static_cast<float>(extent.width),
        .height = static_cast<float>(extent.height),
        .maxDepth = 1.0f,
    });
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_wireframe(const VkBool32 wireframe) {
    m_rasterization_sci.polygonMode = (wireframe == VK_TRUE) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
    return *this;
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
