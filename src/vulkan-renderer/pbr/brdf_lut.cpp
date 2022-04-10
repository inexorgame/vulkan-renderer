#include "inexor/vulkan-renderer/pbr/brdf_lut.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/renderpass.hpp"
#include "inexor/vulkan-renderer/wrapper/shader_loader.hpp"

#include "vulkan/vulkan_core.h"

#include <spdlog/spdlog.h>

#include <vector>

namespace inexor::vulkan_renderer::pbr {

BRDFLUTGenerator::BRDFLUTGenerator(wrapper::Device &device) : m_device(device) {

    VkDescriptorSetLayout m_desc_set_layout;

    const auto format{VK_FORMAT_R16G16_SFLOAT};
    const VkExtent3D image_extent{512, 512, 1};

    spdlog::trace("Generating BRDFLUT texture of size {} x {} pixels", image_extent.width, image_extent.height);

    auto image_ci = wrapper::make_info<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.format = format;
    image_ci.extent = image_extent;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    auto image_view_ci = wrapper::make_info<VkImageViewCreateInfo>();
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = format;
    image_view_ci.subresourceRange = {};
    image_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.layerCount = 1;

    auto sampler_ci = wrapper::make_info<VkSamplerCreateInfo>();
    sampler_ci.magFilter = VK_FILTER_LINEAR;
    sampler_ci.minFilter = VK_FILTER_LINEAR;
    sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    sampler_ci.minLod = 0.0f;
    sampler_ci.maxLod = 1.0f;
    sampler_ci.maxAnisotropy = 1.0f;
    sampler_ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    m_brdf_texture = std::make_unique<texture::GpuTexture>(device, image_ci, image_view_ci, sampler_ci, "brdf lut");

    std::vector<VkAttachmentDescription> att_desc(1);
    att_desc[0].format = format;
    att_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    att_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att_desc[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkAttachmentReference color_ref = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    std::vector<VkSubpassDescription> subpass_desc(1);
    subpass_desc[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc[0].colorAttachmentCount = 1;
    subpass_desc[0].pColorAttachments = &color_ref;

    std::vector<VkSubpassDependency> deps(2);

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

    const auto renderpass_ci = wrapper::make_info(att_desc, subpass_desc, deps);

    const std::vector<VkImageView> attachments = {m_brdf_texture->image_view()};

    wrapper::RenderPass renderpass(device, renderpass_ci, "bdrf renderpass");

    m_framebuffer = std::make_unique<wrapper::Framebuffer>(device, renderpass, attachments, image_extent.width,
                                                           image_extent.height, "framebuffer");

    const auto desc_set_layout_ci = wrapper::make_info<VkDescriptorSetLayoutCreateInfo>();

    if (const auto result =
            vkCreateDescriptorSetLayout(device.device(), &desc_set_layout_ci, nullptr, &m_desc_set_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to create descriptor set layout (vkCreateDescriptorSetLayout)!", result);
    }

    const std::vector desc_set{m_desc_set_layout};

    wrapper::PipelineLayout pipeline_layout(device, wrapper::make_info(desc_set), "pipeline layout");

    const auto input_assembly_sci = wrapper::make_info<VkPipelineInputAssemblyStateCreateInfo>();
    const auto rasterization_sci = wrapper::make_info<VkPipelineRasterizationStateCreateInfo>();

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

    const std::vector dynamic_states{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    const auto dynamic_state_ci = wrapper::make_info(dynamic_states);
    const auto empty_input_sci = wrapper::make_info<VkPipelineVertexInputStateCreateInfo>();

    const std::vector<wrapper::ShaderLoaderJob> m_shader_files{
        {"shaders/brdflut/genbrdflut.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "BRDFLUT vertex shader"},
        {"shaders/brdflut/genbrdflut.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "BRDFLUT fragment shader"}};

    wrapper::ShaderLoader m_shader_loader(m_device, m_shader_files, "brdf");

    const auto pipeline_ci = wrapper::make_info(
        pipeline_layout.pipeline_layout(), renderpass.renderpass(), m_shader_loader.shader_stage_create_infos(),
        &empty_input_sci, &input_assembly_sci, &viewport_sci, &rasterization_sci, &multisample_sci, &depth_stencil_sci,
        &color_blend_sci, &dynamic_state_ci);

    wrapper::GraphicsPipeline pipeline(device, pipeline_ci, "pipeline");

    VkClearValue clear_values[1];
    clear_values[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

    auto renderpass_bi = wrapper::make_info<VkRenderPassBeginInfo>();
    renderpass_bi.renderPass = renderpass.renderpass();
    renderpass_bi.renderArea.extent.width = image_extent.width;
    renderpass_bi.renderArea.extent.height = image_extent.height;
    renderpass_bi.clearValueCount = 1;
    renderpass_bi.pClearValues = clear_values;
    renderpass_bi.framebuffer = m_framebuffer->framebuffer();

    wrapper::OnceCommandBuffer single_command(device, [&](const wrapper::CommandBuffer &cmd_buf) {
        cmd_buf.begin_render_pass(renderpass_bi)
            .set_viewport(image_extent.width, image_extent.height)
            .set_scissor(image_extent.width, image_extent.height)
            .bind_graphics_pipeline(pipeline)
            .draw(3, 0, 1, 0)
            .end_render_pass();

        m_brdf_texture->change_image_layout(cmd_buf, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    });

    spdlog::trace("Generating BRDF look-up table finished.");

    // TODO: Put into RAII wrappers!
    vkDestroyDescriptorSetLayout(m_device.device(), m_desc_set_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::pbr