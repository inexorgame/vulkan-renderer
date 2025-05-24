#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <unordered_set>

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::InexorException;
using wrapper::VulkanException;

RenderGraph::RenderGraph(Device &device, const PipelineCache &pipeline_cache)
    : m_device(device), m_graphics_pipeline_builder(device), m_pipeline_cache(pipeline_cache),
      m_descriptor_set_layout_builder(device), m_descriptor_set_allocator(m_device),
      m_write_descriptor_set_builder(m_device) {}

void RenderGraph::allocate_descriptor_sets() {
    for (const auto &render_module : m_render_modules) {
        render_module->allocate_descriptor_sets(m_descriptor_set_allocator);
    }
}

void RenderGraph::check_for_cycles() {
    // TODO: Implement
}

void RenderGraph::collect_swapchain_image_available_semaphores() {
#if 0
    m_swapchains_imgs_available.clear();
    // Use an std::unordered_set to make sure every swapchain image available semaphore is in there only once!
    std::unordered_set<VkSemaphore> unique_semaphores;
    for (const auto &pass : m_graphics_passes) {
        for (const auto &swapchain : pass->m_write_swapchains) {
            unique_semaphores.emplace(swapchain.first.lock()->m_img_available->m_semaphore);
        }
    }
    // Convert the unordered_set into the std::vector so we can pass it in command buffer submission
    m_swapchains_imgs_available = std::vector<VkSemaphore>(unique_semaphores.begin(), unique_semaphores.end());
#endif
}

void RenderGraph::compile() {
    // TODO: What needs to be re-done when swapchain is recreated?
    check_for_cycles();
    determine_pass_order();
    update_buffers();
    update_textures();
    create_descriptor_set_layouts();
    allocate_descriptor_sets();
    create_graphics_pipelines();
    collect_swapchain_image_available_semaphores();
}

void RenderGraph::create_descriptor_set_layouts() {
    for (const auto &render_module : m_render_modules) {
        render_module->setup_descriptor_set_layouts(m_descriptor_set_layout_builder);
    }
}

void RenderGraph::create_graphics_pipelines() {
    // TODO: Implement a GraphicsPipelineCreateBuilder

    // For performance, we batch it all into one std::vector and call vkCreateGraphicsPipelines once
    std::vector<VkGraphicsPipelineCreateInfo> graphics_pipeline_create_infos;
    // Iterate through all render modules
    for (const auto &render_module : m_render_modules) {
        // Iterate through all graphics pipelines of the render module
        for (const auto &pipeline : render_module->m_graphics_pipelines) {
            graphics_pipeline_create_infos.push_back(pipeline.m_pipeline_ci);
        }
    }
    // The pipelines to create
    std::vector<VkPipeline> graphics_pipelines(graphics_pipeline_create_infos.size());
    // Create the graphics pipelines in one batched call to vkCreateGraphicsPipelines
    if (const auto result = vkCreateGraphicsPipelines(
            m_device.device(), m_pipeline_cache.get_handle(), graphics_pipeline_create_infos.size(),
            graphics_pipeline_create_infos.data(), nullptr, graphics_pipelines.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateGraphicsPipelines failed!", result);
    }
    // Now we created all graphics pipelines, we need to put them back into the wrappers
    std::size_t pipeline_index = 0;
    // Iterate through all render modules
    for (const auto &render_module : m_render_modules) {
        // Iterate through all graphics pipelines of the render module
        for (auto &pipeline : render_module->m_graphics_pipelines) {
            pipeline.m_pipeline = graphics_pipelines[pipeline_index];
            pipeline_index++;
        }
    }
}

void RenderGraph::determine_pass_order() {
    // TODO: Implement
}

void RenderGraph::fill_graphics_pass_rendering_info(GraphicsPass &pass) {
#if 0
    pass.reset_rendering_info();

    /// Fill the VkRenderingattachmentInfo for a color, depth, or stencil attachment
    /// @param write_attachment The attachment this graphics pass writes to
    /// @param clear_value The clear value
    auto fill_rendering_info_for_attachment = [&](const std::weak_ptr<Texture> &write_attachment,
                                                  const std::optional<VkClearValue> &clear_value) {
        const auto &attachment = write_attachment.lock();
        auto get_image_layout = [&]() {
            switch (attachment->m_usage) {
            case TextureUsage::COLOR_ATTACHMENT:
            case TextureUsage::DEFAULT: {
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
            .imageView = attachment->m_image->m_img_view,
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

        switch (attachment.lock()->m_usage) {
        case TextureUsage::COLOR_ATTACHMENT:
        case TextureUsage::DEFAULT: {
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
            .imageView = write_swapchain.lock()->m_current_swapchain_img_view,
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
#endif
}

void RenderGraph::record_command_buffer_for_pass(const CommandBuffer &cmd_buf, const GraphicsPass &pass) {

#if 0
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
#endif
}

void RenderGraph::record_and_submit_command_buffers() {
    // Iterate through all render modules
    for (const auto &render_module : m_render_modules) {
        // Iterate through every graphics pass of the current render module
        for (const auto &pass : render_module->m_graphics_passes) {
            m_device.execute(pass.m_name, VK_QUEUE_GRAPHICS_BIT, DebugLabelColor::CYAN,
                             [&](const CommandBuffer &cmd_buf) {
                                 // Record one command buffer for every pass in every render module
                                 record_command_buffer_for_pass(cmd_buf, pass);
                             });
        }
    }
    // TODO: Wait on m_swapchains_imgs_available...
}

void RenderGraph::render() {
    update_buffers();
    update_textures();
    update_write_descriptor_sets();
    record_and_submit_command_buffers();
}

void RenderGraph::reset() {
    // TODO: Implement me!
}

void RenderGraph::update_buffers() {
    // NOTE: Even if a buffer update can be done via vmaCopyMemoryToAllocation, we still need a command buffer for
    // placing the pipeline barrier after the write operation! We also need the command buffer in case we need to update
    // a buffer with a staging buffer and copy command of course, and such a case required barriers as well.

    // NOTE: If there are no buffer updates required, an empty command buffer will be recorded, which is valid too.

    // TODO: Update by using transfer queue (this requires transfer of ownership though)
    m_device.execute("RenderGraph::update_buffers", VK_QUEUE_GRAPHICS_BIT, DebugLabelColor::GREEN,
                     [&](const CommandBuffer &cmd_buf) {
                         // Iterate through all render modules in the rendergraph
                         for (const auto &render_module : m_render_modules) {
                             // Update all buffers of this render module
                             render_module->update_buffers();
                             // Iterate through all buffers of the render module and perform update if required
                             for (const auto &buffer : render_module->m_buffers) {
                                 // Does this buffer require an update?
                                 if (buffer->is_update_requested()) {
                                     buffer->update(cmd_buf);
                                 }
                             }
                         }
                         // TODO: Batch buffer memory barriers here!
                     });
}

void RenderGraph::update_textures() {
    // TODO: Update by using transfer queue (this requires transfer of ownership though)
    m_device.execute("RenderGraph::update_textures", VK_QUEUE_GRAPHICS_BIT, DebugLabelColor::GREEN,
                     [&](const CommandBuffer &cmd_buf) {
                         // Iterate through all render modules in the rendergraph
                         for (const auto &render_module : m_render_modules) {
                             // Update all textures of the render module
                             render_module->update_textures();
                             // TODO: Move responsibility from rendergraph to update_textures?
                             for (const auto &texture : render_module->m_textures) {
                                 if (texture->is_update_requested()) {
                                     texture->update(cmd_buf);
                                 }
                             }
                         }
                     });
}

void RenderGraph::update_write_descriptor_sets() {
    // Iterate through all render modules in the rendergraph
    for (const auto &render_module : m_render_modules) {
        // Update the descriptor sets of this render module
        render_module->update_descriptor_sets(m_write_descriptor_set_builder);
    }
    // NOTE: We batch all descriptor set updates off all render modules into one std::vector of write descriptor
    // sets and call vkUpdateDescriptorSets only once per frame with it for improved performance
    m_device.update_descriptor_sets(m_write_descriptor_set_builder.build());
}

} // namespace inexor::vulkan_renderer::render_graph
