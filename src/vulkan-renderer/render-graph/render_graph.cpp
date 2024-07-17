#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <unordered_set>

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
        // Call descriptor set allocation function of each resource descriptor
        std::invoke(std::get<1>(descriptor), m_descriptor_set_allocator);
    }
}

void RenderGraph::add_resource_descriptor(OnBuildDescriptorSetLayout on_build_descriptor_set_layout,
                                          OnAllocateDescriptorSet on_allocate_descriptor_set,
                                          OnUpdateDescriptorSet on_update_descriptor_set) {
    m_resource_descriptors.emplace_back(std::move(on_build_descriptor_set_layout),
                                        std::move(on_allocate_descriptor_set), std::move(on_update_descriptor_set));
}

std::weak_ptr<Texture> RenderGraph::add_texture(std::string texture_name,
                                                const TextureUsage usage,
                                                const VkFormat format,
                                                const std::uint32_t width,
                                                const std::uint32_t height,
                                                const VkSampleCountFlagBits sample_count,
                                                std::optional<std::function<void()>> on_init,
                                                std::optional<std::function<void()>> on_update) {
    m_textures.emplace_back(std::make_shared<Texture>(m_device, std::move(texture_name), usage, format, width, height,
                                                      sample_count, std::move(on_init), std::move(on_update)));
    return m_textures.back();
}

void RenderGraph::check_for_cycles() {
    std::unordered_set<std::shared_ptr<GraphicsPass>> visited;
    std::unordered_set<std::shared_ptr<GraphicsPass>> recursion_stack;

    std::function<bool(const std::shared_ptr<GraphicsPass> &)> dfs_detect_cycle =
        [&](const std::shared_ptr<GraphicsPass> &pass) {
            if (recursion_stack.find(pass) != recursion_stack.end()) {
                return true; // Cycle detected
            }
            if (visited.find(pass) != visited.end()) {
                return false; // Already visited, no cycle
            }
            visited.insert(pass);
            recursion_stack.insert(pass);
            for (const auto &parent : pass->m_graphics_pass_reads) {
                if (dfs_detect_cycle(parent.lock())) {
                    return true; // Cycle detected in one of the parents
                }
            }
            recursion_stack.erase(pass);
            return false; // No cycle found for this pass
        };

    for (const auto &pass : m_graphics_passes) {
        if (dfs_detect_cycle(pass)) {
            throw std::runtime_error("[RenderGraph::check_for_cycles] Error: Rendergraph is not acyclic! "
                                     "A cycle was detected for graphics pass " +
                                     pass->m_name + "!");
        }
    }
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
    check_for_cycles();
}

void RenderGraph::create_buffers() {
    m_device.execute("RenderGraph::create_buffers|", [&](const CommandBuffer &cmd_buf) {
        for (const auto &buffer : m_buffers) {
            buffer->m_on_update();
            cmd_buf.set_suboperation_debug_name("Buffer:" + buffer->m_name);
            buffer->create(cmd_buf);
        }
    });
}

void RenderGraph::create_descriptor_set_layouts() {
    for (const auto &descriptor : m_resource_descriptors) {
        // Call descriptor set layout create function for each descriptor
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

void RenderGraph::create_textures() {
    m_device.execute("RenderGraph::create_textures", [&](const CommandBuffer &cmd_buf) {
        for (const auto &texture : m_textures) {
            if (texture->m_on_init) {
                cmd_buf.set_suboperation_debug_name("|Texture|Initialize:" + texture->m_name);
                std::invoke(texture->m_on_init.value());
            }
            cmd_buf.set_suboperation_debug_name("|Texture|Create:" + texture->m_name);
            texture->create();
            if (texture->m_usage == TextureUsage::NORMAL) {
                cmd_buf.set_suboperation_debug_name("|Texture|Update:" + texture->m_name);
                // Only external textures are updated, not back or depth buffers used internally in rendergraph
                texture->update(cmd_buf);
            }
        }
    });
}

void RenderGraph::determine_pass_order() {
    m_log->warn("Implement determine_pass_order()");
    // TODO: The data structure to determine pass order should be created before rendergraph compilation!
}

void RenderGraph::fill_graphics_pass_rendering_info(GraphicsPass &pass) {
    /// Fill VkRenderingAttachmentInfo for a given render_graph::Attachment
    /// @param attachment The attachment (color, depth, or stencil)
    /// @return VkRenderingAttachmentInfo  The filled rendering info struct
    auto fill_rendering_info = [&](const RenderingAttachment &attachment) {
        const auto attach_ptr = attachment.first.lock();
        const auto img_layout = [&]() -> VkImageLayout {
            switch (attach_ptr->m_usage) {
            case TextureUsage::BACK_BUFFER:
            case TextureUsage::NORMAL: {
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            case TextureUsage::DEPTH_STENCIL_BUFFER: {
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
            default:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            }
        }(); // Invoke the lambda, not just define it!

        auto is_integer_format = [](const VkFormat format) {
            switch (format) {
            // 8-bit integer formats
            case VK_FORMAT_R8_UINT:
            case VK_FORMAT_R8_SINT:
            case VK_FORMAT_R8G8_UINT:
            case VK_FORMAT_R8G8_SINT:
            case VK_FORMAT_R8G8B8_UINT:
            case VK_FORMAT_R8G8B8_SINT:
            case VK_FORMAT_B8G8R8_UINT:
            case VK_FORMAT_B8G8R8_SINT:
            case VK_FORMAT_R8G8B8A8_UINT:
            case VK_FORMAT_R8G8B8A8_SINT:
            case VK_FORMAT_B8G8R8A8_UINT:
            case VK_FORMAT_B8G8R8A8_SINT:
            case VK_FORMAT_A2R10G10B10_UINT_PACK32:
            case VK_FORMAT_A2B10G10R10_UINT_PACK32:

            // 16-bit integer formats
            case VK_FORMAT_R16_UINT:
            case VK_FORMAT_R16_SINT:
            case VK_FORMAT_R16G16_UINT:
            case VK_FORMAT_R16G16_SINT:
            case VK_FORMAT_R16G16B16_UINT:
            case VK_FORMAT_R16G16B16_SINT:
            case VK_FORMAT_R16G16B16A16_UINT:
            case VK_FORMAT_R16G16B16A16_SINT:

            // 32-bit integer formats
            case VK_FORMAT_R32_UINT:
            case VK_FORMAT_R32_SINT:
            case VK_FORMAT_R32G32_UINT:
            case VK_FORMAT_R32G32_SINT:
            case VK_FORMAT_R32G32B32_UINT:
            case VK_FORMAT_R32G32B32_SINT:
            case VK_FORMAT_R32G32B32A32_UINT:
            case VK_FORMAT_R32G32B32A32_SINT:

            // 64-bit integer formats
            case VK_FORMAT_R64_UINT:
            case VK_FORMAT_R64_SINT:
            case VK_FORMAT_R64G64_UINT:
            case VK_FORMAT_R64G64_SINT:
            case VK_FORMAT_R64G64B64_UINT:
            case VK_FORMAT_R64G64B64_SINT:
            case VK_FORMAT_R64G64B64A64_UINT:
            case VK_FORMAT_R64G64B64A64_SINT:
                return true;

            // Non-integer formats
            default:
                return false;
            }
        };

        bool msaa_enabled = (attach_ptr->m_msaa_img != nullptr);

        // This decides if MSAA is enabled on a per-texture basis
        return wrapper::make_info<VkRenderingAttachmentInfo>({
            .imageView = msaa_enabled ? attach_ptr->m_msaa_img->m_img_view : attach_ptr->m_img->m_img_view,
            // The image layout is the same for m_img and m_msaa_img
            .imageLayout = img_layout,
            // The resolve mode must be chosen on whether it's an integer format
            .resolveMode = msaa_enabled ? (!is_integer_format(attach_ptr->m_format) ? VK_RESOLVE_MODE_AVERAGE_BIT
                                                                                    : VK_RESOLVE_MODE_MIN_BIT)
                                        : VK_RESOLVE_MODE_NONE, // No resolve if MSAA is disabled
            // This will be filled during rendering!
            .resolveImageView = m_swapchain.m_current_img_view,
            // TODO: Is this correct layout?
            .resolveImageLayout = msaa_enabled ? img_layout : VK_IMAGE_LAYOUT_UNDEFINED,
            // Does this pass require clearing this attachment?
            .loadOp = attachment.second ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            // The result will always be stored
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            // If clearing is enabled, specify the clear value
            .clearValue = attachment.second ? attachment.second.value() : VkClearValue{},
        });
    };

    // Reserve memory for the color attachments
    pass.m_color_attachment_infos.reserve(pass.m_color_attachments.size());

    // Fill the color attachments
    const bool has_any_color_attachment = pass.m_color_attachments.size() > 0;
    for (const auto &color_attachment : pass.m_color_attachments) {
        pass.m_color_attachment_infos.push_back(fill_rendering_info(color_attachment));
    }
    // Fill the color attachment (if specified)
    const bool has_depth_attachment = !pass.m_depth_attachment.first.expired();
    if (has_depth_attachment) {
        pass.m_depth_attachment_info = fill_rendering_info(pass.m_depth_attachment);
        pass.m_depth_attachment_info.resolveMode = VK_RESOLVE_MODE_NONE;
    }
    // Fill the stencil attachment (if specified)
    const bool has_stencil_attachment = !pass.m_stencil_attachment.first.expired();
    if (has_stencil_attachment) {
        pass.m_stencil_attachment_info = fill_rendering_info(pass.m_stencil_attachment);
    }

    // Fill the rendering info of the pass
    pass.m_rendering_info = wrapper::make_info<VkRenderingInfo>({
        .renderArea =
            {
                .extent = m_swapchain.extent(),
            },
        .layerCount = 1,
        .viewMask = 0,
        .colorAttachmentCount = static_cast<std::uint32_t>(pass.m_color_attachments.size()),
        .pColorAttachments = has_any_color_attachment ? pass.m_color_attachment_infos.data() : nullptr,
        .pDepthAttachment = has_depth_attachment ? &pass.m_depth_attachment_info : nullptr,
        .pStencilAttachment = has_stencil_attachment ? &pass.m_stencil_attachment_info : nullptr,
    });
}

void RenderGraph::record_command_buffer_for_pass(const CommandBuffer &cmd_buf, GraphicsPass &pass) {
    cmd_buf.set_suboperation_debug_name("|Pass:" + pass.m_name);
    // Start a new debug label for this graphics pass (visible in graphics debuggers like RenderDoc)
    cmd_buf.begin_debug_label_region(pass.m_name, pass.m_debug_label_color);

    // Fill the VKRenderingInfo of the graphics pass
    fill_graphics_pass_rendering_info(pass);

    // Start dynamic rendering with the compiled rendering info
    cmd_buf.begin_rendering(pass.m_rendering_info);

    // Call the command buffer recording function of this graphics pass. In this function, the actual rendering takes
    // place: the programmer binds pipelines, descriptor sets, buffers, and calls Vulkan commands. Note that rendergraph
    // does not bind any pipelines, descriptor sets, or buffers automatically!
    std::invoke(pass.m_on_record_cmd_buffer, cmd_buf);

    // End dynamic rendering
    cmd_buf.end_rendering();
    // End the debug label for this graphics pass
    cmd_buf.end_debug_label_region();
}

void RenderGraph::render() {
    // Acquire the next swapchain image index
    m_swapchain.acquire_next_image_index();

    // TODO: Refactor this into .exec();
    const auto &cmd_buf = m_device.request_command_buffer("RenderGraph::render");

    // TODO: Is this correct?
    //
    // Transform the image layout of the swapchain (it comes in undefined layout after presenting)
    cmd_buf.change_image_layout(m_swapchain.m_current_img, VK_IMAGE_LAYOUT_UNDEFINED,
                                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // Call the command buffer recording function of every pass
    for (auto &pass : m_graphics_passes) {
        record_command_buffer_for_pass(cmd_buf, *pass);
    }

    // TODO: Is this correct?
    //
    // Change the layout of the swapchain image to make it ready for presenting
    cmd_buf.change_image_layout(m_swapchain.m_current_img, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    // TODO: Further abstract this away?
    const std::array<VkPipelineStageFlags, 1> stage_mask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    cmd_buf.submit_and_wait(wrapper::make_info<VkSubmitInfo>({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_swapchain.image_available_semaphore(),
        .pWaitDstStageMask = stage_mask.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &cmd_buf.m_cmd_buf,
    }));
    m_swapchain.present();
}

void RenderGraph::reset() {
    // TODO: Implement me!
}

void RenderGraph::update_buffers() {
    m_device.execute("RenderGraph::update_buffers", [&](const CommandBuffer &cmd_buf) {
        for (const auto &buffer : m_buffers) {
            if (buffer->m_update_requested) {
                cmd_buf.set_suboperation_debug_name("|Buffer|Destroy:" + buffer->m_name);
                buffer->destroy();
                cmd_buf.set_suboperation_debug_name("|Buffer|Update:" + buffer->m_name);
                std::invoke(buffer->m_on_update);
                cmd_buf.set_suboperation_debug_name("|Buffer|Create:" + buffer->m_name);
                buffer->create(cmd_buf);
            }
        }
    });
}

void RenderGraph::update_descriptor_sets() {
    for (const auto &descriptor : m_resource_descriptors) {
        // Call descriptor set builder function for each descriptor
        std::invoke(std::get<2>(descriptor), m_descriptor_set_update_builder);
    }
}

void RenderGraph::update_textures() {
    m_device.execute("RenderGraph::update_textures", [&](const CommandBuffer &cmd_buf) {
        for (const auto &texture : m_textures) {
            // Only for dynamic textures m_on_lambda which is not std::nullopt
            if (texture->m_on_update) {
                if (texture->m_update_requested) {
                    cmd_buf.set_suboperation_debug_name("|Texture|Destroy:" + texture->m_name);
                    texture->destroy();
                    cmd_buf.set_suboperation_debug_name("|Texture|Update:" + texture->m_name);
                    std::invoke(texture->m_on_update.value());
                    cmd_buf.set_suboperation_debug_name("|Texture|Create:" + texture->m_name);
                    texture->create();
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
}

} // namespace inexor::vulkan_renderer::render_graph
