#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <unordered_map>

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(Device &device, Swapchain &swapchain)
    : m_device(device), m_swapchain(swapchain), m_graphics_pipeline_builder(device),
      m_descriptor_set_layout_cache(device), m_descriptor_set_layout_builder(device, m_descriptor_set_layout_cache),
      m_descriptor_set_allocator(m_device), m_descriptor_set_update_builder(m_device) {}

void RenderGraph::add_graphics_pass(GraphicsPassCreateFunction on_pass_create) {
    m_graphics_pass_create_functions.emplace_back(std::move(on_pass_create));
}

void RenderGraph::add_graphics_pipeline(GraphicsPipelineCreateFunction on_pipeline_create) {
    m_pipeline_create_functions.emplace_back(std::move(on_pipeline_create));
}

std::shared_ptr<Buffer>
RenderGraph::add_buffer(std::string buffer_name, const BufferType buffer_type, std::function<void()> on_update) {
    m_buffers.emplace_back(
        std::make_shared<Buffer>(m_device, std::move(buffer_name), buffer_type, std::move(on_update)));
    return m_buffers.back();
}

void RenderGraph::allocate_descriptor_sets() {
    for (const auto &descriptor : m_resource_descriptors) {
        // Call on_update_descriptor_set for each descriptor
        std::invoke(std::get<1>(descriptor), m_descriptor_set_allocator);
    }
}

void RenderGraph::add_resource_descriptor(
    std::function<void(DescriptorSetLayoutBuilder &)> on_create_descriptor_set_layout,
    std::function<void(DescriptorSetAllocator &)> on_allocate_descriptor_set,
    std::function<void(DescriptorSetUpdateBuilder &)> on_update_descriptor_set) {
    // NOTE: emplace_back directly constructs the tuple in place, no need for push_back and std::make_tuple
    m_resource_descriptors.emplace_back(std::move(on_create_descriptor_set_layout),
                                        std::move(on_allocate_descriptor_set), std::move(on_update_descriptor_set));
}

std::shared_ptr<Texture>
RenderGraph::add_texture(std::string texture_name, const TextureUsage usage, const VkFormat format) {
    m_textures.emplace_back(std::make_shared<Texture>(m_device, std::move(texture_name), usage, format));
    return m_textures.back();
}

std::shared_ptr<Texture> RenderGraph::add_texture(std::string texture_name,
                                                  const TextureUsage usage,
                                                  std::optional<std::function<void()>> on_init,
                                                  std::optional<std::function<void()>> on_update) {
    m_textures.emplace_back(
        std::make_shared<Texture>(m_device, std::move(texture_name), usage, std::move(on_init), std::move(on_update)));
    return m_textures.back();
}

void RenderGraph::check_for_cycles() {
    // TODO: Implement me!
}

void RenderGraph::compile() {
    validate_render_graph();
    determine_pass_order();
    create_buffers();
    create_textures();
    create_descriptor_set_layouts();
    allocate_descriptor_sets();
    update_descriptor_sets();
    create_graphics_passes();
    create_graphics_pipelines();
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
    m_graphics_pipelines.clear();
    m_graphics_pipelines.reserve(m_pipeline_create_functions.size());
    for (const auto &create_func : m_pipeline_create_functions) {
        m_graphics_pipelines.emplace_back(create_func(m_graphics_pipeline_builder));
    }
}

void RenderGraph::create_textures() {
    /// The following code should not be part of texture wrapper because its only purpose is to fill VkImageCreateInfo
    /// and VkImageViewCreateInfo in the code below to make it shorter.
    auto fill_image_ci = [&](const VkFormat format, const VkImageUsageFlags image_usage,
                             const VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT) {
        return wrapper::make_info<VkImageCreateInfo>({
            .imageType = VK_IMAGE_TYPE_2D,
            .format = format,
            .extent =
                {
                    .width = static_cast<std::uint32_t>(m_swapchain.extent().width),
                    .height = static_cast<std::uint32_t>(m_swapchain.extent().height),
                    .depth = 1,
                },
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = sample_count,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = image_usage,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        });
    };

    auto fill_image_view_ci = [&](const VkFormat format, const VkImageAspectFlags aspect_flags) {
        return wrapper::make_info<VkImageViewCreateInfo>({
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = format,
            .subresourceRange =
                {
                    .aspectMask = aspect_flags,
                    .baseMipLevel = 0,
                    .levelCount = 1,
                    .baseArrayLayer = 0,
                    .layerCount = 1,
                },
        });
    };

    m_device.execute("[RenderGraph::create_textures]", [&](const CommandBuffer &cmd_buf) {
        for (const auto &texture : m_textures) {
            switch (texture->m_usage) {
            case TextureUsage::NORMAL: {
                if (texture->m_on_init) {
                    std::invoke(texture->m_on_init.value());
                    // TODO: How to unify ->create()?
                    texture->create();
                    texture->update(cmd_buf);
                }
                break;
            }
            case TextureUsage::DEPTH_STENCIL_BUFFER: {
                texture->create(
                    fill_image_ci(texture->m_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT),
                    fill_image_view_ci(texture->m_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
                break;
            }
            case TextureUsage::BACK_BUFFER: {
                texture->create(fill_image_ci(texture->m_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT),
                                fill_image_view_ci(texture->m_format, VK_IMAGE_ASPECT_COLOR_BIT));
                break;
            }
            case TextureUsage::MSAA_BACK_BUFFER: {
                texture->create(fill_image_ci(texture->m_format, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                              // TODO: Expose this as a parameter
                                              // NOTE: We use the highest available sample count for MSAA
                                              m_device.get_max_usable_sample_count()),
                                fill_image_view_ci(texture->m_format, VK_IMAGE_ASPECT_COLOR_BIT));
                break;
            }
            case TextureUsage::MSAA_DEPTH_STENCIL_BUFFER: {
                texture->create(
                    fill_image_ci(texture->m_format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                  // TODO: Expose this as a parameter
                                  // NOTE: We use the highest available sample count for MSAA
                                  m_device.get_max_usable_sample_count()),
                    fill_image_view_ci(texture->m_format, VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT));
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

void RenderGraph::record_command_buffer_for_pass(const CommandBuffer &cmd_buf,
                                                 const GraphicsPass &pass,
                                                 const bool is_first_pass,
                                                 const bool is_last_pass,
                                                 const std::uint32_t img_index) {
    // TODO: Expose pass_index as parameter?
    // TODO: Remove img_index and implement swapchain.get_current_image()
    // TODO: Or do we need the image index for buffers? (We want to automatically double or triple buffer them)

    // Start a new debug label for this graphics pass (visible in graphics debuggers like RenderDoc)
    // TODO: Generate color gradient?
    cmd_buf.begin_debug_label_region(pass.m_name, {1.0f, 0.0f, 0.0f, 1.0f});

    // If this is the first graphics pass, change the image layout of the swapchain image which comes back in undefined
    // image layout after presenting
    if (is_first_pass) {
        cmd_buf.change_image_layout(m_swapchain.image(img_index), VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    const auto color_attachment = wrapper::make_info<VkRenderingAttachmentInfo>({
        .imageView =
            (pass.m_enable_msaa) ? pass.m_msaa_color_attachment.lock()->m_img_view : m_swapchain.image_view(img_index),
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,
        .resolveImageView = m_swapchain.image_view(img_index),
        .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = (pass.m_clear_color_attachment) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = pass.m_clear_values.value(),
    });

    const auto depth_attachment = wrapper::make_info<VkRenderingAttachmentInfo>({
        .imageView =
            (pass.m_enable_msaa) ? pass.m_msaa_depth_attachment.lock()->m_img_view : m_swapchain.image_view(img_index),
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_MIN_BIT,
        .resolveImageView = pass.m_depth_attachment.lock()->m_img_view,
        .resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .loadOp = (pass.m_clear_stencil_attachment) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = pass.m_clear_values.value(),
    });

    // TODO: Implement stencil attachment

    const auto rendering_info = wrapper::make_info<VkRenderingInfo>({
        .renderArea =
            {
                .extent = m_swapchain.extent(),
            },
        .layerCount = 1,
        .colorAttachmentCount = 1, // TODO: Implement multiple color attachments
        .pColorAttachments = &color_attachment,
        .pDepthAttachment = &depth_attachment,
        // TODO: Implement stencil attachment
    });

    // Start dynamic rendering
    cmd_buf.begin_rendering(rendering_info);

    // Call the command buffer recording function of the graphics pass
    std::invoke(pass.m_on_record, cmd_buf);

    // End dynamic rendering
    cmd_buf.end_rendering();

    // If this is the last graphics pass, change the image layout of the swapchain image for presenting
    if (is_last_pass) {
        cmd_buf.change_image_layout(m_swapchain.image(img_index), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }
    // End the debug label for this graphics pass
    cmd_buf.end_debug_label_region();
}

void RenderGraph::record_command_buffers(const CommandBuffer &cmd_buf, const std::uint32_t img_index) {
    for (std::size_t pass_index = 0; pass_index < m_graphics_passes.size(); pass_index++) {
        const bool is_first_pass = (pass_index == 0);
        const bool is_last_pass = (pass_index == (m_graphics_passes.size() - 1));
        record_command_buffer_for_pass(cmd_buf, *m_graphics_passes[pass_index], is_first_pass, is_last_pass, img_index);
    }
}

void RenderGraph::render() {
    const auto &cmd_buf = m_device.request_command_buffer("[RenderGraph::render]");
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
