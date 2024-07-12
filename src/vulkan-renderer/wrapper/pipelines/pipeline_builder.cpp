#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

GraphicsPipelineBuilder::GraphicsPipelineBuilder(const Device &device) : m_device(device) {
    reset();
}

GraphicsPipelineBuilder::GraphicsPipelineBuilder(GraphicsPipelineBuilder &&other) noexcept : m_device(other.m_device) {
    m_depth_attachment_format = other.m_depth_attachment_format;
    m_stencil_attachment_format = other.m_stencil_attachment_format;
    m_color_attachments = std::move(other.m_color_attachments);
    m_pipeline_rendering_ci = std::move(other.m_pipeline_rendering_ci);
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
    m_dynamic_states = std::move(other.m_dynamic_states);
    m_viewports = std::move(other.m_viewports);
    m_scissors = std::move(other.m_scissors);
    m_shader_stages = std::move(other.m_shader_stages);
    m_vertex_input_binding_descriptions = std::move(other.m_vertex_input_binding_descriptions);
    m_vertex_input_attribute_descriptions = std::move(other.m_vertex_input_attribute_descriptions);
    m_color_blend_attachment_states = std::move(other.m_color_blend_attachment_states);
}

std::shared_ptr<GraphicsPipeline> GraphicsPipelineBuilder::build(std::string name) {
    if (name.empty()) {
        throw std::invalid_argument("Error: No name specified for graphics pipeline in GraphicsPipelineBuilder!");
    }
    // NOTE: Inside of GraphicsPipelineBuilder, we do almost no error checks when it comes to the data which is used to
    // build the graphics pipeline. This is because validation of this data is job of the validation layers, and not the
    // job of GraphicsPipelineBuilder. We don't need to mimic the behavious of validation layers in here.

    // We don't really need all the make_infos here, as we initialized it all in reset() already,
    // but it makes the code look cleaner and more consistent
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

    if (!m_dynamic_states.empty()) {
        m_dynamic_states_sci = make_info<VkPipelineDynamicStateCreateInfo>({
            .dynamicStateCount = static_cast<std::uint32_t>(m_dynamic_states.size()),
            .pDynamicStates = m_dynamic_states.data(),
        });
    }

    m_pipeline_rendering_ci = make_info<VkPipelineRenderingCreateInfo>({
        // NOTE: Because we pass m_pipeline_rendering_ci as pNext parameter
        // in graphics_pipeline below, we need to end the pNext chain here!
        .pNext = nullptr,
        .colorAttachmentCount = static_cast<std::uint32_t>(m_color_attachments.size()),
        .pColorAttachmentFormats = m_color_attachments.data(),
        .depthAttachmentFormat = m_depth_attachment_format,
        .stencilAttachmentFormat = m_stencil_attachment_format,
    });

    m_color_blend_sci = wrapper::make_info<VkPipelineColorBlendStateCreateInfo>({
        .attachmentCount = static_cast<std::uint32_t>(m_color_blend_attachment_states.size()),
        .pAttachments = m_color_blend_attachment_states.data(),
    });

    auto graphics_pipeline = std::make_shared<GraphicsPipeline>(
        m_device, std::vector{m_descriptor_set_layout}, m_push_constant_ranges,
        make_info<VkGraphicsPipelineCreateInfo>({
            // NOTE: This is one of those rare cases where pNext is actually not nullptr!
            .pNext = &m_pipeline_rendering_ci,
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
            // NOTE: This is VK_NULL_HANDLE because we use dynamic rendering
            .renderPass = VK_NULL_HANDLE,
        }),
        std::move(name));

    // NOTE: The data of the builder can be reset now that the graphics pipeline was created
    reset();

    // Return the graphics pipeline we created
    return graphics_pipeline;
}

void GraphicsPipelineBuilder::reset() {
    m_color_attachments.clear();
    m_depth_attachment_format = VK_FORMAT_UNDEFINED;
    m_stencil_attachment_format = VK_FORMAT_UNDEFINED;
    m_pipeline_layout = VK_NULL_HANDLE;
    m_color_blend_attachment_states.clear();
    m_shader_stages.clear();

    m_vertex_input_binding_descriptions.clear();
    m_vertex_input_attribute_descriptions.clear();
    m_vertex_input_sci = {
        make_info<VkPipelineVertexInputStateCreateInfo>(),
    };

    m_input_assembly_sci = {
        make_info<VkPipelineInputAssemblyStateCreateInfo>({
            .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            .primitiveRestartEnable = VK_FALSE,
        }),
    };

    m_tesselation_sci = {
        make_info<VkPipelineTessellationStateCreateInfo>(),
    };

    m_viewports.clear();
    m_scissors.clear();
    m_viewport_sci = {
        make_info<VkPipelineViewportStateCreateInfo>(),
    };

    m_rasterization_sci = {
        make_info<VkPipelineRasterizationStateCreateInfo>({
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_CLOCKWISE,
            .lineWidth = 1.0f,
        }),
    };

    m_multisample_sci = {
        make_info<VkPipelineMultisampleStateCreateInfo>({
            .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            .sampleShadingEnable = VK_FALSE,
            .minSampleShading = 1.0f,
        }),
    };

    m_depth_stencil_sci = {
        make_info<VkPipelineDepthStencilStateCreateInfo>(),
    };

    m_color_blend_sci = {
        make_info<VkPipelineColorBlendStateCreateInfo>(),
    };

    m_dynamic_states.clear();
    m_dynamic_states_sci = {
        make_info<VkPipelineDynamicStateCreateInfo>(),
    };
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
