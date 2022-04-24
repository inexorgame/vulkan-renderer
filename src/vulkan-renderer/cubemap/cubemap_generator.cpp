#include "inexor/vulkan-renderer/cubemap/cubemap_generator.hpp"

#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"
#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/graphics_pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/renderpass.hpp"
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

void CubemapGenerator::draw_node(const wrapper::CommandBuffer &cmd_buf, const gltf::ModelNode &node) {
    if (node.mesh) {
        for (const auto &primitive : node.mesh->primitives) {
            cmd_buf.draw_indexed(primitive.index_count, primitive.first_index);
        }
    }

    for (const auto &child : node.children) {
        draw_node(cmd_buf, *child);
    }
}

// TODO: Separate into class methods!
CubemapGenerator::CubemapGenerator(const wrapper::Device &device, const skybox::SkyboxGpuData &skybox,
                                   const cubemap::GpuCubemap &skybox_gpu_cubemap) {

    wrapper::Shader filtercube(device, VK_SHADER_STAGE_VERTEX_BIT, "shaders/cubemap/filtercube.vert.spv", "filtercube");

    wrapper::Shader irradiancecube(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/cubemap/irradiancecube.frag.spv",
                                   "irradiancecube");

    wrapper::Shader prefilterenvmap(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/cubemap/prefilterenvmap.frag.spv",
                                    "prefilterenvmap");

    // TODO: Can we make this a scoped enum?
    enum CubemapTarget { IRRADIANCE = 0, PREFILTEREDENV = 1 };

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

        const VkExtent2D image_extent{dim, dim};

        const auto miplevel_count = static_cast<std::uint32_t>(floor(log2(dim))) + 1;

        m_cubemap_texture = std::make_unique<cubemap::GpuCubemap>(device, format, dim, dim, miplevel_count, "cubemap");

        std::vector<VkAttachmentDescription> att_desc(1);
        att_desc[0].format = format;
        att_desc[0].samples = VK_SAMPLE_COUNT_1_BIT;
        att_desc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att_desc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att_desc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        att_desc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        att_desc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att_desc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference color_ref;
        color_ref.attachment = 0;
        color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        // TODO: Don't call constructor twice!
        std::vector<VkSubpassDescription> subpass_desc(1);
        subpass_desc[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass_desc[0].colorAttachmentCount = 1;
        subpass_desc[0].pColorAttachments = &color_ref;

        // TODO: Don't call constructor twice!
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

        const VkRenderPassCreateInfo renderpass_ci = wrapper::make_info(att_desc, subpass_desc, deps);

        wrapper::RenderPass renderpass(device, renderpass_ci, "cubemap renderpass");

        wrapper::OffscreenFramebuffer m_offscreen_framebuffer(device, format, image_extent.width, image_extent.height,
                                                              renderpass.renderpass(), "offscreen framebuffer");

        {
            wrapper::CommandPool cmd_pool(device);
            wrapper::CommandBuffer cmd_buf2(device, cmd_pool.get(), "test");

            cmd_buf2.begin_command_buffer()
                .change_image_layout(m_offscreen_framebuffer.image(), VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                .flush_command_buffer_and_wait("offscreen framebuffer image layout change")
                .free_command_buffer(cmd_pool.get());
        }

        // wrapper::DescriptorBuilder builder(device);
        // m_descriptor = builder.add_combined_image_sampler(skybox_gpu_cubemap).build("cubemap generator", 2);

        // Descriptors
        VkDescriptorSetLayout descriptorsetlayout;
        VkDescriptorSetLayoutBinding setLayoutBinding = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                                         VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
        descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCI.pBindings = &setLayoutBinding;
        descriptorSetLayoutCI.bindingCount = 1;

        vkCreateDescriptorSetLayout(device.device(), &descriptorSetLayoutCI, nullptr, &descriptorsetlayout);

        // Descriptor Pool
        VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
        VkDescriptorPoolCreateInfo descriptorPoolCI{};
        descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCI.poolSizeCount = 1;
        descriptorPoolCI.pPoolSizes = &poolSize;
        descriptorPoolCI.maxSets = 2;
        VkDescriptorPool descriptorpool;

        vkCreateDescriptorPool(device.device(), &descriptorPoolCI, nullptr, &descriptorpool);

        // Descriptor sets
        VkDescriptorSet descriptorset;
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = descriptorpool;
        descriptorSetAllocInfo.pSetLayouts = &descriptorsetlayout;
        descriptorSetAllocInfo.descriptorSetCount = 1;

        vkAllocateDescriptorSets(device.device(), &descriptorSetAllocInfo, &descriptorset);

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstSet = descriptorset;
        writeDescriptorSet.dstBinding = 0;
        writeDescriptorSet.pImageInfo = &skybox_gpu_cubemap.descriptor_image_info;

        vkUpdateDescriptorSets(device.device(), 1, &writeDescriptorSet, 0, nullptr);

        // TODO: Move this code block to ...?
        struct PushBlockIrradiance {
            glm::mat4 mvp;
            // TODO: Use static_cast here!
            // TODO: Use C++20 math constants here
            float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
            float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
        } pushBlockIrradiance;

        // TODO: Move this code block to ...?
        struct PushBlockPrefilterEnv {
            glm::mat4 mvp;
            float roughness;
            std::uint32_t numSamples = 32;
        } pushBlockPrefilterEnv;

        std::vector<VkPushConstantRange> push_constant_range(1);
        push_constant_range[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

        switch (target) {
        case IRRADIANCE:
            push_constant_range[0].size = sizeof(PushBlockIrradiance);
            break;
        case PREFILTEREDENV:
            push_constant_range[0].size = sizeof(PushBlockPrefilterEnv);
            break;
        };

        const std::vector desc_set_layouts{descriptorsetlayout};

        wrapper::PipelineLayout pipeline_layout(device, wrapper::make_info(desc_set_layouts, push_constant_range),
                                                "pipeline layout");

        std::vector<VkPipelineColorBlendAttachmentState> blend_att_state(1);
        blend_att_state[0].colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend_att_state[0].blendEnable = VK_FALSE;

        // TODO: Move this code block to ...?
        struct CubemapVertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv0;
            glm::vec2 uv1;
            glm::vec4 joint0;
            glm::vec4 weight0;
        };

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages(2);
        shader_stages[0] = wrapper::make_info(filtercube);

        switch (target) {
        case IRRADIANCE:
            shader_stages[1] = wrapper::make_info(irradiancecube);
            break;
        case PREFILTEREDENV:
            shader_stages[1] = wrapper::make_info(prefilterenvmap);
            break;
        };

        wrapper::GraphicsPipelineBuilder pipeline_builder(device);

        const std::vector<VkVertexInputAttributeDescription> vertex_input_attribute_descs{
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}};

        const std::vector<VkVertexInputBindingDescription> vertex_input_binding_descs{
            {0, sizeof(CubemapVertex), VK_VERTEX_INPUT_RATE_VERTEX}};

        const auto pipeline = pipeline_builder.set_color_blend_attachments(blend_att_state)
                                  .set_vertex_input_attributes(vertex_input_attribute_descs)
                                  .set_vertex_input_bindings(vertex_input_binding_descs)
                                  .set_dynamic_states({VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR})
                                  .build(pipeline_layout, renderpass, shader_stages, "test");

        std::vector<VkClearValue> clear_values(1);
        clear_values[0].color = {{0.0f, 0.0f, 0.2f, 0.0f}};

        VkRect2D render_area{};
        render_area.extent.width = dim;
        render_area.extent.height = dim;

        const auto renderpass_bi = wrapper::make_info(renderpass.renderpass(), m_offscreen_framebuffer.framebuffer(),
                                                      render_area, clear_values);

        const std::vector matrices{
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
                        glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
                        glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
            glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
        };

        const auto subres_range = wrapper::make_info(miplevel_count, 6);

        {
            wrapper::CommandPool cmd_pool(device);
            wrapper::CommandBuffer cmd_buf2(device, cmd_pool.get(), "test");

            cmd_buf2.begin_command_buffer()
                .change_image_layout(m_cubemap_texture->image(), VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subres_range)
                .flush_command_buffer_and_wait("cubemap texture image layout transition");
        }

        for (std::uint32_t mip_level = 0; mip_level < miplevel_count; mip_level++) {
            for (std::uint32_t face = 0; face < CUBE_FACE_COUNT; face++) {

                wrapper::CommandPool cmd_pool(device);
                wrapper::CommandBuffer cmd_buf2(device, cmd_pool.get(), "test");

                const auto mip_level_dim = static_cast<std::uint32_t>(dim * std::pow(0.5f, mip_level));

                cmd_buf2.begin_command_buffer()
                    .set_viewport(mip_level_dim, mip_level_dim)
                    .set_scissor(dim, dim)
                    .begin_render_pass(renderpass_bi);

                switch (target) {
                case IRRADIANCE:
                    pushBlockIrradiance.mvp =
                        glm::perspective(static_cast<float>(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[face];

                    cmd_buf2.push_constant(pushBlockIrradiance, pipeline_layout,
                                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
                    break;

                case PREFILTEREDENV:
                    pushBlockPrefilterEnv.mvp =
                        glm::perspective(static_cast<float>(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[face];
                    pushBlockPrefilterEnv.roughness =
                        static_cast<float>(mip_level) / static_cast<float>(miplevel_count - 1);

                    cmd_buf2.push_constant(pushBlockPrefilterEnv, pipeline_layout,
                                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
                    break;
                };

                cmd_buf2.bind_graphics_pipeline(pipeline)
                    .bind_descriptor_set(descriptorset, pipeline_layout)
                    .bind_vertex_buffer(skybox);

                if (skybox.has_index_buffer()) {
                    cmd_buf2.bind_index_buffer(skybox);
                }

                for (auto &node : skybox.nodes()) {
                    draw_node(cmd_buf2, *node);
                }

                VkImageCopy copy_region{};
                copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copy_region.srcSubresource.baseArrayLayer = 0;
                copy_region.srcSubresource.mipLevel = 0;
                copy_region.srcSubresource.layerCount = 1;
                copy_region.srcOffset = {0, 0, 0};
                copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copy_region.dstSubresource.baseArrayLayer = face;
                copy_region.dstSubresource.mipLevel = mip_level;
                copy_region.dstSubresource.layerCount = 1;
                copy_region.dstOffset = {0, 0, 0};
                copy_region.extent.width = static_cast<uint32_t>(mip_level_dim);
                copy_region.extent.height = static_cast<uint32_t>(mip_level_dim);
                copy_region.extent.depth = 1;

                cmd_buf2.end_render_pass()
                    .change_image_layout(m_offscreen_framebuffer.image(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                         VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
                    .copy_image(m_offscreen_framebuffer.image(), m_cubemap_texture->image(), copy_region)
                    .change_image_layout(m_offscreen_framebuffer.image(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
                    .flush_command_buffer_and_wait("copy offscreen framebuffer into cubemap");
            }
        }

        {
            wrapper::CommandPool cmd_pool(device);
            wrapper::CommandBuffer cmd_buf2(device, cmd_pool.get(), "test");

            cmd_buf2.begin_command_buffer()
                .change_image_layout(m_cubemap_texture->image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subres_range)
                .flush_command_buffer_and_wait("cubemap texture image layout transition");
        }

        switch (target) {
        case IRRADIANCE:
            m_irradiance_cube_texture = std::move(m_cubemap_texture);
            break;
        case PREFILTEREDENV:
            m_prefiltered_cube_texture = std::move(m_cubemap_texture);
            // TODO: FIX ME!
            // shaderValuesParams.prefilteredCubeMipLevels = static_cast<float>(numMips);
            break;
        };
    }
}

} // namespace inexor::vulkan_renderer::cubemap
