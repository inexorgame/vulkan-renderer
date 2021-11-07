#include "inexor/vulkan-renderer/cubemap/cubemap_generator.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <spdlog/spdlog.h>

#define _USE_MATH_DEFINES
#include <cmath>

// TODO: This must be removed before code review!!
// I am not saying C++20 would resolve this issue but C++20 would resolve this issue
#ifndef M_PI
#define M_PI 3.1415926535897932
#endif

#include <vector>

namespace inexor::vulkan_renderer::cubemap {

CubemapGenerator::CubemapGenerator(const wrapper::Device &device, const CubemapCpuTexture &texture) {

    enum Target { IRRADIANCE = 0, PREFILTEREDENV = 1 };

    for (std::uint32_t target = 0; target < PREFILTEREDENV + 1; target++) {

        VkFormat format;
        std::uint32_t dim;

        switch (target) {
        case IRRADIANCE:
            format = VK_FORMAT_R32G32B32A32_SFLOAT;
            dim = 64;
            break;
        case PREFILTEREDENV:
            format = VK_FORMAT_R16G16B16A16_SFLOAT;
            dim = 512;
            break;
        };

        // TODO: Who makes sure the texture has this size actually?
        const VkExtent2D image_extent{dim, dim};

        const std::uint32_t miplevel_count = static_cast<std::uint32_t>(floor(log2(dim))) + 1;

        constexpr std::uint32_t array_layer_count = 6;

        // Each cube has 6 faces
        constexpr std::uint32_t cube_face_count = 6;

        // TODO: Create cubemap texture here!

        VkAttachmentDescription att_desc{};
        att_desc.format = format;
        att_desc.samples = VK_SAMPLE_COUNT_1_BIT;
        att_desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att_desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att_desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att_desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att_desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att_desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_ref;
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

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

        VkRenderPass renderpass;

        if (const auto result = vkCreateRenderPass(device.device(), &renderpass_ci, nullptr, &renderpass);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to create renderpass for cubemap generation (vkCreateRenderPass)!", result);
        }

        // TODO: Is this correct?
        m_offscreen_framebuffer = std::make_unique<wrapper::OffscreenFramebuffer>(
            device, format, image_extent.width, image_extent.height, renderpass, "offscreen framebuffer");

        VkImageSubresourceRange subres_range{};
        subres_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres_range.baseMipLevel = 0;
        subres_range.levelCount = 1;
        subres_range.baseArrayLayer = 0;
        subres_range.layerCount = 1;

        // TODO: Should the pipeline barrier be embedded into the framebuffer?
        m_offscreen_framebuffer->m_image->place_pipeline_barrier(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0,
                                                                 VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, subres_range);

        const std::vector<VkDescriptorPoolSize> pool_sizes{{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}};

        m_descriptor_pool = std::make_unique<wrapper::DescriptorPool>(device, pool_sizes, "gltf");

        wrapper::DescriptorBuilder builder(device, m_descriptor_pool->descriptor_pool());

        m_descriptor = builder.add_combined_image_sampler(*m_cubemap_texture).build("octree");

        struct PushBlockIrradiance {
            glm::mat4 mvp;
            float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
            float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
        } pushBlockIrradiance;

        struct PushBlockPrefilterEnv {
            glm::mat4 mvp;
            float roughness;
            uint32_t numSamples = 32u;
        } pushBlockPrefilterEnv;

        VkPipelineLayout pipelinelayout;

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        switch (target) {
        case IRRADIANCE:
            pushConstantRange.size = sizeof(PushBlockIrradiance);
            break;
        case PREFILTEREDENV:
            pushConstantRange.size = sizeof(PushBlockPrefilterEnv);
            break;
        };

        auto pipeline_layout_ci = wrapper::make_info<VkPipelineLayoutCreateInfo>();
        pipeline_layout_ci.setLayoutCount = 1;
        pipeline_layout_ci.pSetLayouts = &m_descriptor->descriptor_set_layout();
        pipeline_layout_ci.pushConstantRangeCount = 1;
        pipeline_layout_ci.pPushConstantRanges = &pushConstantRange;

        if (const auto result = vkCreatePipelineLayout(device.device(), &pipeline_layout_ci, nullptr, &pipelinelayout);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to create pipeline layout (vkCreatePipelineLayout)!", result);
        }

        auto input_assembly_sci = wrapper::make_info<VkPipelineInputAssemblyStateCreateInfo>();
        input_assembly_sci.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        auto pipeline_rast_sci = wrapper::make_info<VkPipelineRasterizationStateCreateInfo>();
        pipeline_rast_sci.polygonMode = VK_POLYGON_MODE_FILL;
        pipeline_rast_sci.cullMode = VK_CULL_MODE_NONE;
        pipeline_rast_sci.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        pipeline_rast_sci.lineWidth = 1.0f;

        VkPipelineColorBlendAttachmentState blend_att_state{};
        blend_att_state.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend_att_state.blendEnable = VK_FALSE;

        // TODO: Move this into make_info wrapper function?
        auto color_blend_sci = wrapper::make_info<VkPipelineColorBlendStateCreateInfo>();
        color_blend_sci.attachmentCount = 1;
        color_blend_sci.pAttachments = &blend_att_state;

        auto depth_stencil_sci = wrapper::make_info<VkPipelineDepthStencilStateCreateInfo>();
        depth_stencil_sci.depthTestEnable = VK_FALSE;
        depth_stencil_sci.depthWriteEnable = VK_FALSE;
        depth_stencil_sci.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depth_stencil_sci.front = depth_stencil_sci.back;
        depth_stencil_sci.back.compareOp = VK_COMPARE_OP_ALWAYS;

        // TODO: Turn all this into a builder pattern?
        auto viewport_sci = wrapper::make_info<VkPipelineViewportStateCreateInfo>();
        viewport_sci.viewportCount = 1;
        viewport_sci.scissorCount = 1;

        auto multisample_sci = wrapper::make_info<VkPipelineMultisampleStateCreateInfo>();
        multisample_sci.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        auto dynamic_state_ci = wrapper::make_info<VkPipelineDynamicStateCreateInfo>();
        dynamic_state_ci.pDynamicStates = dynamicStateEnables.data();
        dynamic_state_ci.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

        // TODO: Use vertex structure from other code part
        struct CubemapVertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv0;
            glm::vec2 uv1;
            glm::vec4 joint0;
            glm::vec4 weight0;
        };

        VkVertexInputBindingDescription vertexInputBinding = {0, sizeof(CubemapVertex), VK_VERTEX_INPUT_RATE_VERTEX};

        VkVertexInputAttributeDescription vertexInputAttribute = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};

        // Move into make_info? builder pattern?
        auto vertexInputStateCI = wrapper::make_info<VkPipelineVertexInputStateCreateInfo>();
        vertexInputStateCI.vertexBindingDescriptionCount = 1;
        vertexInputStateCI.pVertexBindingDescriptions = &vertexInputBinding;
        vertexInputStateCI.vertexAttributeDescriptionCount = 1;
        vertexInputStateCI.pVertexAttributeDescriptions = &vertexInputAttribute;

        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;

        // TODO: Use ShaderLoader here as well?
        wrapper::Shader filtercube(device, VK_SHADER_STAGE_VERTEX_BIT, "shaders/cubemap/filtercube.vert.spv",
                                   "filtercube");

        wrapper::Shader irradiancecube(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/cubemap/irradiancecube.frag.spv",
                                       "irradiancecube");

        wrapper::Shader prefilterenvmap(device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                        "shaders/cubemap/prefilterenvmap.frag.spv", "prefilterenvmap");

        shader_stages[0] = wrapper::make_info<VkPipelineShaderStageCreateInfo>();
        shader_stages[0].module = filtercube.module();
        shader_stages[0].stage = filtercube.type();
        shader_stages[0].pName = filtercube.entry_point().c_str();

        shader_stages[1] = wrapper::make_info<VkPipelineShaderStageCreateInfo>();
        // Both irradiancecube and prefilterenvmap are fragment shaders!
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;

        switch (target) {
        case IRRADIANCE:
            shader_stages[1].module = irradiancecube.module();
            shader_stages[1].pName = irradiancecube.entry_point().c_str();
            break;
        case PREFILTEREDENV:
            shader_stages[1].module = prefilterenvmap.module();
            shader_stages[1].pName = prefilterenvmap.entry_point().c_str();
            break;
        };

        auto pipeline_ci = wrapper::make_info<VkGraphicsPipelineCreateInfo>();
        pipeline_ci.layout = pipelinelayout;
        pipeline_ci.renderPass = renderpass;
        pipeline_ci.pInputAssemblyState = &input_assembly_sci;
        pipeline_ci.pVertexInputState = &vertexInputStateCI;
        pipeline_ci.pRasterizationState = &pipeline_rast_sci;
        pipeline_ci.pColorBlendState = &color_blend_sci;
        pipeline_ci.pMultisampleState = &multisample_sci;
        pipeline_ci.pViewportState = &viewport_sci;
        pipeline_ci.pDepthStencilState = &depth_stencil_sci;
        pipeline_ci.pDynamicState = &dynamic_state_ci;
        pipeline_ci.stageCount = static_cast<std::uint32_t>(shader_stages.size());
        pipeline_ci.pStages = shader_stages.data();

        // TODO: Create builder patter? no?
        VkPipeline pipeline;
        if (const auto result =
                vkCreateGraphicsPipelines(device.device(), nullptr, 1, &pipeline_ci, nullptr, &pipeline);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to create graphics pipeline (vkCreateGraphicsPipelines)!", result);
        }

        // We could destroy the shader modules here already btw..

        VkClearValue clearValues[1];
        clearValues[0].color = {{0.0f, 0.0f, 0.2f, 0.0f}};

        // builder pattern?
        auto renderpass_bi = wrapper::make_info<VkRenderPassBeginInfo>();
        renderpass_bi.renderPass = renderpass;
        renderpass_bi.framebuffer = m_offscreen_framebuffer->framebuffer();
        renderpass_bi.renderArea.extent.width = dim;
        renderpass_bi.renderArea.extent.height = dim;
        renderpass_bi.clearValueCount = 1;
        renderpass_bi.pClearValues = clearValues;

        const std::vector<glm::mat4> matrices = {
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
                        glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
                        glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };

        VkViewport viewport{};
        viewport.width = static_cast<float>(dim);
        viewport.height = static_cast<float>(dim);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        VkRect2D scissor{};
        scissor.extent.width = dim;
        scissor.extent.height = dim;

        subres_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres_range.baseMipLevel = 0;
        subres_range.levelCount = miplevel_count;
        subres_range.layerCount = cube_face_count;

        m_cubemap_texture->image_wrapper()->place_pipeline_barrier(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0,
                                                                   VK_ACCESS_TRANSFER_WRITE_BIT, subres_range);

        // TODO: Implement graphics pipeline builder
        // TODO: Split up in setup of pipeline and rendering of cubemaps
        for (std::uint32_t mip_level = 0; mip_level < miplevel_count; mip_level++) {
            for (std::uint32_t face = 0; face < cube_face_count; face++) {

                wrapper::OnceCommandBuffer cmd_buf(device);

                cmd_buf.create_command_buffer();
                cmd_buf.start_recording();

                viewport.width = static_cast<float>(dim * std::pow(0.5f, mip_level));
                viewport.height = static_cast<float>(dim * std::pow(0.5f, mip_level));

                vkCmdSetViewport(cmd_buf.command_buffer(), 0, 1, &viewport);
                vkCmdSetScissor(cmd_buf.command_buffer(), 0, 1, &scissor);
                vkCmdBeginRenderPass(cmd_buf.command_buffer(), &renderpass_bi, VK_SUBPASS_CONTENTS_INLINE);

                switch (target) {
                case IRRADIANCE:
                    pushBlockIrradiance.mvp =
                        glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[face];

                    vkCmdPushConstants(cmd_buf.command_buffer(), pipelinelayout,
                                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                       sizeof(PushBlockIrradiance), &pushBlockIrradiance);
                    break;
                case PREFILTEREDENV:
                    pushBlockPrefilterEnv.mvp =
                        glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[face];
                    pushBlockPrefilterEnv.roughness = (float)mip_level / (float)(miplevel_count - 1);

                    vkCmdPushConstants(cmd_buf.command_buffer(), pipelinelayout,
                                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                       sizeof(PushBlockPrefilterEnv), &pushBlockPrefilterEnv);
                    break;
                };

                vkCmdBindPipeline(cmd_buf.command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

                vkCmdBindDescriptorSets(cmd_buf.command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1,
                                        &m_descriptor->descriptor_set(), 0, nullptr);

                VkDeviceSize offsets[1] = {0};

                // TODO: draw skybox!

                vkCmdEndRenderPass(cmd_buf.command_buffer());

                // TODO: Move this into wrapper::Image !
                {
                    VkImageMemoryBarrier imageMemoryBarrier{};
                    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageMemoryBarrier.image = m_offscreen_framebuffer->image();
                    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

                    vkCmdPipelineBarrier(cmd_buf.command_buffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                         &imageMemoryBarrier);
                }

                VkImageCopy copyRegion{};
                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = {0, 0, 0};
                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = face;
                copyRegion.dstSubresource.mipLevel = mip_level;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = {0, 0, 0};
                copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
                copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
                copyRegion.extent.depth = 1;

                vkCmdCopyImage(cmd_buf.command_buffer(), m_offscreen_framebuffer->image(),
                               VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_cubemap_texture->image(),
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

                {
                    VkImageMemoryBarrier imageMemoryBarrier{};
                    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageMemoryBarrier.image = m_offscreen_framebuffer->image();
                    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

                    vkCmdPipelineBarrier(cmd_buf.command_buffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                         &imageMemoryBarrier);
                }

                cmd_buf.end_recording_and_submit_command();
            }
        }

        m_cubemap_texture->image_wrapper()->place_pipeline_barrier(
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT,
            VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT, subres_range);

        vkDestroyRenderPass(device.device(), renderpass, nullptr);
        vkDestroyPipeline(device.device(), pipeline, nullptr);
        vkDestroyPipelineLayout(device.device(), pipelinelayout, nullptr);

        spdlog::trace("Cubemap generation finished");
    }
}

} // namespace inexor::vulkan_renderer::cubemap
