#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

GraphicsPass::GraphicsPass(
    std::string name,
    OnRecordCommandBufferForPass on_record_cmd_buffer,
    std::vector<std::weak_ptr<GraphicsPass>> graphics_pass_reads,
    std::vector<std::pair<std::weak_ptr<Texture>, std::optional<VkClearValue>>> write_attachments,
    std::vector<std::pair<std::weak_ptr<Swapchain>, std::optional<VkClearValue>>> write_swapchains,
    const wrapper::DebugLabelColor pass_debug_label_color) {
    // If an extent has already been specified, all attachments must match this!
    if (m_extent.width != 0 && m_extent.height != 0) {
        for (const auto &write_attachment : write_attachments) {
            const auto &attachment = write_attachment.first.lock();
            if (attachment->m_width != m_extent.width) {
                throw std::invalid_argument("[GraphicsPass::GraphicsPass] Error: Width of graphics pass " + m_name +
                                            " is already specified (" + std::to_string(m_extent.width) +
                                            "), but width of write attachment " + attachment->m_name + " (" +
                                            std::to_string(attachment->m_width) + ") does not match!");
            }
            if (attachment->m_height != m_extent.height) {
                throw std::invalid_argument("[GraphicsPass::GraphicsPass] Error: Height of graphics pass " + m_name +
                                            " is already specified (" + std::to_string(m_extent.height) +
                                            "), but width of write attachment " + attachment->m_name + " (" +
                                            std::to_string(attachment->m_height) + ") does not match!");
            }
        }
        for (const auto &write_swapchain : write_swapchains) {
            const auto &swapchain = write_swapchain.first.lock();
            if (swapchain->m_extent.width != m_extent.width) {
                throw std::invalid_argument("[GraphicsPass::GraphicsPass] Error: Width of graphics pass " + m_name +
                                            " is already specified (" + std::to_string(m_extent.width) +
                                            "), but width of write swapchain " + swapchain->m_name + " (" +
                                            std::to_string(swapchain->m_extent.width) + ") does not match!");
            }
            if (swapchain->m_extent.height != m_extent.height) {
                throw std::invalid_argument("[GraphicsPass::GraphicsPass] Error: Height of graphics pass " + m_name +
                                            " is already specified (" + std::to_string(m_extent.height) +
                                            "), but width of write swapchain " + swapchain->m_name + " (" +
                                            std::to_string(swapchain->m_extent.height) + ") does not match!");
            }
        }
    }

    // Pick any extent and store it, they must be all the same at this point
    if (!write_attachments.empty()) {
        const auto &attachment = write_attachments[0].first.lock();
        m_extent = {
            .width = attachment->m_width,
            .height = attachment->m_height,
        };
    } else if (!write_swapchains.empty()) {
        // No color attachments, so pick the extent from any of the swapchains specified
        const auto &swapchain = write_swapchains[0].first.lock();
        m_extent = swapchain->m_extent;
    }
    // Check if either width or height is 0
    if (m_extent.width == 0) {
        throw std::runtime_error("[GraphicsPass::GraphicsPass] Error: m_extent.width is 0!");
    }
    if (m_extent.height == 0) {
        throw std::runtime_error("[GraphicsPass::GraphicsPass] Error: m_extent.height is 0!");
    }

    // Store the other data
    m_name = std::move(name);
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    m_debug_label_color = wrapper::get_debug_label_color(pass_debug_label_color);
    m_graphics_pass_reads = std::move(graphics_pass_reads);
    m_write_attachments = std::move(write_attachments);
    m_write_swapchains = std::move(write_swapchains);
}

GraphicsPass::GraphicsPass(GraphicsPass &&other) noexcept {
    // TODO: Check me!
    m_name = std::move(other.m_name);
    m_on_record_cmd_buffer = std::move(other.m_on_record_cmd_buffer);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, nullptr);
    m_descriptor_set = std::exchange(other.m_descriptor_set, VK_NULL_HANDLE);
    m_rendering_info = std::move(other.m_rendering_info);
    m_write_attachments = std::move(other.m_write_attachments);
    m_write_swapchains = std::move(other.m_write_swapchains);
    m_color_attachments = std::move(other.m_color_attachments);
    m_depth_attachment = std::move(other.m_depth_attachment);
    m_stencil_attachment = std::move(other.m_stencil_attachment);
    m_graphics_pass_reads = std::move(other.m_graphics_pass_reads);
    m_debug_label_color = other.m_debug_label_color;
}

/// Reset the rendering info
void GraphicsPass::reset_rendering_info() {
    m_rendering_info = wrapper::make_info<VkRenderingInfo>();
    m_color_attachments.clear();
    m_has_depth_attachment = false;
    m_depth_attachment = wrapper::make_info<VkRenderingAttachmentInfo>();
    m_has_stencil_attachment = false;
    m_stencil_attachment = wrapper::make_info<VkRenderingAttachmentInfo>();
}

} // namespace inexor::vulkan_renderer::render_graph
