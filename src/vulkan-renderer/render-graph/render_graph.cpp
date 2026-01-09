#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include <functional>
#include <unordered_set>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(Device &device, const PipelineCache &pipeline_cache)
    : m_device(device), m_descriptor_set_allocator(device), m_write_descriptor_set_builder(device),
      m_graphics_pipeline_builder(device, pipeline_cache), m_descriptor_set_layout_builder(device) {}

std::weak_ptr<Buffer> RenderGraph::add_buffer(std::string name, const BufferType type,
                                              std::function<void()> on_update) {
    return m_buffers.emplace_back(std::make_shared<Buffer>(m_device, std::move(name), type, std::move(on_update)));
}

std::weak_ptr<GraphicsPass> RenderGraph::add_graphics_pass(std::shared_ptr<GraphicsPass> graphics_pass) {
    return m_graphics_passes.emplace_back(std::move(graphics_pass));
}

void RenderGraph::add_graphics_pipeline(OnCreateGraphicsPipeline on_create_graphics_pipeline) {
    m_graphics_pipeline_create_functions.emplace_back(std::move(on_create_graphics_pipeline));
}

void RenderGraph::allocate_descriptor_sets() {
    for (const auto &descriptor : m_resource_descriptors) {
        std::invoke(std::get<1>(descriptor), m_descriptor_set_allocator);
    }
}

void RenderGraph::add_resource_descriptor(OnBuildDescriptorSetLayout on_build_descriptor_set_layout,
                                          OnAllocateDescriptorSet on_allocate_descriptor_set,
                                          OnBuildWriteDescriptorSets on_update_descriptor_set) {
    m_resource_descriptors.emplace_back(std::move(on_build_descriptor_set_layout),
                                        std::move(on_allocate_descriptor_set), std::move(on_update_descriptor_set));
}

std::weak_ptr<Texture> RenderGraph::add_texture(std::string name, const TextureUsage usage, const VkFormat format,
                                                const std::uint32_t width, const std::uint32_t height,
                                                const std::uint32_t channels, const VkSampleCountFlagBits sample_count,
                                                std::function<void()> on_update) {
    return m_textures.emplace_back(std::make_shared<Texture>(m_device, std::move(name), usage, format, width, height,
                                                             channels, sample_count, std::move(on_update)));
}

void RenderGraph::create_descriptor_set_layouts() {
    for (const auto &descriptor : m_resource_descriptors) {
        std::invoke(std::get<0>(descriptor), m_descriptor_set_layout_builder);
    }
}

void RenderGraph::create_graphics_pipelines() {
    for (const auto &create_func : m_graphics_pipeline_create_functions) {
        std::invoke(create_func, m_graphics_pipeline_builder);
    }
}

void RenderGraph::check_for_cycles() {
    // @TODO Implement!
}

void RenderGraph::collect_swapchain_img_available_semaphores() {
    m_swapchains_imgs_available.clear();
    // Use an std::unordered_set to make sure every swapchain image available semaphore is in there only once!
    std::unordered_set<VkSemaphore> unique_semaphores;
    for (const auto &pass : m_graphics_passes) {
        for (const auto &swapchain : pass->m_write_swapchains) {
            unique_semaphores.emplace(swapchain.first.lock()->image_available_semaphore());
        }
    }
    // Convert the unordered_set into the std::vector so we can pass it during command buffer submission
    m_swapchains_imgs_available = std::vector<VkSemaphore>(unique_semaphores.begin(), unique_semaphores.end());
}

void RenderGraph::compile() {
    check_for_cycles();
    sort_graphics_passes_by_order();
    update_buffers();
    update_textures();
    create_descriptor_set_layouts();
    allocate_descriptor_sets();
    // NOTE: Creating graphics pipelines requires us to know the corresponding pipeline layouts, which means descriptor
    // set layouts must be known! This means the descriptor management must become before creating graphics pipelines!
    create_graphics_pipelines();
    collect_swapchain_img_available_semaphores();
}

void RenderGraph::fill_graphics_pass_rendering_info(GraphicsPass &pass) {
    pass.reset_rendering_info();

    /// Fill the VkRenderingattachmentInfo for a color, depth, or stencil attachment
    /// @param write_attachment The attachment this graphics pass writes to
    /// @param clear_value The clear value
    auto fill_rendering_info_for_attachment = [&](const std::weak_ptr<Texture> &write_attachment,
                                                  const std::optional<VkClearValue> &clear_value) {
        const auto attachment = write_attachment.lock();
        auto get_image_layout = [&]() {
            switch (attachment->usage()) {
            case TextureUsage::COLOR_ATTACHMENT: {
                return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            }
            case TextureUsage::DEPTH_ATTACHMENT:
            case TextureUsage::STENCIL_ATTACHMENT: {
                return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            }
            default:
                return VK_IMAGE_LAYOUT_UNDEFINED;
            }
        };

        // TODO: Support MSAA again!
        return wrapper::make_info<VkRenderingAttachmentInfo>({
            // TODO: Implement m_current_img_view when double/triple buffering and do this on init, not per-frame?
            .imageView = attachment->image_view(),
            .imageLayout = get_image_layout(),
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = nullptr,
            .loadOp = clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_value.value_or(VkClearValue{}),
        });
    };

    // Step 1: Process all write attachments (color, depth, stencil) of the graphics pass into VkRenderingInfo
    for (const auto &write_attachment : pass.m_write_attachments) {
        // What type of attachment is this?
        const auto &attachment = write_attachment.first;
        const auto &clear_value = write_attachment.second;
        const auto rendering_info = fill_rendering_info_for_attachment(attachment, clear_value);

        switch (attachment.lock()->usage()) {
        case TextureUsage::COLOR_ATTACHMENT: {
            pass.m_color_attachments.push_back(rendering_info);
            break;
        }
        case TextureUsage::DEPTH_ATTACHMENT: {
            pass.m_depth_attachment = rendering_info;
            pass.m_has_depth_attachment = true;
            break;
        }
        case TextureUsage::STENCIL_ATTACHMENT: {
            pass.m_stencil_attachment = rendering_info;
            pass.m_has_stencil_attachment = true;
            break;
        }
        default:
            continue;
        }
    }

    /// Fill the VkRenderingAttachmentInfo for a swapchain
    /// @param write_swapchain The swapchain to which this graphics pass writes to
    /// @param clear_value The optional clear value for the swapchain image
    auto fill_write_info_for_swapchain = [&](const std::weak_ptr<Swapchain> &write_swapchain,
                                             const std::optional<VkClearValue> &clear_value) {
        // TODO: Support MSAA again!
        return wrapper::make_info<VkRenderingAttachmentInfo>({
            // TODO: Does this mean we can do this on init now? Not on a per-frame basis?
            .imageView = write_swapchain.lock()->current_swapchain_image_view(),
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = nullptr,
            .loadOp = clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_value.value_or(VkClearValue{}),
        });
    };

    // TODO: Step 2: Process all swapchain writes of the graphics pass into VkRenderingInfo
    for (const auto &write_swapchain : pass.m_write_swapchains) {
        const auto &swapchain = write_swapchain.first.lock();
        const auto &clear_value = write_swapchain.second;
        pass.m_color_attachments.push_back(fill_write_info_for_swapchain(swapchain, clear_value));
    }

    // TODO: If a pass has multiple color attachments those are multiple swapchains, does that mean we must group
    // rendering by swapchains because there is no guarantee that they all have the same swapchain extent?

    // Step 3: Fill the rendering info
    pass.m_rendering_info = wrapper::make_info<VkRenderingInfo>({
        .renderArea =
            {
                .extent = pass.m_extent,
            },
        .layerCount = 1,
        .colorAttachmentCount = static_cast<std::uint32_t>(pass.m_color_attachments.size()),
        .pColorAttachments = (pass.m_color_attachments.size() > 0) ? pass.m_color_attachments.data() : nullptr,
        .pDepthAttachment = pass.m_has_depth_attachment ? &pass.m_depth_attachment : nullptr,
        .pStencilAttachment = pass.m_has_stencil_attachment ? &pass.m_stencil_attachment : nullptr,
    });
}

void RenderGraph::record_command_buffer_for_pass(const CommandBuffer &cmd_buf, GraphicsPass &pass) {
    cmd_buf.set_suboperation_debug_name("[Pass:" + pass.m_name + "]");
    // Start a new debug label for this graphics pass (visible in graphics debuggers like RenderDoc)
    cmd_buf.begin_debug_label_region(pass.m_name, pass.m_debug_label_color);

    // Fill the VKRenderingInfo of the graphics pass
    fill_graphics_pass_rendering_info(pass);

    // If there are writes to swapchains, the image layout of the swapchain must be changed because it comes back in
    // undefined layout after presenting
    for (const auto &swapchain : pass.m_write_swapchains) {
        // NOTE: We don't need to check if the previous pass wrote to this swapchain because we already check in the
        // code below if the next pass (if any) will write to this swapchain again, so if the last pass already wrote to
        // this swapchain, calling change_image_layout_to_prepare_for_rendering will not do anything.
        swapchain.first.lock()->change_image_layout_to_prepare_for_rendering(cmd_buf);
    }

    // Start dynamic rendering with the compiled rendering info
    cmd_buf.begin_rendering(pass.m_rendering_info);

    // NOTE: Pipeline barriers must not be placed inside of dynamic rendering instances!

    // Call the command buffer recording function of this graphics pass. In this function, the actual rendering takes
    // place: the programmer binds pipelines, descriptor sets, buffers, and calls Vulkan commands. Note that rendergraph
    // does not bind any pipelines, descriptor sets, or buffers automatically!
    std::invoke(pass.m_on_record_cmd_buffer, cmd_buf);

    // End dynamic rendering
    cmd_buf.end_rendering();

    // TODO: Not only check for next pass, but check all following passes! For example if pass A writes to swapchain,
    // pass B doesn't, but pass C does again, we would unnecessarily transition between A and B, then B and C, and after
    // C again!

    // Change the swapchain image layouts to prepare the swapchains for presenting
    for (const auto &swapchain : pass.m_write_swapchains) {
        // TODO: Check if next pass (if any) writes to that swapchain as well!
        bool next_pass_writes_to_this_swapchain = false;
        if (!pass.m_next_pass.expired()) {
            const auto &next_pass = pass.m_next_pass.lock();
            for (const auto &next_pass_write_swapchain : next_pass->m_write_swapchains) {
                if (next_pass_write_swapchain.first.lock() == swapchain.first.lock()) {
                    next_pass_writes_to_this_swapchain = true;
                }
            }
        }
        // NOTE: If the next pass writes to this swapchain as well, we can keep it in the current image layout.
        // Only otherwise, we change the image layout to prepare the swapchain image for presenting.
        if (!next_pass_writes_to_this_swapchain) {
            swapchain.first.lock()->change_image_layout_to_prepare_for_presenting(cmd_buf);
        }
    }

    // End the debug label for this graphics pass
    cmd_buf.end_debug_label_region();
}

void RenderGraph::render() {
    update_buffers();
    update_textures();
    // @TODO Only call if any data changed and try to accumulate write descriptor sets
    update_write_descriptor_sets();

    // @TODO Use std::source_location for naming!
    // @TODO: Implement random_debug_label_color?
    m_device.execute(
        "RenderGraph::render", VK_QUEUE_GRAPHICS_BIT, DebugLabelColor::CYAN,
        [&](const CommandBuffer &cmd_buf) {
            // Call the command buffer recording function of every graphics pass
            for (const auto &pass : m_graphics_passes) {
                record_command_buffer_for_pass(cmd_buf, *pass);
            }
        },
        m_swapchains_imgs_available);

    // @TODO Implement!
}

void RenderGraph::reset() {
    m_buffers.clear();
    m_textures.clear();
    m_graphics_passes.clear();
    m_resource_descriptors.clear();
}

void RenderGraph::sort_graphics_passes_by_order() {
    // @TODO Implement!
}

void RenderGraph::update_buffers() {
    // Check if there is any update required
    bool any_update_required = false;
    for (const auto &buffer : m_buffers) {
        std::invoke(buffer->m_on_check_for_update);
        // TODO: A command buffer copy command is only required if the memory is not updated through std::memcpy!
        if (buffer->m_update_requested) {
            any_update_required = true;
        }
    }

    // Only start recording and submitting a command buffer on transfer queue if any update is required
    // TODO: Use dedicated transfer queue instead of transfer queue for buffer updates!
    if (any_update_required) {
        m_device.execute("[RenderGraph::update_buffers]", VK_QUEUE_GRAPHICS_BIT, DebugLabelColor::MAGENTA,
                         [&](const CommandBuffer &cmd_buf) {
                             for (const auto &buffer : m_buffers) {
                                 if (buffer->m_update_requested) {
                                     cmd_buf.set_suboperation_debug_name("[Buffer|Destroy:" + buffer->name() + "]");
                                     buffer->destroy_all();
                                     cmd_buf.set_suboperation_debug_name("[Buffer|Update:" + buffer->name() + "]");
                                     buffer->create(cmd_buf);
                                 }
                             }
                         });
    }
    // NOTE: For the "else" case: We can't insert a debug label here telling us that there are no buffer updates
    // required because that command itself would require a command buffer to be in recording state
}

void RenderGraph::update_textures() {
    // Check if there is any update required
    bool any_update_required = false;
    for (const auto &texture : m_textures) {
        // Check if this texture needs an update
        if (texture->usage() == TextureUsage::COLOR_ATTACHMENT) {
            texture->m_on_check_for_updates();
        }
        if (texture->m_update_requested) {
            any_update_required = true;
        }
    }
    // Only start recording and submitting a command buffer if any update is required
    // TODO: Use dedicated transfer queue instead of graphics queue for texture updates!
    if (any_update_required) {
        m_device.execute("[RenderGraph::update_textures]", VK_QUEUE_GRAPHICS_BIT, DebugLabelColor::LIME,
                         [&](const CommandBuffer &cmd_buf) {
                             for (const auto &texture : m_textures) {
                                 if (texture->m_update_requested) {
                                     // TODO: Remove set_suboperation_debug_name entirely and use debug label?
                                     cmd_buf.set_suboperation_debug_name("[Texture|Destroy:" + texture->name() + "]");
                                     texture->destroy();
                                     cmd_buf.set_suboperation_debug_name("[Texture|Create:" + texture->name() + "]");
                                     texture->create();
                                     texture->update(cmd_buf);
                                 }
                             }
                         });
    }
    // NOTE: For the "else" case: We can't insert a debug label here telling us that there are no buffer updates
    // required because that command itself would require a command buffer to be in recording state
}

void RenderGraph::update_write_descriptor_sets() {
    m_write_descriptor_sets.clear();
    // NOTE: We don't reserve memory for the std::vector because we don't know how many write descriptor sets will exist
    // in total (each resource descriptor can have an arbitrary number of write descriptor sets). Because we call
    // update_write_descriptor_sets() only once during rendergraph compilation, this is not a problem.
    for (const auto &descriptor : m_resource_descriptors) {
        // Call descriptor set builder function (OnBuildWriteDescriptorSets) for each descriptor
        auto write_descriptor_sets = std::invoke(std::get<2>(descriptor), m_write_descriptor_set_builder);
        // Store the results of the function that was called
        std::move(write_descriptor_sets.begin(), write_descriptor_sets.end(),
                  std::back_inserter(m_write_descriptor_sets));
    }
    // NOTE: We batch all descriptor set updates into one function call for optimal performance
    m_device.update_descriptor_sets(m_write_descriptor_sets);
}

} // namespace inexor::vulkan_renderer::render_graph
