#include "inexor/vulkan-renderer/pbr/brdf_lut.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <spdlog/spdlog.h>

#include <vector>

namespace inexor::vulkan_renderer::pbr {

BrdfLutGenerator::BrdfLutGenerator(const wrapper::Device &device) {
    spdlog::trace("BRDF LUT generation started");

    const auto format = VK_FORMAT_R16G16_SFLOAT;
    const VkExtent2D image_extent{512, 512};

    m_brdf_lut_image = std::make_unique<wrapper::Image>(
        device, format, image_extent.width, image_extent.height,
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, "texture");

    // FB, Att, RP, Pipe, etc.
    VkAttachmentDescription att_desc{};

    // Color attachment
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

    // Use subpass dependencies for layout transitions
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

    // Create the actual renderpass
    VkRenderPassCreateInfo renderpass_ci{};
    renderpass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_ci.attachmentCount = 1;
    renderpass_ci.pAttachments = &att_desc;
    renderpass_ci.subpassCount = 1;
    renderpass_ci.pSubpasses = &subpass_desc;
    renderpass_ci.dependencyCount = 2;
    renderpass_ci.pDependencies = deps.data();

    VkRenderPass renderpass;

    if (const auto result = vkCreateRenderPass(device.device(), &renderpass_ci, nullptr, &renderpass);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create renderpass (vkCreateRenderPass)!", result);
    }

    const std::vector<VkImageView> attachments = {m_brdf_lut_image->image_view()};

    m_framebuffer = std::make_unique<wrapper::Framebuffer>(device, renderpass, attachments, image_extent.width,
                                                           image_extent.height, "framebuffer");

    // Desriptors
    VkDescriptorSetLayout descriptorsetlayout;
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
    descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;

    if (const auto result =
            vkCreateDescriptorSetLayout(device.device(), &descriptorSetLayoutCI, nullptr, &descriptorsetlayout);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create descriptor set layout (vkCreateDescriptorSetLayout)!", result);
    }

    // Pipeline layout
    VkPipelineLayout pipelinelayout;
    VkPipelineLayoutCreateInfo pipelineLayoutCI{};
    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.setLayoutCount = 1;
    pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;

    if (const auto result = vkCreatePipelineLayout(device.device(), &pipelineLayoutCI, nullptr, &pipelinelayout);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create pipeline layout (vkCreatePipelineLayout)!", result);
        return;
    }

    // Pipeline
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
    inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
    rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
    rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCI.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState blendAttachmentState{};
    blendAttachmentState.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    blendAttachmentState.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
    colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendStateCI.attachmentCount = 1;
    colorBlendStateCI.pAttachments = &blendAttachmentState;

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
    depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilStateCI.depthTestEnable = VK_FALSE;
    depthStencilStateCI.depthWriteEnable = VK_FALSE;
    depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    depthStencilStateCI.front = depthStencilStateCI.back;
    depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

    VkPipelineViewportStateCreateInfo viewportStateCI{};
    viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateCI.viewportCount = 1;
    viewportStateCI.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
    multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateCI{};
    dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
    dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

    VkPipelineVertexInputStateCreateInfo emptyInputStateCI{};
    emptyInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

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

    VkGraphicsPipelineCreateInfo pipeline_ci{};
    pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_ci.layout = pipelinelayout;
    pipeline_ci.renderPass = renderpass;
    pipeline_ci.pInputAssemblyState = &inputAssemblyStateCI;
    pipeline_ci.pVertexInputState = &emptyInputStateCI;
    pipeline_ci.pRasterizationState = &rasterizationStateCI;
    pipeline_ci.pColorBlendState = &colorBlendStateCI;
    pipeline_ci.pMultisampleState = &multisampleStateCI;
    pipeline_ci.pViewportState = &viewportStateCI;
    pipeline_ci.pDepthStencilState = &depthStencilStateCI;
    pipeline_ci.pDynamicState = &dynamicStateCI;
    pipeline_ci.stageCount = 2;
    pipeline_ci.pStages = shader_stages.data();

    VkPipeline pipeline;

    if (const auto result = vkCreateGraphicsPipelines(device.device(), nullptr, 1, &pipeline_ci, nullptr, &pipeline);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create pipeline layout (vkCreatePipelineLayout)!", result);
    }

    // Render
    VkClearValue clear_values[1];
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderPassBeginInfo renderpass_bi{};
    renderpass_bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_bi.renderPass = renderpass;
    renderpass_bi.renderArea.extent = image_extent;
    renderpass_bi.clearValueCount = 1;
    renderpass_bi.pClearValues = clear_values;
    renderpass_bi.framebuffer = m_framebuffer->framebuffer();

    wrapper::OnceCommandBuffer cmd_buf(device, device.graphics_queue(), device.graphics_queue_family_index());

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
    vkCmdBindPipeline(cmd_buf.command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    vkCmdDraw(cmd_buf.command_buffer(), 3, 1, 0, 0);
    vkCmdEndRenderPass(cmd_buf.command_buffer());

    cmd_buf.end_recording_and_submit_command();

    spdlog::trace("Generating BRDF look-up table finished.");
}

} // namespace inexor::vulkan_renderer::pbr
