#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"

namespace inexor::vulkan_renderer::wrapper::pipelines {

GraphicsPipelineBuilder::GraphicsPipelineBuilder(const core::Device &device) : m_device(device) {
    reset();
}

std::shared_ptr<GraphicsPipeline> GraphicsPipelineBuilder::build(std::string name) {
    if (name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
    // NOTE: Inside of GraphicsPipelineBuilder, we do not carry out error checks when it comes to the data which is used
    // to build the graphics pipeline. This is because validation of this data is job of the validation layers, and not
    // the job of GraphicsPipelineBuilder. We should not mimic the behavious of validation layers here.

    auto graphics_pipeline = std::make_shared<GraphicsPipeline>(m_device, std::move(m_data), std::move(name));

    // NOTE: We reset the data of the builder here so it can be re-used
    reset();

    // Return the graphics pipeline we created
    return graphics_pipeline;
}

GraphicsPipelineBuilder &GraphicsPipelineBuilder::add_standard_alpha_blend_attachment() {
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

GraphicsPipelineBuilder &GraphicsPipelineBuilder::set_standard_depth_stencil() {
    return set_depth_stencil(make_info<VkPipelineDepthStencilStateCreateInfo>({
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    }));
}

void GraphicsPipelineBuilder::reset() {
    m_data = {};
    // NOTE: This is the implicit default value
    m_data.rasterization_sci = make_info<VkPipelineRasterizationStateCreateInfo>({
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .lineWidth = 1.0f,
    });
    // NOTE: This is the implicit default value
    m_data.input_assembly_sci = make_info<VkPipelineInputAssemblyStateCreateInfo>({
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    });
    // NOTE: This is the implicit default value
    m_data.multisample_sci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    // NOTE: This is the implicit default value
    m_data.multisample_sci.minSampleShading = 1.0f;
    // NOTE: This is the implicit default value
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
