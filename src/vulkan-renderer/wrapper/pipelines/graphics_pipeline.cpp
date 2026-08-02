#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/per_frame_descriptor_sets.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_layout.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

using tools::InexorException;
using tools::VulkanException;

GraphicsPipeline::GraphicsPipeline(const core::Device &device, GraphicsPipelineSetupData setup_data, std::string name)
    : m_device(device), m_name(std::move(name)) {

    spdlog::trace("   - Building graphics pipeline [{}]", m_name);

    // NOTE: It's important to fill VkGraphicsPipelineCreateInfo in the constructor of GraphicsPipeline!
    // GraphicsPipeline wrapper is responsible for keeping the memory alive which is in VkGraphicsPipelineCreateInfo!
    // If we would store GraphicsPipelineSetupData in the graphics pipeline builder, the created graphics pipeline would
    // become invalidate as soon as the graphics pipeline builder finishes building the pipeline because this will reset
    // the graphics pipeline builder so it can be re-used. This is one of the small but extremely important things to
    // keep in mind when dealing with Vulkan API: object lifetime!

    setup_data.pipeline_rendering_ci = make_info<VkPipelineRenderingCreateInfo>({
        // TODO: Support multiview rendering and expose viewMask parameter
        .colorAttachmentCount = static_cast<std::uint32_t>(setup_data.color_attachments.size()),
        .pColorAttachmentFormats = setup_data.color_attachments.data(),
        .depthAttachmentFormat = setup_data.depth_attachment_format,
        .stencilAttachmentFormat = setup_data.stencil_attachment_format,
    });

    setup_data.vertex_input_sci = make_info<VkPipelineVertexInputStateCreateInfo>({
        .vertexBindingDescriptionCount =
            static_cast<std::uint32_t>(setup_data.vertex_input_binding_descriptions.size()),
        .pVertexBindingDescriptions = setup_data.vertex_input_binding_descriptions.data(),
        .vertexAttributeDescriptionCount =
            static_cast<std::uint32_t>(setup_data.vertex_input_attribute_descriptions.size()),
        .pVertexAttributeDescriptions = setup_data.vertex_input_attribute_descriptions.data(),

    });

    setup_data.viewport_sci = make_info<VkPipelineViewportStateCreateInfo>({
        .viewportCount = static_cast<uint32_t>(setup_data.viewports.size()),
        .pViewports = setup_data.viewports.data(),
        .scissorCount = static_cast<uint32_t>(setup_data.scissors.size()),
        .pScissors = setup_data.scissors.data(),
    });

    // If we use VK_DYNAMIC_STATE_SCISSOR, we set scissor count to 1
    /// @TODO Implement multiple dynamic scissors with VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT
    if (std::find(setup_data.dynamic_states.begin(), setup_data.dynamic_states.end(), VK_DYNAMIC_STATE_SCISSOR) !=
        setup_data.dynamic_states.end()) {
        // Even with scissor as dynamic state enabled, there must be at least one scissor here!
        setup_data.viewport_sci.scissorCount = 1;
        setup_data.viewport_sci.pScissors = nullptr;
    }

    // If we use VK_DYNAMIC_STATE_VIEWPORT, we set scissor count to 1
    /// @TODO Implement multiple dynamic viewports with VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT
    if (std::find(setup_data.dynamic_states.begin(), setup_data.dynamic_states.end(), VK_DYNAMIC_STATE_VIEWPORT) !=
        setup_data.dynamic_states.end()) {
        // Even with viewport as dynamic state enabled, there must be at least one viewport here!
        setup_data.viewport_sci.viewportCount = 1;
        setup_data.viewport_sci.pViewports = nullptr;
    }

    setup_data.color_blend_sci = make_info<VkPipelineColorBlendStateCreateInfo>({
        .attachmentCount = static_cast<std::uint32_t>(setup_data.color_blend_attachment_states.size()),
        .pAttachments = setup_data.color_blend_attachment_states.data(),
    });

    setup_data.dynamic_states_sci = make_info<VkPipelineDynamicStateCreateInfo>({
        .dynamicStateCount = static_cast<std::uint32_t>(setup_data.dynamic_states.size()),
        .pDynamicStates = setup_data.dynamic_states.data(),
    });

    // @TODO Expose the pipeline layout as parameter
    m_pipeline_layout = std::make_unique<PipelineLayout>(m_device, m_name, setup_data.descriptor_set_layouts,
                                                         setup_data.push_constant_ranges);

    for (const auto &descriptor_set : setup_data.associated_descriptor_sets) {
        const auto descriptor_set_ref = descriptor_set.lock();
        if (!descriptor_set_ref) {
            throw InexorException("Error: Associated descriptor set is invalid!");
        }
        descriptor_set_ref->set_pipeline_layout(m_pipeline_layout->pipeline_layout());
    }

    auto pipeline_ci = make_info<VkGraphicsPipelineCreateInfo>({
        // NOTE: This is one of those rare cases where pNext is actually not nullptr!
        .pNext = &setup_data.pipeline_rendering_ci,
        .stageCount = static_cast<std::uint32_t>(setup_data.shader_stages.size()),
        .pStages = setup_data.shader_stages.data(),
        .pVertexInputState = &setup_data.vertex_input_sci,
        .pInputAssemblyState = &setup_data.input_assembly_sci,
        .pTessellationState = &setup_data.tesselation_sci,
        .pViewportState = &setup_data.viewport_sci,
        .pRasterizationState = &setup_data.rasterization_sci,
        .pMultisampleState = &setup_data.multisample_sci,
        .pDepthStencilState = &setup_data.depth_stencil_sci,
        .pColorBlendState = &setup_data.color_blend_sci,
        .pDynamicState = &setup_data.dynamic_states_sci,
        .layout = m_pipeline_layout->pipeline_layout(),
        .renderPass = VK_NULL_HANDLE,
    });

    // Create the graphics pipeline
    if (const auto result = vkCreateGraphicsPipelines(m_device.device(), m_device.pipeline_cache(), 1, &pipeline_ci,
                                                      nullptr, &m_pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateGraphicsPipelines failed!", result, m_name);
    }
    m_device.set_debug_name(m_pipeline, m_name);
}

GraphicsPipeline::~GraphicsPipeline() {
    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
}

VkPipelineLayout GraphicsPipeline::pipeline_layout() const {
    return m_pipeline_layout->pipeline_layout();
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
