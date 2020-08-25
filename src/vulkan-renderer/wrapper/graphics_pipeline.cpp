#include "inexor/vulkan-renderer/wrapper/graphics_pipeline.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline &&other) noexcept
    : device(other.device), graphics_pipeline(std::exchange(other.graphics_pipeline, nullptr)),
      pipeline_cache(std::exchange(other.pipeline_cache, nullptr)), name(std::move(other.name)) {}

GraphicsPipeline::GraphicsPipeline(const VkDevice device, const VkPipelineLayout pipeline_layout,
                                   const VkRenderPass render_pass,
                                   const std::vector<VkPipelineShaderStageCreateInfo> &shader_stages,
                                   const VkVertexInputBindingDescription vertex_binding,
                                   const std::vector<VkVertexInputAttributeDescription> &attribute_binding,
                                   const std::uint32_t window_width, const std::uint32_t window_height,
                                   const std::string &name)
    : device(device), name(name) {

    assert(device);
    assert(pipeline_layout);
    assert(render_pass);
    assert(!shader_stages.empty());
    assert(!attribute_binding.empty());
    assert(window_width > 0);
    assert(window_height > 0);
    assert(!name.empty());

    VkPipelineVertexInputStateCreateInfo vertex_input_ci = {};
    vertex_input_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_ci.vertexBindingDescriptionCount = 1;
    vertex_input_ci.pVertexBindingDescriptions = &vertex_binding;
    vertex_input_ci.vertexAttributeDescriptionCount = static_cast<std::uint32_t>(attribute_binding.size());
    vertex_input_ci.pVertexAttributeDescriptions = attribute_binding.data();

    // TODO: Change topology to support wireframe.
    VkPipelineInputAssemblyStateCreateInfo input_assembly_ci = {};
    input_assembly_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly_ci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly_ci.primitiveRestartEnable = VK_FALSE;

    // TODO: Make viewport a parameter.
    VkViewport view_port = {};
    view_port.x = 0.0f;
    view_port.y = 0.0f;
    view_port.width = static_cast<float>(window_width);
    view_port.height = static_cast<float>(window_height);
    view_port.minDepth = 0.0f;
    view_port.maxDepth = 1.0f;

    // TODO: Make scissor a parameter.
    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = {window_width, window_height};

    // TODO: Examine how creating multiple viewports and scissors at once could be useful.
    VkPipelineViewportStateCreateInfo viewport_state_ci = {};
    viewport_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_ci.viewportCount = 1;
    viewport_state_ci.pViewports = &view_port;
    viewport_state_ci.scissorCount = 1;
    viewport_state_ci.pScissors = &scissor;

    VkPipelineMultisampleStateCreateInfo multisample_state_ci = {};
    multisample_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state_ci.sampleShadingEnable = VK_FALSE;
    multisample_state_ci.minSampleShading = 1.0f;
    multisample_state_ci.pSampleMask = nullptr;
    multisample_state_ci.alphaToCoverageEnable = VK_FALSE;
    multisample_state_ci.alphaToOneEnable = VK_FALSE;
    multisample_state_ci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // TODO: Support wireframe.
    VkPipelineRasterizationStateCreateInfo rasterization_state_ci = {};
    rasterization_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state_ci.depthClampEnable = VK_FALSE;
    rasterization_state_ci.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state_ci.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_state_ci.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterization_state_ci.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state_ci.depthBiasEnable = VK_FALSE;
    rasterization_state_ci.depthBiasConstantFactor = 0.0f;
    rasterization_state_ci.depthBiasClamp = 0.0f;
    rasterization_state_ci.depthBiasSlopeFactor = 0.0f;
    rasterization_state_ci.lineWidth = 1.0f;

    // TODO: Make compare function a parameter.
    VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.depthTestEnable = VK_TRUE;
    depth_stencil.depthWriteEnable = VK_TRUE;
    depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil.depthBoundsTestEnable = VK_FALSE;
    depth_stencil.stencilTestEnable = VK_FALSE;

    // TODO: Examine how this could be parameterized.
    VkPipelineColorBlendAttachmentState color_blend_attachment = {};
    color_blend_attachment.blendEnable = VK_TRUE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    color_blend_attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    // TODO: Examine how this could be parameterized.
    VkPipelineColorBlendStateCreateInfo color_blend_state_ci = {};
    color_blend_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_ci.logicOpEnable = VK_FALSE;
    color_blend_state_ci.logicOp = VK_LOGIC_OP_NO_OP;
    color_blend_state_ci.attachmentCount = 1;
    color_blend_state_ci.pAttachments = &color_blend_attachment;
    color_blend_state_ci.blendConstants[0] = 0.0f;
    color_blend_state_ci.blendConstants[1] = 0.0f;
    color_blend_state_ci.blendConstants[2] = 0.0f;
    color_blend_state_ci.blendConstants[3] = 0.0f;

    // TODO: Parameterize this.
    // Tell Vulkan that we want to change viewport and scissor during runtime so it's a dynamic state.
    const std::vector<VkDynamicState> enabled_dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamic_state_ci = {};
    dynamic_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_ci.pDynamicStates = enabled_dynamic_states.data();
    dynamic_state_ci.dynamicStateCount = static_cast<std::uint32_t>(enabled_dynamic_states.size());

    // TODO: Support tesselation stage in the future.
    // TODO: Examine in how far sub-passes should be used?
    VkGraphicsPipelineCreateInfo pipeline_ci = {};
    pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_ci.stageCount = static_cast<std::uint32_t>(shader_stages.size());
    pipeline_ci.pStages = shader_stages.data();
    pipeline_ci.pVertexInputState = &vertex_input_ci;
    pipeline_ci.pInputAssemblyState = &input_assembly_ci;
    pipeline_ci.pTessellationState = nullptr;
    pipeline_ci.pViewportState = &viewport_state_ci;
    pipeline_ci.pRasterizationState = &rasterization_state_ci;
    pipeline_ci.pMultisampleState = &multisample_state_ci;
    pipeline_ci.pDepthStencilState = &depth_stencil;
    pipeline_ci.pColorBlendState = &color_blend_state_ci;
    pipeline_ci.pDynamicState = &dynamic_state_ci;
    pipeline_ci.layout = pipeline_layout;
    pipeline_ci.renderPass = render_pass;
    pipeline_ci.subpass = 0;
    pipeline_ci.basePipelineHandle = VK_NULL_HANDLE;
    pipeline_ci.basePipelineIndex = -1;

    spdlog::debug("Creating cache for graphics pipeline.");

    VkPipelineCacheCreateInfo cache_ci = {};
    cache_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    if (vkCreatePipelineCache(device, &cache_ci, nullptr, &pipeline_cache) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreatePipelineCache failed for " + name + " !");
    }

    // TODO: Assign an internal name to this pipeline cache using Vulkan debug markers!

    spdlog::debug("Creating graphics pipeline.");

    if (vkCreateGraphicsPipelines(device, pipeline_cache, 1, &pipeline_ci, nullptr, &graphics_pipeline) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateGraphicsPipelines failed for " + name + " !");
    }

    // TODO: Assign an internal name to this graphics pipeline using Vulkan debug markers!

    spdlog::debug("Created graphics pipeline and cache successfully.");
}

GraphicsPipeline::~GraphicsPipeline() {
    spdlog::trace("Destroying pipeline cache {}.", name);
    vkDestroyPipelineCache(device, pipeline_cache, nullptr);

    spdlog::trace("Destroying pipeline {}.", name);
    vkDestroyPipeline(device, graphics_pipeline, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
