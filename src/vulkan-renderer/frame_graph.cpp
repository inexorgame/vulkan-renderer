#include "inexor/vulkan-renderer/frame_graph.hpp"

#include <spdlog/spdlog.h>
#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <cassert>
#include <functional>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace inexor::vulkan_renderer {

namespace {

std::shared_ptr<spdlog::logger> s_log;

} // namespace

// TODO(): Use wrapper::Image
PhysicalImage::~PhysicalImage() {
    vkDestroyImageView(m_device, m_image_view, nullptr);
    vmaDestroyImage(m_allocator, m_image, m_allocation);
}

void RenderStage::writes_to(const RenderResource &resource) {
    m_writes.push_back(&resource);
}

void RenderStage::reads_from(const RenderResource &resource) {
    m_reads.push_back(&resource);
}

void GraphicsStage::uses_shader(const wrapper::Shader &shader) {
    VkPipelineShaderStageCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    create_info.module = shader.get_module();

    // TODO(): Have stage type (e.g. vertex/fragment) as field of GraphicsStage
    // TODO(): Maybe even have a separate VertexStage and FragmentStage?
    create_info.stage = shader.get_type();
    create_info.pName = shader.get_entry_point().c_str();
    m_shaders.push_back(create_info);
}

FrameGraph::FrameGraph() {
    s_log = spdlog::default_logger()->clone("frame-graph");
}

void FrameGraph::compile(const RenderResource &target, VkDevice device, VkCommandPool command_pool,
                         VmaAllocator allocator, const wrapper::Swapchain &swapchain) {
    // TODO(): Better logging and input validation
    // TODO(): Many opportunities for optimisation
    // TODO(): Use OOP wrapper functions (when we have them), e.g. device->alloc_command_pool() and
    // TODO(): command_pool.alloc_command_buffer()

    std::unordered_map<const RenderResource *, std::vector<RenderStage *>> writers;
    for (auto &stage : m_stages) {
        for (const auto *resource : stage->m_writes) {
            writers[resource].push_back(stage.get());
        }
    }

    // Post order depth first search
    // NOTE: Doesn't do any colouring, only works on acyclic graphs!
    // TODO(): Stage graph validation (ensuring no cycles, etc)
    // TODO(): Move away from recursive dfs algo
    std::function<void(RenderStage *)> dfs = [&](RenderStage *stage) {
        for (const auto *resource : stage->m_reads) {
            for (auto *writer : writers[resource]) {
                dfs(writer);
            }
        }
        m_stage_stack.push_back(stage);
    };

    // DFS starting from writers of target (initial stage executants)
    // TODO(): Will there be more than one writer to the target (back buffer), maybe with blending?
    for (auto *stage : writers[&target]) {
        dfs(stage);
    }

    s_log->debug("Final stage order:");
    for (auto *stage : m_stage_stack) {
        s_log->debug("  - {}", stage->m_name);
    }

    // Render pass creation
    // NOTE: Each render stage, after merging and reordering, maps to a vulkan render pass and pipeline
    for (auto &stage : m_stages) {
        auto *graphics_stage = dynamic_cast<GraphicsStage *>(stage.get());
        if (graphics_stage == nullptr) {
            continue;
        }

        /*
                dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
           VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT; dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
                dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
         */

        VkSubpassDependency subpass_dependency = {};
        subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

        std::vector<VkAttachmentDescription> attachments;
        std::vector<VkAttachmentReference> colour_refs;
        std::vector<VkAttachmentReference> depth_refs;

        // Attachment creation
        // TODO(): Support multisampled attachments
        for (std::size_t i = 0; i < stage->m_writes.size(); i++) {
            const auto *resource = stage->m_writes[i];
            const auto *texture = dynamic_cast<const TextureResource *>(resource);
            if (texture == nullptr) {
                continue;
            }

            VkAttachmentDescription attachment = {};
            attachment.format = texture->m_format;
            attachment.samples = VK_SAMPLE_COUNT_1_BIT;

            // TODO(): Switch to VK_ATTACHMENT_LOAD_OP_DONT_CARE?
            attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

            attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            switch (texture->m_usage) {
            case TextureUsage::BACK_BUFFER:
                attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                colour_refs.push_back({static_cast<std::uint32_t>(i), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                break;
            case TextureUsage::DEPTH_BUFFER:
                attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                depth_refs.push_back({static_cast<std::uint32_t>(i), attachment.finalLayout});
                break;
            default:
                attachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                colour_refs.push_back({static_cast<std::uint32_t>(i), attachment.finalLayout});
                break;
            }

            attachments.push_back(attachment);
        }

        VkSubpassDescription subpass_description = {};
        subpass_description.colorAttachmentCount = colour_refs.size();
        subpass_description.pColorAttachments = colour_refs.data();
        subpass_description.pDepthStencilAttachment = depth_refs.data();
        subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

        // TODO(): Use wrapper::RenderPass (maybe with some kind of builder pattern?)
        VkRenderPassCreateInfo render_pass_ci = {};
        render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_ci.attachmentCount = static_cast<std::uint32_t>(attachments.size());
        render_pass_ci.dependencyCount = 1;
        render_pass_ci.subpassCount = 1;
        render_pass_ci.pAttachments = attachments.data();
        render_pass_ci.pDependencies = &subpass_dependency;
        render_pass_ci.pSubpasses = &subpass_description;

        if (vkCreateRenderPass(device, &render_pass_ci, nullptr, &graphics_stage->m_render_pass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }
    }

    // Physical resource allocation (using VMA for now)
    // TODO(): Resource aliasing (i.e. reusing the same physical resource for multiple resources)
    for (const auto &resource : m_resources) {
        s_log->trace("Allocating physical buffer for resource '{}'", resource->m_name);
        VmaAllocationCreateInfo alloc_ci = {};
        alloc_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        alloc_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

        // TODO(): Use a constexpr bool
#if VMA_RECORDING_ENABLED
        alloc_ci.flags |= VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
        alloc_ci.pUserData = const_cast<char *>(resource->m_name.data());
#endif

        const auto *texture = dynamic_cast<const TextureResource *>(resource.get());
        if (texture == nullptr) {
            continue;
        }

        PhysicalImage *phys = nullptr;
        if (texture->m_usage == TextureUsage::BACK_BUFFER) {
            phys = &create_phys_resource<PhysicalBackBuffer>(texture, allocator, device);
        } else {
            phys = &create_phys_resource<PhysicalImage>(texture, allocator, device);
        }

        // TODO(): Use image wrapper
        VkImageCreateInfo image_ci = {};
        image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_ci.imageType = VK_IMAGE_TYPE_2D;

        // TODO(): Support textures with dimensions not equal to back buffer size
        image_ci.extent.width = swapchain.get_extent().width;
        image_ci.extent.height = swapchain.get_extent().height;
        image_ci.extent.depth = 1;

        image_ci.arrayLayers = 1;
        image_ci.mipLevels = 1;
        image_ci.format = texture->m_format;
        image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_ci.samples = VK_SAMPLE_COUNT_1_BIT;
        image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
        image_ci.usage = texture->m_usage == TextureUsage::DEPTH_BUFFER ? VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
                                                                        : VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        VmaAllocationInfo alloc_info;
        if (vmaCreateImage(allocator, &image_ci, &alloc_ci, &phys->m_image, &phys->m_allocation, &alloc_info) !=
            VK_SUCCESS) {
            throw std::runtime_error("Failed to create image!");
        }

        VkImageViewCreateInfo image_view_ci = {};
        image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_ci.format = texture->m_format;
        image_view_ci.image = phys->m_image;
        image_view_ci.subresourceRange.aspectMask =
            texture->m_usage == TextureUsage::DEPTH_BUFFER ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_ci.subresourceRange.layerCount = 1;
        image_view_ci.subresourceRange.levelCount = 1;
        image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;

        if (vkCreateImageView(device, &image_view_ci, nullptr, &phys->m_image_view) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create image view!");
        }
    }

    // Special back buffer handling
    // TODO(): Refactor, this code is a mess
    for (const auto &resource : m_resources) {
        const auto *texture = dynamic_cast<const TextureResource *>(resource.get());
        if (texture == nullptr) {
            continue;
        }

        if (texture->m_usage != TextureUsage::BACK_BUFFER) {
            continue;
        }

        // Find depth buffer
        const TextureResource *depth_buffer = nullptr;
        for (const auto &res : m_resources) {
            if (const auto *tex = dynamic_cast<const TextureResource *>(res.get())) {
                if (tex->m_usage == TextureUsage::DEPTH_BUFFER) {
                    depth_buffer = tex;
                }
            }
        }

        const auto *back_buffer_writer = dynamic_cast<const GraphicsStage *>(writers[texture][0]);
        assert(back_buffer_writer != nullptr);
        assert(depth_buffer != nullptr);

        // TODO(): Merge swapchain with back buffer/depth buffer
        std::vector<VkImageView> image_views;
        image_views.push_back(nullptr);
        image_views.push_back(static_cast<PhysicalImage *>(m_resource_map[depth_buffer].get())->m_image_view);

        auto *phys_back_buffer = static_cast<PhysicalBackBuffer *>(m_resource_map[texture].get());
        phys_back_buffer->m_framebuffer =
            std::make_unique<wrapper::Framebuffer>(device, back_buffer_writer->m_render_pass, swapchain, image_views);
    }

    // Pipeline layout creation
    for (auto &stage : m_stages) {
        auto *graphics_stage = dynamic_cast<GraphicsStage *>(stage.get());
        if (graphics_stage == nullptr) {
            continue;
        }

        graphics_stage->m_pipeline_layout = std::make_unique<wrapper::PipelineLayout>(
            device, graphics_stage->m_descriptor_layouts, "Default pipeline layout");
    }

    // Pipeline creation
    for (auto &stage : m_stages) {
        auto *graphics_stage = dynamic_cast<GraphicsStage *>(stage.get());
        if (graphics_stage == nullptr) {
            continue;
        }

        // TODO(): Add wrapper::VertexBuffer (as well as UniformBuffer)
        VkPipelineVertexInputStateCreateInfo vertex_input = {};
        vertex_input.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input.vertexAttributeDescriptionCount = graphics_stage->m_attribute_bindings.size();
        vertex_input.vertexBindingDescriptionCount = graphics_stage->m_vertex_bindings.size();
        vertex_input.pVertexAttributeDescriptions = graphics_stage->m_attribute_bindings.data();
        vertex_input.pVertexBindingDescriptions = graphics_stage->m_vertex_bindings.data();

        // TODO(): Support primitives other than triangles
        VkPipelineInputAssemblyStateCreateInfo input_assembly = {};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.primitiveRestartEnable = VK_FALSE;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        VkPipelineDepthStencilStateCreateInfo depth_stencil = {};
        depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL; // TODO(): Check nvidia tips for best func
        depth_stencil.depthTestEnable = VK_TRUE;
        depth_stencil.depthWriteEnable = VK_TRUE; // TODO(): Needed?

        VkPipelineMultisampleStateCreateInfo multisample_state = {};
        multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state.minSampleShading = 1.0F;
        multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineRasterizationStateCreateInfo rasterization_state = {};
        rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterization_state.lineWidth = 1.0F;
        rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;

        VkPipelineColorBlendAttachmentState blend_attachment = {};
        blend_attachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

        VkPipelineColorBlendStateCreateInfo blend_state = {};
        blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        blend_state.attachmentCount = 1;
        blend_state.pAttachments = &blend_attachment;

        VkRect2D scissor = {};
        scissor.extent = swapchain.get_extent();

        VkViewport viewport = {};
        viewport.width = swapchain.get_extent().width;
        viewport.height = swapchain.get_extent().height;
        viewport.maxDepth = 1.0F;

        // TODO(): Custom scissors?
        VkPipelineViewportStateCreateInfo viewport_state = {};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.scissorCount = 1;
        viewport_state.viewportCount = 1;
        viewport_state.pScissors = &scissor;
        viewport_state.pViewports = &viewport;

        VkGraphicsPipelineCreateInfo pipeline_ci = {};
        pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline_ci.pVertexInputState = &vertex_input;
        pipeline_ci.pInputAssemblyState = &input_assembly;
        pipeline_ci.pDepthStencilState = &depth_stencil;
        pipeline_ci.pMultisampleState = &multisample_state;
        pipeline_ci.pRasterizationState = &rasterization_state;
        pipeline_ci.pColorBlendState = &blend_state;
        pipeline_ci.pViewportState = &viewport_state;
        pipeline_ci.layout = graphics_stage->m_pipeline_layout->get();
        pipeline_ci.renderPass = graphics_stage->m_render_pass;
        pipeline_ci.stageCount = graphics_stage->m_shaders.size();
        pipeline_ci.pStages = graphics_stage->m_shaders.data();

        if (vkCreateGraphicsPipelines(device, nullptr, 1, &pipeline_ci, nullptr, &stage->m_pipeline) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline!");
        }
    }

    const auto *back_buffer = dynamic_cast<PhysicalBackBuffer *>(m_resource_map[&target].get());
    assert(back_buffer != nullptr);

    // Command buffer creation and recording
    for (auto &stage : m_stages) {
        s_log->trace("Allocating command buffers for stage '{}'", stage->m_name);
        stage->m_command_buffers.reserve(swapchain.get_image_count());

        VkCommandBufferAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        alloc_info.commandBufferCount = swapchain.get_image_count();
        alloc_info.commandPool = command_pool;
        alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

        if (vkAllocateCommandBuffers(device, &alloc_info, stage->m_command_buffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate command buffers!");
        }

        VkCommandBufferBeginInfo cmd_buf_bi = {};
        cmd_buf_bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        s_log->trace("Recording command buffers for stage '{}'", stage->m_name);
        for (std::uint32_t i = 0; i < swapchain.get_image_count(); i++) {
            auto *cmd_buf = stage->m_command_buffers[i];
            vkBeginCommandBuffer(cmd_buf, &cmd_buf_bi);

            // Record render pass for graphics stages
            const auto *graphics_stage = dynamic_cast<const GraphicsStage *>(stage.get());
            if (graphics_stage != nullptr) {
                VkRenderPassBeginInfo render_pass_bi = {};
                render_pass_bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_bi.clearValueCount = 2;
                render_pass_bi.pClearValues = &graphics_stage->m_clear_colour;
                render_pass_bi.framebuffer = back_buffer->m_framebuffer->get(i);
                render_pass_bi.renderArea.extent = swapchain.get_extent();
                render_pass_bi.renderPass = graphics_stage->m_render_pass;

                vkCmdBeginRenderPass(cmd_buf, &render_pass_bi, VK_SUBPASS_CONTENTS_INLINE);
            }

            vkCmdBindPipeline(cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, stage->m_pipeline);
            stage->m_on_record(cmd_buf);

            if (graphics_stage != nullptr) {
                vkCmdEndRenderPass(cmd_buf);
            }
            vkEndCommandBuffer(cmd_buf);
        }
    }
}

} // namespace inexor::vulkan_renderer
