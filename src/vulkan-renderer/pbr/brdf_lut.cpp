#include "inexor/vulkan-renderer/pbr/brdf_lut.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "vulkan/vulkan_core.h"

#include <spdlog/spdlog.h>

#include <vector>

namespace inexor::vulkan_renderer::pbr {

BRDFLUTGenerator::BRDFLUTGenerator(const wrapper::Device &device) : m_device(device) {
    spdlog::trace("BRDF LUT generation started");

    const auto format = VK_FORMAT_R16G16_SFLOAT;
    const VkExtent2D image_extent{512, 512};

    spdlog::trace("Generating BRDFLUT texture of size {} x {} pixels", image_extent.width, image_extent.height);

    m_brdf_lut_image = std::make_unique<wrapper::Image>(
        device, format, image_extent.width, image_extent.height,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, "texture");

    VkAttachmentDescription att_desc{};
    att_desc.format = format;
    att_desc.samples = VK_SAMPLE_COUNT_1_BIT;
    att_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att_desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpass_desc{};
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &color_ref;

    std::array<VkSubpassDependency, 2> deps;

    deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    deps[0].dstSubpass = 0;
    deps[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    deps[1].srcSubpass = 0;
    deps[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    deps[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    deps[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    deps[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    deps[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    deps[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    auto renderpass_ci = wrapper::make_info<VkRenderPassCreateInfo>();
    renderpass_ci.attachmentCount = 1;
    renderpass_ci.pAttachments = &att_desc;
    renderpass_ci.subpassCount = 1;
    renderpass_ci.pSubpasses = &subpass_desc;
    renderpass_ci.dependencyCount = static_cast<std::uint32_t>(deps.size());
    renderpass_ci.pDependencies = deps.data();

    if (const auto result = vkCreateRenderPass(device.device(), &renderpass_ci, nullptr, &m_renderpass);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create renderpass (vkCreateRenderPass)!", result);
    }

    const std::vector<VkImageView> attachments = {m_brdf_lut_image->image_view()};

    m_framebuffer = std::make_unique<wrapper::Framebuffer>(device, m_renderpass, attachments, image_extent.width,
                                                           image_extent.height, "framebuffer");

    VkDescriptorSetLayoutCreateInfo desc_set_layout_ci = wrapper::make_info<VkDescriptorSetLayoutCreateInfo>();

    if (const auto result =
            vkCreateDescriptorSetLayout(device.device(), &desc_set_layout_ci, nullptr, &m_desc_set_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create descriptor set layout (vkCreateDescriptorSetLayout)!", result);
    }

    auto pipeline_layout_ci = wrapper::make_info<VkPipelineLayoutCreateInfo>();
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &m_desc_set_layout;

    if (const auto result = vkCreatePipelineLayout(device.device(), &pipeline_layout_ci, nullptr, &m_pipeline_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create pipeline layout (vkCreatePipelineLayout)!", result);
        return;
    }

    auto input_assembly_sci = wrapper::make_info<VkPipelineInputAssemblyStateCreateInfo>();
    input_assembly_sci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    auto rasterization_sci = wrapper::make_info<VkPipelineRasterizationStateCreateInfo>();
    rasterization_sci.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization_sci.cullMode = VK_CULL_MODE_NONE;
    rasterization_sci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_sci.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blend_att_state{};
    blend_att_state.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blend_att_state.blendEnable = VK_FALSE;

    auto color_blend_sci = wrapper::make_info<VkPipelineColorBlendStateCreateInfo>();
    color_blend_sci.attachmentCount = 1;
    color_blend_sci.pAttachments = &blend_att_state;

    auto depth_stencil_sci = wrapper::make_info<VkPipelineDepthStencilStateCreateInfo>();
    depth_stencil_sci.depthTestEnable = VK_FALSE;
    depth_stencil_sci.depthWriteEnable = VK_FALSE;
    depth_stencil_sci.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil_sci.front = depth_stencil_sci.back;
    depth_stencil_sci.back.compareOp = VK_COMPARE_OP_ALWAYS;

    auto viewport_sci = wrapper::make_info<VkPipelineViewportStateCreateInfo>();
    viewport_sci.viewportCount = 1;
    viewport_sci.scissorCount = 1;

    auto multisample_sci = wrapper::make_info<VkPipelineMultisampleStateCreateInfo>();
    multisample_sci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDynamicState> dynamic_state_enables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    auto dynamic_state_ci = wrapper::make_info<VkPipelineDynamicStateCreateInfo>();
    dynamic_state_ci.pDynamicStates = dynamic_state_enables.data();
    dynamic_state_ci.dynamicStateCount = static_cast<uint32_t>(dynamic_state_enables.size());

    auto empty_input_sci = wrapper::make_info<VkPipelineVertexInputStateCreateInfo>();

    wrapper::Shader lut_generator_vertex(device, VK_SHADER_STAGE_VERTEX_BIT, "brdf_lut_vertex",
                                         "shaders/brdflut/genbrdflut.vert.spv");

    wrapper::Shader lut_generator_fragment(device, VK_SHADER_STAGE_FRAGMENT_BIT, "brdf_lut_fragment",
                                           "shaders/brdflut/genbrdflut.frag.spv");

    std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;

    shader_stages[0] = wrapper::make_info<VkPipelineShaderStageCreateInfo>();
    shader_stages[0].module = lut_generator_vertex.module();
    shader_stages[0].stage = lut_generator_vertex.type();
    shader_stages[0].pName = lut_generator_vertex.entry_point().c_str();

    shader_stages[1] = wrapper::make_info<VkPipelineShaderStageCreateInfo>();
    shader_stages[1].module = lut_generator_fragment.module();
    shader_stages[1].stage = lut_generator_fragment.type();
    shader_stages[1].pName = lut_generator_fragment.entry_point().c_str();

    auto pipeline_ci = wrapper::make_info<VkGraphicsPipelineCreateInfo>();
    pipeline_ci.layout = m_pipeline_layout;
    pipeline_ci.renderPass = m_renderpass;
    pipeline_ci.pInputAssemblyState = &input_assembly_sci;
    pipeline_ci.pVertexInputState = &empty_input_sci;
    pipeline_ci.pRasterizationState = &rasterization_sci;
    pipeline_ci.pColorBlendState = &color_blend_sci;
    pipeline_ci.pMultisampleState = &multisample_sci;
    pipeline_ci.pViewportState = &viewport_sci;
    pipeline_ci.pDepthStencilState = &depth_stencil_sci;
    pipeline_ci.pDynamicState = &dynamic_state_ci;
    pipeline_ci.stageCount = static_cast<std::uint32_t>(shader_stages.size());
    pipeline_ci.pStages = shader_stages.data();

    if (const auto result = vkCreateGraphicsPipelines(device.device(), nullptr, 1, &pipeline_ci, nullptr, &m_pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create pipeline layout (vkCreatePipelineLayout)!", result);
    }

    VkClearValue clear_values[1];
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    auto renderpass_bi = wrapper::make_info<VkRenderPassBeginInfo>();
    renderpass_bi.renderPass = m_renderpass;
    renderpass_bi.renderArea.extent = image_extent;
    renderpass_bi.clearValueCount = 1;
    renderpass_bi.pClearValues = clear_values;
    renderpass_bi.framebuffer = m_framebuffer->framebuffer();

    wrapper::OnceCommandBuffer cmd_buf(device);

    cmd_buf.create_command_buffer();
    cmd_buf.start_recording();

    vkCmdBeginRenderPass(cmd_buf.command_buffer(), &renderpass_bi, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.width = static_cast<float>(image_extent.width);
    viewport.height = static_cast<float>(image_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.extent.width = image_extent.width;
    scissor.extent.height = image_extent.height;

    vkCmdSetViewport(cmd_buf.command_buffer(), 0, 1, &viewport);
    vkCmdSetScissor(cmd_buf.command_buffer(), 0, 1, &scissor);
    vkCmdBindPipeline(cmd_buf.command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdDraw(cmd_buf.command_buffer(), 3, 1, 0, 0);
    vkCmdEndRenderPass(cmd_buf.command_buffer());

    cmd_buf.end_recording_and_submit_command();

    spdlog::trace("Generating BRDF look-up table finished.");
}

BRDFLUTGenerator::~BRDFLUTGenerator() {
    vkDestroyPipelineLayout(m_device.device(), m_pipeline_layout, nullptr);
    vkDestroyRenderPass(m_device.device(), m_renderpass, nullptr);
    vkDestroyPipeline(m_device.device(), m_pipeline, nullptr);
    vkDestroyDescriptorSetLayout(m_device.device(), m_desc_set_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::pbr
