#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(Device &device, Swapchain &swapchain)
    : m_device(device), m_swapchain(swapchain), m_graphics_pipeline_builder(device),
      m_descriptor_set_layout_builder(device), m_descriptor_set_allocator(m_device),
      m_descriptor_set_update_builder(m_device) {}

void RenderGraph::add_graphics_pass(OnCreateGraphicsPass on_pass_create) {
    m_graphics_pass_create_functions.emplace_back(std::move(on_pass_create));
}

void RenderGraph::add_graphics_pipeline(OnCreateGraphicsPipeline on_pipeline_create) {
    m_pipeline_create_functions.emplace_back(std::move(on_pipeline_create));
}

std::weak_ptr<Buffer>
RenderGraph::add_buffer(std::string buffer_name, const BufferType buffer_type, std::function<void()> on_update) {
    m_buffers.emplace_back(
        std::make_shared<Buffer>(m_device, std::move(buffer_name), buffer_type, std::move(on_update)));
    return m_buffers.back();
}

void RenderGraph::allocate_descriptor_sets() {
    for (const auto &descriptor : m_resource_descriptors) {
        // Call the on_update_descriptor_set function of each resource descriptor
        std::invoke(std::get<1>(descriptor), m_descriptor_set_allocator);
    }
}

void RenderGraph::add_resource_descriptor(OnBuildDescriptorSetLayout on_build_descriptor_set_layout,
                                          OnAllocateDescriptorSet on_allocate_descriptor_set,
                                          OnUpdateDescriptorSet on_update_descriptor_set) {
    // NOTE: This only stores the functions and they will be called in the correct order during rendergraph compilation
    m_resource_descriptors.emplace_back(std::move(on_build_descriptor_set_layout),
                                        std::move(on_allocate_descriptor_set), std::move(on_update_descriptor_set));
}

std::weak_ptr<Texture> RenderGraph::add_texture(std::string texture_name,
                                                const TextureUsage usage,
                                                const VkFormat format,
                                                const std::uint32_t width,
                                                const std::uint32_t height,
                                                std::optional<std::function<void()>> on_init,
                                                std::optional<std::function<void()>> on_update) {
    m_textures.emplace_back(std::make_shared<Texture>(m_device, std::move(texture_name), usage, format, width, height,
                                                      std::move(on_init), std::move(on_update)));
    return m_textures.back();
}

void RenderGraph::check_for_cycles() {
    // TODO: Implement me!
}

void RenderGraph::compile() {
    // TODO: What needs to be re-done when swapchain is recreated?
    validate_render_graph();
    determine_pass_order();
    create_buffers();
    create_textures();
    create_descriptor_set_layouts();
    allocate_descriptor_sets();
    update_descriptor_sets();
    create_graphics_passes();
    create_graphics_pipelines();
    create_rendering_infos();
}

void RenderGraph::create_buffers() {
    m_device.execute("[RenderGraph::create_buffers]", [&](const CommandBuffer &cmd_buf) {
        for (const auto &buffer : m_buffers) {
            buffer->m_on_update();
            buffer->create(cmd_buf);
        }
    });
}

void RenderGraph::create_descriptor_set_layouts() {
    for (const auto &descriptor : m_resource_descriptors) {
        // Call on_update_descriptor_set for each descriptor
        std::invoke(std::get<0>(descriptor), m_descriptor_set_layout_builder);
    }
}

void RenderGraph::create_graphics_passes() {
    m_graphics_passes.clear();
    m_graphics_passes.reserve(m_graphics_pass_create_functions.size());
    for (const auto &create_func : m_graphics_pass_create_functions) {
        m_graphics_passes.emplace_back(create_func(m_graphics_pass_builder));
    }
}

void RenderGraph::create_graphics_pipelines() {
    for (const auto &create_func : m_pipeline_create_functions) {
        create_func(m_graphics_pipeline_builder);
    }
}

void RenderGraph::create_rendering_infos() {
    for (auto &pass : m_graphics_passes) {
        /// Fill VkRenderingAttachmentInfo for a given render_graph::Attachment
        /// @param attachment The attachment (color, depth, or stencil)
        /// @reutrn VkRenderingAttachmentInfo  The filled rendering info struct
        auto fill_rendering_info = [&](const Attachment &attachment) {
            const auto attach_ptr = attachment.first.lock();
            const auto img_layout = [&]() -> VkImageLayout {
                switch (attach_ptr->m_usage) {
                case TextureUsage::BACK_BUFFER:
                case TextureUsage::DEPTH_STENCIL_BUFFER: {
                    return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                }
                default:
                    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
                }
            }(); // Invoke the lambda, not just define it!

            // This decides if MSAA is enabled on a per-texture basis
            return wrapper::make_info<VkRenderingAttachmentInfo>({
                .imageView = attach_ptr->m_img->m_img_view,
                .imageLayout = img_layout,
                .resolveMode = attach_ptr ? VK_RESOLVE_MODE_MIN_BIT : VK_RESOLVE_MODE_NONE,
                .resolveImageView = attach_ptr ? attach_ptr->m_msaa_img->m_img_view : VK_NULL_HANDLE,
                .resolveImageLayout = attach_ptr ? img_layout : VK_IMAGE_LAYOUT_UNDEFINED,
                .loadOp = attachment.second ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .clearValue = attachment.second.value_or(VkClearValue{}),
            });
        };
        // Fill the color attachments
        pass.m_color_attachment_infos.reserve(pass.m_color_attachments.size());
        for (const auto &color_attachment : pass.m_color_attachments) {
            pass.m_color_attachment_infos.push_back(fill_rendering_info(color_attachment));
        }
        // Fill the color attachment (if specified)
        const bool has_depth_attachment = !pass.m_depth_attachment.first.expired();
        if (has_depth_attachment) {
            pass.m_depth_attachment_info = fill_rendering_info(pass.m_depth_attachment);
        }
        // Fill the stencil attachment (if specified)
        const bool has_stencil_attachment = !pass.m_stencil_attachment.first.expired();
        if (has_stencil_attachment) {
            pass.m_stencil_attachment_info = fill_rendering_info(pass.m_stencil_attachment);
        }
        // We store all this data in the rendering info in the graphics pass itself
        // The advantage of this is that we don't have to fill this during the actual rendering
        pass.m_rendering_info = std::move(wrapper::make_info<VkRenderingInfo>({
            .renderArea =
                {
                    .extent = m_swapchain.extent(),
                },
            .layerCount = 1,
            .colorAttachmentCount = static_cast<std::uint32_t>(pass.m_color_attachment_infos.size()),
            .pColorAttachments = pass.m_color_attachment_infos.data(),
            .pDepthAttachment = has_depth_attachment ? &pass.m_depth_attachment_info : nullptr,
            .pStencilAttachment = has_stencil_attachment ? &pass.m_stencil_attachment_info : nullptr,
        }));
    }
}

void RenderGraph::create_textures() {
    m_device.execute("[RenderGraph::create_textures]", [&](const CommandBuffer &cmd_buf) {
        for (const auto &texture : m_textures) {
            switch (texture->m_usage) {
            case TextureUsage::NORMAL: {
                if (texture->m_on_init) {
                    std::invoke(texture->m_on_init.value());
                    // TODO: Implement me!
                    texture->create();
                    texture->update(cmd_buf);
                }
                break;
            }
            case TextureUsage::DEPTH_STENCIL_BUFFER: {
                // TODO: Implement me!
                break;
            }
            case TextureUsage::BACK_BUFFER: {
                // TODO: Implement me!
                break;
            }
            default: {
                break;
            }
            }
        }
    });
}

void RenderGraph::determine_pass_order() {
    m_log->warn("Implement determine_pass_order()");
    // TODO: The data structure to determine pass order should be created before rendergraph compilation!
}

void RenderGraph::record_command_buffer_for_pass(const CommandBuffer &cmd_buf, const GraphicsPass &pass) {
    // TODO: Define default color values for debug labels!
    // Start a new debug label for this graphics pass (visible in graphics debuggers like RenderDoc)
    cmd_buf.begin_debug_label_region(pass.m_name, {1.0f, 0.0f, 0.0f, 1.0f});
    // Start dynamic rendering with the compiled rendering info
    cmd_buf.begin_rendering(pass.m_rendering_info);
    // Call the command buffer recording function of this graphics pass
    std::invoke(pass.m_on_record_cmd_buffer, cmd_buf);
    // End dynamic rendering
    cmd_buf.end_rendering();
    // End the debug label for this graphics pass
    cmd_buf.end_debug_label_region();
}

void RenderGraph::record_command_buffers(const CommandBuffer &cmd_buf, const std::uint32_t img_index) {
    // Transform the image layout of the swapchain (it comes in undefined layout after presenting)
    cmd_buf.change_image_layout(m_swapchain.image(img_index), VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    for (const auto &pass : m_graphics_passes) {
        record_command_buffer_for_pass(cmd_buf, pass);
    }
    // Change the layout of the swapchain image to make it ready for presenting
    cmd_buf.change_image_layout(m_swapchain.image(img_index), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
}

void RenderGraph::render() {
    const auto &cmd_buf = m_device.request_command_buffer("[RenderGraph::render]");
    // TODO: Record command buffers for passes in parallel!
    record_command_buffers(cmd_buf, m_swapchain.acquire_next_image_index());

    // TODO: Further abstract this away?
    const std::array<VkPipelineStageFlags, 1> stage_mask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    cmd_buf.submit_and_wait(wrapper::make_info<VkSubmitInfo>({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_swapchain.image_available_semaphore(),
        .pWaitDstStageMask = stage_mask.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = cmd_buf.ptr(),
    }));
    m_swapchain.present();
}

void RenderGraph::reset() {
    // TODO: Implement me!
}

void RenderGraph::update_buffers() {
    m_device.execute("[RenderGraph::update_buffers]", [&](const CommandBuffer &cmd_buf) {
        for (const auto &buffer : m_buffers) {
            // Does this buffer need to be updated?
            if (buffer->m_update_requested) {
                buffer->destroy();
                // Call the buffer update function
                std::invoke(buffer->m_on_update);
                buffer->create(cmd_buf);
            }
        }
    });
}

void RenderGraph::update_descriptor_sets() {
    for (const auto &descriptor : m_resource_descriptors) {
        // Call on_update_descriptor_set for each descriptor
        std::invoke(std::get<2>(descriptor), m_descriptor_set_update_builder);
    }
}

void RenderGraph::update_textures() {
    m_device.execute("[RenderGraph::update_textures]", [&](const CommandBuffer &cmd_buf) {
        for (const auto &texture : m_textures) {
            // Only for dynamic textures m_on_lambda which is not std::nullopt
            if (texture->m_on_update) {
                if (texture->m_update_requested) {
                    texture->destroy();
                    std::invoke(texture->m_on_update.value());
                    texture->create();
                    texture->update(cmd_buf);
                }
            }
        }
    });
}

void RenderGraph::validate_render_graph() {
    if (m_graphics_pass_create_functions.empty()) {
        throw std::runtime_error("[RenderGraph::validate_render_graph] Error: No graphics passes in rendergraph!");
    }
    if (m_pipeline_create_functions.empty()) {
        throw std::runtime_error("[RenderGraph::validate_render_graph] Error: No graphics pipelines in rendergraph!");
    }
    check_for_cycles();
}

} // namespace inexor::vulkan_renderer::render_graph
