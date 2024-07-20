#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <unordered_set>

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(Device &device)
    : m_device(device), m_graphics_pipeline_builder(device), m_descriptor_set_layout_builder(device),
      m_descriptor_set_allocator(m_device), m_write_descriptor_set_builder(m_device) {}

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
                                          OnBuildWriteDescriptorSets on_update_descriptor_set) {
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

void RenderGraph::collect_swapchain_image_available_semaphores() {
    m_swapchains_imgs_available.clear();
    for (const auto &pass : m_graphics_passes) {
        for (const auto &swapchain : pass->m_write_swapchains) {
            m_swapchains_imgs_available.push_back(swapchain.first.lock()->m_img_available->m_semaphore);
        }
    }
}

void RenderGraph::compile() {
    // TODO: What needs to be re-done when swapchain is recreated?
    validate_render_graph();
    determine_pass_order();
    update_buffers();
    create_textures();
    create_descriptor_set_layouts();
    allocate_descriptor_sets();
    update_write_descriptor_sets();
    create_graphics_passes();
    create_graphics_pipelines();
    collect_swapchain_image_available_semaphores();
    check_for_cycles();
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
    m_device.execute("[RenderGraph::create_textures]", DebugLabelColor::BLUE, [&](const CommandBuffer &cmd_buf) {
        for (const auto &texture : m_textures) {
            if (texture->m_on_init) {
                cmd_buf.set_suboperation_debug_name("[Texture|Initialize]:" + texture->m_name + "]");
                std::invoke(texture->m_on_init.value());
            }
            cmd_buf.set_suboperation_debug_name("[Texture|Create]:" + texture->m_name + "]");
            texture->create();
            if (texture->m_usage == TextureUsage::NORMAL) {
                cmd_buf.set_suboperation_debug_name("[Texture|Update]:" + texture->m_name + "]");
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
    pass.reset_rendering_info();

    ///
    ///
    ///
    auto fill_rendering_info_for_attachment = [&](const std::weak_ptr<Texture> &write_attachment,
                                                  const std::optional<VkClearValue> &clear_value) {
        const auto &attachment = write_attachment.lock();
        auto get_image_layout = [&]() {
            switch (attachment->m_usage) {
            case TextureUsage::COLOR_ATTACHMENT:
            case TextureUsage::NORMAL: {
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
            .imageView = attachment->m_img->m_img_view,
            .imageLayout = get_image_layout(),
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = nullptr,
            .loadOp = clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_value ? clear_value.value() : VkClearValue{},
        });
    };

    // Step 1: Process all write attachments (color, depth, stencil) of the graphics pass into VkRenderingInfo
    for (const auto &write_attachment : pass.m_write_attachments) {
        // What type of attachment is this?
        const auto &attachment = write_attachment.first;
        const auto &clear_value = write_attachment.second;

        switch (attachment.lock()->m_usage) {
        case TextureUsage::COLOR_ATTACHMENT:
        case TextureUsage::NORMAL: {
            pass.m_color_attachments.push_back(fill_rendering_info_for_attachment(attachment, clear_value));
            break;
        }
        case TextureUsage::DEPTH_ATTACHMENT: {
            pass.m_depth_attachment = fill_rendering_info_for_attachment(attachment, clear_value);
            pass.m_has_depth_attachment = true;
            break;
        }
        case TextureUsage::STENCIL_ATTACHMENT: {
            pass.m_stencil_attachment = fill_rendering_info_for_attachment(attachment, clear_value);
            pass.m_has_stencil_attachment = true;
            break;
        }
        default:
            continue;
        }
    }

    ///
    ///
    ///
    auto fill_write_info_for_swapchain = [&](const std::weak_ptr<Swapchain> &write_swapchain,
                                             const std::optional<VkClearValue> &clear_value) {
        // TODO: Support MSAA again!
        return wrapper::make_info<VkRenderingAttachmentInfo>({
            .imageView = write_swapchain.lock()->m_current_img_view,
            .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .resolveMode = VK_RESOLVE_MODE_NONE,
            .resolveImageView = nullptr,
            .loadOp = clear_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clear_value ? clear_value.value() : VkClearValue{},
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

    // TODO: Only change image layout of swapchain if previous pass did not already do this!

    // If there are writes to swapchains, the image layout of the swapchain must be changed because it comes back in
    // undefined layout after presenting
    for (const auto &swapchain : pass.m_write_swapchains) {
        cmd_buf.insert_debug_label("Changing Swapchain image layout", get_debug_label_color(DebugLabelColor::GREEN));
        cmd_buf.change_image_layout(swapchain.first.lock()->m_current_img, VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    // NOTE: Pipeline barriers must not be placed inside of dynamic rendering instances, which means we must change the
    // image layout of all swapchains we write to before we call begin_rendering and then again after we call
    // end_rendering.
    // ----------------------------------------------------------------------------------------------------------------
    // Start dynamic rendering with the compiled rendering info
    cmd_buf.begin_rendering(pass.m_rendering_info);
    // Call the command buffer recording function of this graphics pass. In this function, the actual rendering takes
    // place: the programmer binds pipelines, descriptor sets, buffers, and calls Vulkan commands. Note that rendergraph
    // does not bind any pipelines, descriptor sets, or buffers automatically!
    std::invoke(pass.m_on_record_cmd_buffer, cmd_buf);
    // End dynamic rendering
    cmd_buf.end_rendering();
    // ----------------------------------------------------------------------------------------------------------------

    // TODO: Only change image layout of swapchain if previous pass did not already do this!

    // Change the swapchain image layouts to prepare the swapchains for presenting
    for (const auto &swapchain : pass.m_write_swapchains) {
        cmd_buf.insert_debug_label("Changing Swapchain image layout", get_debug_label_color(DebugLabelColor::GREEN));
        cmd_buf.change_image_layout(swapchain.first.lock()->m_current_img, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    // End the debug label for this graphics pass
    cmd_buf.end_debug_label_region();
}

void RenderGraph::render() {
    update_buffers();
    update_textures();
    update_write_descriptor_sets();
    // NOTE: We only need to call update_write_descriptor_sets() once in rendergraph compilation, not every frame!

    // So if we are writing to multiple swapchains in this pass, we must wait for every swapchains
    // semaphore!
    m_device.execute(
        "[RenderGraph::render]", DebugLabelColor::CYAN,
        [&](const CommandBuffer &cmd_buf) {
            // Call the command buffer recording function of every graphics pass
            for (auto &pass : m_graphics_passes) {
                record_command_buffer_for_pass(cmd_buf, *pass);
            }
        },
        m_swapchains_imgs_available);
}

void RenderGraph::reset() {
    // TODO: Implement me!
}

void RenderGraph::update_buffers() {
    // Check if there is any update required
    bool any_update_required = false;
    for (const auto &buffer : m_buffers) {
        std::invoke(buffer->m_on_check_for_update);
        if (buffer->m_update_requested) {
            any_update_required = true;
        }
    }
    // Only start recording and submitting a command buffer if any update is required
    if (any_update_required) {
        m_device.execute("[RenderGraph::update_buffers]", DebugLabelColor::MAGENTA, [&](const CommandBuffer &cmd_buf) {
            for (const auto &buffer : m_buffers) {
                if (buffer->m_update_requested) {
                    cmd_buf.set_suboperation_debug_name("[Buffer|Destroy:" + buffer->m_name + "]");
                    buffer->destroy_all();
                    cmd_buf.set_suboperation_debug_name("[Buffer|Update:" + buffer->m_name + "]");
                    buffer->create(cmd_buf);
                }
            }
        });
    }
}

void RenderGraph::update_textures() {
    // Check if there is any update required
    bool any_update_required = false;
    for (const auto &texture : m_textures) {
        // Only for dynamic textures m_on_lambda which is not std::nullopt
        if (texture->m_on_check_for_updates) {
            std::invoke(texture->m_on_check_for_updates.value());
            if (texture->m_update_requested) {
                any_update_required = true;
            }
        }
    }
    // Only start recording and submitting a command buffer if any update is required
    if (any_update_required) {
        m_device.execute("[RenderGraph::update_textures]", DebugLabelColor::LIME, [&](const CommandBuffer &cmd_buf) {
            for (const auto &texture : m_textures) {
                // Only for dynamic textures m_on_lambda which is not std::nullopt
                if (texture->m_update_requested) {
                    cmd_buf.set_suboperation_debug_name("[Texture|Destroy:" + texture->m_name + "]");
                    texture->destroy();
                    cmd_buf.set_suboperation_debug_name("[Texture|Update:" + texture->m_name + "]");
                    std::invoke(texture->m_on_check_for_updates.value());
                    cmd_buf.set_suboperation_debug_name("[Texture|Create:" + texture->m_name + "]");
                    texture->create();
                }
            }
        });
    }
}

void RenderGraph::update_write_descriptor_sets() {
    m_write_descriptor_sets.clear();
    // NOTE: We don't reserve memory for the vector because we don't know how many write descriptor sets will exist in
    // total. Because we call update_descriptor_sets() only once during rendergraph compilation, this is not a problem.
    for (const auto &descriptor : m_resource_descriptors) {
        // Call descriptor set builder function for each descriptor
        auto write_descriptor_sets = std::invoke(std::get<2>(descriptor), m_write_descriptor_set_builder);
        // Store the results of the function that was called
        std::move(write_descriptor_sets.begin(), write_descriptor_sets.end(),
                  std::back_inserter(m_write_descriptor_sets));
    }
    // NOTE: We batch all descriptor set updates into one function call for optimal performance
    vkUpdateDescriptorSets(m_device.device(), static_cast<std::uint32_t>(m_write_descriptor_sets.size()),
                           m_write_descriptor_sets.data(), 0, nullptr);
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
