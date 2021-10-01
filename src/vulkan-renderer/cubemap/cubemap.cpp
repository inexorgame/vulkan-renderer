#include "inexor/vulkan-renderer/cubemap/cubemap.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/offscreen_framebuffer.hpp"
#include "inexor/vulkan-renderer/wrapper/once_command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <spdlog/spdlog.h>

#include <math.h>
#include <vector>

namespace inexor::vulkan_renderer::cubemap {

Cubemap::Cubemap(const wrapper::Device &device) {

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

        const VkExtent2D image_extent{dim, dim};

        const std::uint32_t mipmap_count = static_cast<std::uint32_t>(floor(log2(dim))) + 1;

        m_cubemap_image =
            std::make_unique<wrapper::Image>(device, VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT, VK_IMAGE_TYPE_2D, format,
                                             image_extent.width, image_extent.height, 1, 6, VK_SAMPLE_COUNT_1_BIT,
                                             VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                             VK_IMAGE_VIEW_TYPE_CUBE, VK_IMAGE_ASPECT_COLOR_BIT, "cubemap");

        // Sampler
        VkSamplerCreateInfo sampler_ci{};
        sampler_ci.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_ci.magFilter = VK_FILTER_LINEAR;
        sampler_ci.minFilter = VK_FILTER_LINEAR;
        sampler_ci.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_ci.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_ci.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_ci.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_ci.minLod = 0.0f;
        sampler_ci.maxLod = static_cast<float>(mipmap_count);
        sampler_ci.maxAnisotropy = 1.0f;
        sampler_ci.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        if (const auto result = vkCreateSampler(device.device(), &sampler_ci, nullptr, &m_sampler);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to create cubemap image sampler (vkCreateSampler)!", result);
        }

        // FB, Att, RP, Pipe, etc.
        VkAttachmentDescription attDesc{};

        // Color attachment
        attDesc.format = format;
        attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorReference;

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;
        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        // Renderpass
        VkRenderPassCreateInfo renderPassCI{};
        renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCI.attachmentCount = 1;
        renderPassCI.pAttachments = &attDesc;
        renderPassCI.subpassCount = 1;
        renderPassCI.pSubpasses = &subpassDescription;
        renderPassCI.dependencyCount = 2;
        renderPassCI.pDependencies = dependencies.data();
        VkRenderPass renderpass;

        if (const auto result = vkCreateRenderPass(device.device(), &renderPassCI, nullptr, &renderpass);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to create renderpass for cubemap generation (vkCreateRenderPass)!", result);
        }

        struct Offscreen {
            VkImage image;
            VkImageView view;
            VkDeviceMemory memory;
            VkFramebuffer framebuffer;
        } offscreen;

        // Create offscreen framebuffer
        {
            m_offscreen_framebuffer = std::unique_ptr<wrapper::OffscreenFramebuffer>();

            wrapper::OnceCommandBuffer cmd_buf(device, device.graphics_queue(), device.graphics_queue_family_index());

            cmd_buf.create_command_buffer();
            cmd_buf.start_recording();

            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.image = offscreen.image;
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

            vkCmdPipelineBarrier(cmd_buf.command_buffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            cmd_buf.end_recording_and_submit_command();
        }

        // Descriptors
        VkDescriptorSetLayout descriptorsetlayout;
        VkDescriptorSetLayoutBinding setLayoutBinding = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,
                                                         VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
        descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCI.pBindings = &setLayoutBinding;
        descriptorSetLayoutCI.bindingCount = 1;

        if (const auto result =
                vkCreateDescriptorSetLayout(device.device(), &descriptorSetLayoutCI, nullptr, &descriptorsetlayout);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to create descriptor set layout (vkCreateDescriptorSetLayout)!", result);
        }

        // Descriptor Pool
        VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
        VkDescriptorPoolCreateInfo descriptorPoolCI{};
        descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCI.poolSizeCount = 1;
        descriptorPoolCI.pPoolSizes = &poolSize;
        descriptorPoolCI.maxSets = 2;
        VkDescriptorPool descriptorpool;

        if (const auto result = vkCreateDescriptorPool(device.device(), &descriptorPoolCI, nullptr, &descriptorpool);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to create descriptor pool (vkCreateDescriptorPool)!", result);
        }

        // Descriptor sets
        VkDescriptorSet descriptorset;
        VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
        descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptorSetAllocInfo.descriptorPool = descriptorpool;
        descriptorSetAllocInfo.pSetLayouts = &descriptorsetlayout;
        descriptorSetAllocInfo.descriptorSetCount = 1;

        if (const auto result = vkAllocateDescriptorSets(device.device(), &descriptorSetAllocInfo, &descriptorset);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to allocate descriptor sets (vkAllocateDescriptorSets)!", result);
        }

        VkWriteDescriptorSet writeDescriptorSet{};
        writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writeDescriptorSet.descriptorCount = 1;
        writeDescriptorSet.dstSet = descriptorset;
        writeDescriptorSet.dstBinding = 0;
        // writeDescriptorSet.pImageInfo = ?;

        vkUpdateDescriptorSets(device.device(), 1, &writeDescriptorSet, 0, nullptr);

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

        // Pipeline layout
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

        VkPipelineLayoutCreateInfo pipelineLayoutCI{};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
        pipelineLayoutCI.pushConstantRangeCount = 1;
        pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;

        if (const auto result = vkCreatePipelineLayout(device.device(), &pipelineLayoutCI, nullptr, &pipelinelayout);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to create pipeline layout (vkCreatePipelineLayout)!", result);
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

        // TODO: Use vertex structure from other code part
        struct CubemapVertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec2 uv0;
            glm::vec2 uv1;
            glm::vec4 joint0;
            glm::vec4 weight0;
        };

        // Vertex input state
        VkVertexInputBindingDescription vertexInputBinding = {0, sizeof(CubemapVertex), VK_VERTEX_INPUT_RATE_VERTEX};

        VkVertexInputAttributeDescription vertexInputAttribute = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};

        VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
        vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputStateCI.vertexBindingDescriptionCount = 1;
        vertexInputStateCI.pVertexBindingDescriptions = &vertexInputBinding;
        vertexInputStateCI.vertexAttributeDescriptionCount = 1;
        vertexInputStateCI.pVertexAttributeDescriptions = &vertexInputAttribute;

        std::array<VkPipelineShaderStageCreateInfo, 2> shader_stages;

        wrapper::Shader filtercube(device, VK_SHADER_STAGE_VERTEX_BIT, "filtercube",
                                   "shaders/cubemap/filtercube.vert.spv");

        wrapper::Shader irradiancecube(device, VK_SHADER_STAGE_FRAGMENT_BIT, "irradiancecube",
                                       "shaders/cubemap/irradiancecube.frag.spv");

        wrapper::Shader prefilterenvmap(device, VK_SHADER_STAGE_FRAGMENT_BIT, "prefilterenvmap",
                                        "shaders/cubemap/prefilterenvmap.frag.spv");

        shader_stages[0] = wrapper::make_info<VkPipelineShaderStageCreateInfo>();
        shader_stages[0].module = filtercube.module();
        shader_stages[0].stage = filtercube.type();
        shader_stages[0].pName = filtercube.entry_point().c_str();

        switch (target) {
        case IRRADIANCE:
            shader_stages[1] = wrapper::make_info<VkPipelineShaderStageCreateInfo>();
            shader_stages[1].module = irradiancecube.module();
            shader_stages[1].stage = irradiancecube.type();
            shader_stages[1].pName = irradiancecube.entry_point().c_str();
            break;
        case PREFILTEREDENV:
            shader_stages[1] = wrapper::make_info<VkPipelineShaderStageCreateInfo>();
            shader_stages[1].module = prefilterenvmap.module();
            shader_stages[1].stage = prefilterenvmap.type();
            shader_stages[1].pName = prefilterenvmap.entry_point().c_str();
            break;
        };
        VkGraphicsPipelineCreateInfo pipelineCI{};
        pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCI.layout = pipelinelayout;
        pipelineCI.renderPass = renderpass;
        pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
        pipelineCI.pVertexInputState = &vertexInputStateCI;
        pipelineCI.pRasterizationState = &rasterizationStateCI;
        pipelineCI.pColorBlendState = &colorBlendStateCI;
        pipelineCI.pMultisampleState = &multisampleStateCI;
        pipelineCI.pViewportState = &viewportStateCI;
        pipelineCI.pDepthStencilState = &depthStencilStateCI;
        pipelineCI.pDynamicState = &dynamicStateCI;
        pipelineCI.stageCount = 2;
        pipelineCI.pStages = shader_stages.data();
        pipelineCI.renderPass = renderpass;

        VkPipeline pipeline;
        if (const auto result = vkCreateGraphicsPipelines(device.device(), /*pipelineCache*/ nullptr, 1, &pipelineCI,
                                                          nullptr, &pipeline);
            result != VK_SUCCESS) {
            throw VulkanException("Failed to create graphics pipeline (vkCreateGraphicsPipelines)!", result);
        }

        VkClearValue clearValues[1];
        clearValues[0].color = {{0.0f, 0.0f, 0.2f, 0.0f}};

        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderPass = renderpass;
        renderPassBeginInfo.framebuffer = offscreen.framebuffer;
        renderPassBeginInfo.renderArea.extent.width = dim;
        renderPassBeginInfo.renderArea.extent.height = dim;
        renderPassBeginInfo.clearValueCount = 1;
        renderPassBeginInfo.pClearValues = clearValues;

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

        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = mipmap_count;
        subresourceRange.layerCount = 6;

        // Change image layout for all cubemap faces to transfer destination
        {
            wrapper::OnceCommandBuffer cmd_buf(device, device.graphics_queue(), device.graphics_queue_family_index());

            cmd_buf.create_command_buffer();
            cmd_buf.start_recording();

            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.image = m_cubemap_image->image();
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = 0;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.subresourceRange = subresourceRange;

            vkCmdPipelineBarrier(cmd_buf.command_buffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            cmd_buf.end_recording_and_submit_command();
        }

        for (std::uint32_t m = 0; m < mipmap_count; m++) {
            for (std::uint32_t f = 0; f < 6; f++) {

                wrapper::OnceCommandBuffer cmd_buf(device, device.graphics_queue(),
                                                   device.graphics_queue_family_index());

                cmd_buf.create_command_buffer();
                cmd_buf.start_recording();

                viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
                viewport.height = static_cast<float>(dim * std::pow(0.5f, m));

                vkCmdSetViewport(cmd_buf.command_buffer(), 0, 1, &viewport);
                vkCmdSetScissor(cmd_buf.command_buffer(), 0, 1, &scissor);

                // Render scene from cube face's point of view
                vkCmdBeginRenderPass(cmd_buf.command_buffer(), &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

                // Pass parameters for current pass using a push constant block
                switch (target) {
                case IRRADIANCE:
                    pushBlockIrradiance.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];

                    vkCmdPushConstants(cmd_buf.command_buffer(), pipelinelayout,
                                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                       sizeof(PushBlockIrradiance), &pushBlockIrradiance);
                    break;
                case PREFILTEREDENV:
                    pushBlockPrefilterEnv.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
                    pushBlockPrefilterEnv.roughness = (float)m / (float)(mipmap_count - 1);

                    vkCmdPushConstants(cmd_buf.command_buffer(), pipelinelayout,
                                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                                       sizeof(PushBlockPrefilterEnv), &pushBlockPrefilterEnv);
                    break;
                };

                vkCmdBindPipeline(cmd_buf.command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
                vkCmdBindDescriptorSets(cmd_buf.command_buffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1,
                                        &descriptorset, 0, nullptr);

                VkDeviceSize offsets[1] = {0};

                // models.skybox.draw(cmdBuf);

                vkCmdEndRenderPass(cmd_buf.command_buffer());

                VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
                subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                subresourceRange.baseMipLevel = 0;
                subresourceRange.levelCount = mipmap_count;
                subresourceRange.layerCount = 6;

                {
                    VkImageMemoryBarrier imageMemoryBarrier{};
                    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageMemoryBarrier.image = offscreen.image;
                    imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                    imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
                    imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                    imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                    imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

                    vkCmdPipelineBarrier(cmd_buf.command_buffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                         VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1,
                                         &imageMemoryBarrier);
                }

                // Copy region for transfer from framebuffer to cube face
                VkImageCopy copyRegion{};
                copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.srcSubresource.baseArrayLayer = 0;
                copyRegion.srcSubresource.mipLevel = 0;
                copyRegion.srcSubresource.layerCount = 1;
                copyRegion.srcOffset = {0, 0, 0};
                copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                copyRegion.dstSubresource.baseArrayLayer = f;
                copyRegion.dstSubresource.mipLevel = m;
                copyRegion.dstSubresource.layerCount = 1;
                copyRegion.dstOffset = {0, 0, 0};
                copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
                copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
                copyRegion.extent.depth = 1;

                vkCmdCopyImage(cmd_buf.command_buffer(), offscreen.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                               m_cubemap_image->image(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

                {
                    VkImageMemoryBarrier imageMemoryBarrier{};
                    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
                    imageMemoryBarrier.image = offscreen.image;
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

        {
            wrapper::OnceCommandBuffer cmd_buf(device, device.graphics_queue(), device.graphics_queue_family_index());

            cmd_buf.create_command_buffer();
            cmd_buf.start_recording();

            VkImageMemoryBarrier imageMemoryBarrier{};
            imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
            imageMemoryBarrier.image = m_cubemap_image->image();
            imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
            imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            imageMemoryBarrier.subresourceRange = subresourceRange;

            vkCmdPipelineBarrier(cmd_buf.command_buffer(), VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                 VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

            cmd_buf.end_recording_and_submit_command();
        }

        // cubemap.descriptor.imageView = cubemap.view;
        // cubemap.descriptor.sampler = cubemap.sampler;
        // cubemap.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        // cubemap.device = vulkanDevice;

        /*
        switch (target) {
        case IRRADIANCE:
            textures.irradianceCube = cubemap;
            break;
        case PREFILTEREDENV:
            textures.prefilteredCube = cubemap;
            shaderValuesParams.prefilteredCubeMipLevels = static_cast<float>(mipmap_count);
            break;
        };
        */

        spdlog::trace("Generating cubemap finished");
    }
}

} // namespace inexor::vulkan_renderer::cubemap
