#include "inexor/vulkan-renderer/cubemap/cubemap_generator.hpp"

#include "inexor/vulkan-renderer/cubemap/gpu_cubemap.hpp"
#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/graphics_pipeline.hpp"
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

        const VkRenderPassCreateInfo renderpass_ci = wrapper::make_info(att_desc, subpass_desc, deps);

        wrapper::RenderPass renderpass(device, renderpass_ci, "cubemap renderpass");

        m_offscreen_framebuffer = std::make_unique<wrapper::OffscreenFramebuffer>(
            device, format, image_extent.width, image_extent.height, renderpass.renderpass(), "offscreen framebuffer");

        wrapper::CommandPool cmd_pool(device);
        wrapper::CommandBuffer cmd_buf2(device, cmd_pool.get(), "test");

        cmd_buf2.begin_command_buffer();

        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = m_offscreen_framebuffer->image();
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            cmd_buf2.pipeline_barrier(barrier);
        }

        cmd_buf2.flush_command_buffer_and_wait();

        wrapper::DescriptorBuilder builder(device);

        m_descriptor = builder.add_combined_image_sampler(skybox_gpu_cubemap).build("cubemap generator", 2);

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

        const std::vector desc_set_layouts{m_descriptor->descriptor_set_layout};

        wrapper::PipelineLayout pipeline_layout(device, wrapper::make_info(desc_set_layouts, push_constant_range),
                                                "pipeline layout");

        const auto input_assembly_sci = wrapper::make_info<VkPipelineInputAssemblyStateCreateInfo>();

        const auto rast_sci = wrapper::make_info<VkPipelineRasterizationStateCreateInfo>();

        VkPipelineColorBlendAttachmentState blend_att_state{};
        blend_att_state.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        blend_att_state.blendEnable = VK_FALSE;

        // TODO: Move this into make_info wrapper function?
        // TODO: Use C++20 std::span!
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

        // TODO: Move this into rendergraph and enable dynamic states as you call the specific methods which set them!
        const std::vector dynamic_states = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        const auto dynamic_state_ci = wrapper::make_info(dynamic_states);

        // TODO: Move this code block to ...?
        struct CubemapVertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv0;
            glm::vec2 uv1;
            glm::vec4 joint0;
            glm::vec4 weight0;
        };

        const std::vector<VkVertexInputBindingDescription> vertex_input_binding{
            {0, sizeof(CubemapVertex), VK_VERTEX_INPUT_RATE_VERTEX}};

        const std::vector<VkVertexInputAttributeDescription> vertex_input_attributes{
            {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0}};

        const VkPipelineVertexInputStateCreateInfo vertex_input_state_ci =
            wrapper::make_info(vertex_input_binding, vertex_input_attributes);

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages(2);

        // TODO: Use ShaderLoader here as well?
        wrapper::Shader filtercube(device, VK_SHADER_STAGE_VERTEX_BIT, "shaders/cubemap/filtercube.vert.spv",
                                   "filtercube");

        wrapper::Shader irradiancecube(device, VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/cubemap/irradiancecube.frag.spv",
                                       "irradiancecube");

        wrapper::Shader prefilterenvmap(device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                        "shaders/cubemap/prefilterenvmap.frag.spv", "prefilterenvmap");

        shader_stages[0] = wrapper::make_info(filtercube);

        switch (target) {
        case IRRADIANCE:
            shader_stages[1] = wrapper::make_info(irradiancecube);
            break;
        case PREFILTEREDENV:
            shader_stages[1] = wrapper::make_info(prefilterenvmap);
            break;
        };

        const auto pipeline_ci =
            wrapper::make_info(pipeline_layout.pipeline_layout(), renderpass.renderpass(), shader_stages,
                               &vertex_input_state_ci, &input_assembly_sci, &viewport_sci, &rast_sci, &multisample_sci,
                               &depth_stencil_sci, &color_blend_sci, &dynamic_state_ci);

        wrapper::GraphicsPipeline pipeline(device, pipeline_ci, "pipeline");

        VkClearValue clearValues[1];
        clearValues[0].color = {{0.0f, 0.0f, 0.2f, 0.0f}};

        auto renderpass_bi = wrapper::make_info<VkRenderPassBeginInfo>();
        renderpass_bi.renderPass = renderpass.renderpass();
        renderpass_bi.framebuffer = m_offscreen_framebuffer->framebuffer();
        renderpass_bi.renderArea.extent.width = dim;
        renderpass_bi.renderArea.extent.height = dim;
        renderpass_bi.clearValueCount = 1;
        renderpass_bi.pClearValues = clearValues;

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

        VkImageSubresourceRange subres_range{};
        subres_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres_range.baseMipLevel = 0;
        subres_range.levelCount = miplevel_count;
        subres_range.layerCount = 6;

        cmd_buf2.begin_command_buffer();

        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = m_cubemap_texture->image();
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.subresourceRange = subres_range;
            cmd_buf2.pipeline_barrier(barrier);
        }

        cmd_buf2.flush_command_buffer_and_wait();

        for (std::uint32_t mip_level = 0; mip_level < miplevel_count; mip_level++) {
            for (std::uint32_t face = 0; face < CUBE_FACE_COUNT; face++) {

                const auto mip_level_dim = static_cast<std::uint32_t>(dim * std::pow(0.5f, mip_level));

                cmd_buf2.begin_command_buffer();

                cmd_buf2.set_viewport(mip_level_dim, mip_level_dim)
                    .set_scissor(dim, dim)
                    .begin_render_pass(renderpass_bi);

                switch (target) {
                case IRRADIANCE:
                    pushBlockIrradiance.mvp =
                        glm::perspective(static_cast<float>(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[face];

                    cmd_buf2.push_constants(pushBlockIrradiance, pipeline_layout.pipeline_layout(),
                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
                    break;

                case PREFILTEREDENV:
                    pushBlockPrefilterEnv.mvp =
                        glm::perspective(static_cast<float>(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[face];
                    pushBlockPrefilterEnv.roughness =
                        static_cast<float>(mip_level) / static_cast<float>(miplevel_count - 1);

                    cmd_buf2.push_constants(pushBlockPrefilterEnv, pipeline_layout.pipeline_layout(),
                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
                    break;
                };

                cmd_buf2.bind_graphics_pipeline(pipeline.pipeline())
                    .bind_descriptor_set(m_descriptor->descriptor_set, pipeline_layout.pipeline_layout())
                    .bind_vertex_buffer(skybox.vertex_buffer());

                if (skybox.has_index_buffer()) {
                    cmd_buf2.bind_index_buffer(skybox.index_buffer());
                }

                for (auto &node : skybox.nodes()) {
                    draw_node(cmd_buf2, *node);
                }

                cmd_buf2.end_render_pass();

                {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.image = m_offscreen_framebuffer->image();
                    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                    cmd_buf2.pipeline_barrier(barrier);
                }

                m_cubemap_texture->copy_from_image(cmd_buf2, m_offscreen_framebuffer->image(), face, mip_level,
                                                   mip_level_dim, mip_level_dim);

                {
                    VkImageMemoryBarrier barrier{};
                    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    barrier.image = m_offscreen_framebuffer->image();
                    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                    cmd_buf2.pipeline_barrier(barrier);
                }

                cmd_buf2.flush_command_buffer_and_wait();
            }
        }

        cmd_buf2.begin_command_buffer();

        {
            VkImageMemoryBarrier barrier{};
            barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            barrier.image = m_cubemap_texture->image();
            barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            barrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            cmd_buf2.pipeline_barrier(barrier);
        }

        // TODO: Fix me!
        // m_cubemap_texture->change_image_layout(cmd_buf2, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, miplevel_count,
        //                                       CUBE_FACE_COUNT);

        cmd_buf2.flush_command_buffer_and_wait();

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
