#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using tools::InexorException;
using tools::make_info;

GraphicsPass::GraphicsPass(
    std::string name, std::function<void(const CommandBuffer &)> on_record_cmd_buffer,
    std::vector<std::weak_ptr<Buffer>> buffer_reads,
    std::vector<std::pair<std::weak_ptr<Texture>, std::optional<VkClearValue>>> texture_writes,
    std::vector<std::pair<std::weak_ptr<Swapchain>, std::optional<VkClearValue>>> swapchain_writes,
    const DebugLabelColor pass_debug_label_color) {
    // Pick any extent and store it, they must be all the same at this point
    if (!texture_writes.empty()) {
        const auto &attachment = texture_writes[0].first.lock();
        m_extent = {
            .width = attachment->extent().width,
            .height = attachment->extent().height,
        };
    } else if (!swapchain_writes.empty()) {
        // No color attachments, so pick the extent from any of the swapchains specified
        const auto &swapchain = swapchain_writes[0].first.lock();
        m_extent = swapchain->extent();
    }
    // Check if either width or height is 0
    if (m_extent.width == 0) {
        throw InexorException("Error: m_extent.width is 0!");
    }
    if (m_extent.height == 0) {
        throw InexorException("Error: m_extent.height is 0!");
    }

    buffer_reads.reserve(buffer_reads.size());
    texture_writes.reserve(texture_writes.size());
    swapchain_writes.reserve(swapchain_writes.size());

    m_name = std::move(name);
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    m_buffer_reads = std::move(buffer_reads);
    m_texture_writes = std::move(texture_writes);
    m_swapchain_writes = std::move(swapchain_writes);
    m_debug_label_color = wrapper::core::get_debug_label_color(pass_debug_label_color);

    const auto texture_write_count = m_texture_writes.size();
    const auto swapchain_write_count = m_swapchain_writes.size();
    m_cached_texture_attachment_states.reserve(texture_write_count);
    m_cached_swapchain_attachment_states.reserve(swapchain_write_count);
    m_scratch_current_texture_states.reserve(texture_write_count);
    m_scratch_current_swapchain_states.reserve(swapchain_write_count);
    m_cached_texture_color_attachment_formats.reserve(texture_write_count);
    m_cached_color_attachment_formats.reserve(texture_write_count + swapchain_write_count);
    m_color_attachments.reserve(texture_write_count + swapchain_write_count);
}

GraphicsPass::GraphicsPass(GraphicsPass &&other) noexcept {
    m_name = std::move(other.m_name);
    m_on_record_cmd_buffer = std::move(other.m_on_record_cmd_buffer);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, nullptr);
    m_descriptor_set = std::exchange(other.m_descriptor_set, VK_NULL_HANDLE);
    m_debug_label_color = other.m_debug_label_color;
    m_extent = std::move(other.m_extent);
    m_buffer_reads = std::move(other.m_buffer_reads);
    m_texture_writes = std::move(other.m_texture_writes);
    m_swapchain_writes = std::move(other.m_swapchain_writes);
    m_rendering_info = std::move(other.m_rendering_info);
    m_color_attachments = std::move(other.m_color_attachments);
    m_depth_attachment = std::move(other.m_depth_attachment);
    m_stencil_attachment = std::move(other.m_stencil_attachment);
    m_rendering_info_dirty = other.m_rendering_info_dirty;
    m_cached_render_extent = other.m_cached_render_extent;
    m_cached_texture_attachment_states = std::move(other.m_cached_texture_attachment_states);
    m_cached_swapchain_attachment_states = std::move(other.m_cached_swapchain_attachment_states);
    m_cached_texture_color_attachment_formats = std::move(other.m_cached_texture_color_attachment_formats);
    m_cached_color_attachment_formats = std::move(other.m_cached_color_attachment_formats);
    m_cached_depth_attachment_format = other.m_cached_depth_attachment_format;
    m_cached_stencil_attachment_format = other.m_cached_stencil_attachment_format;
    m_scratch_current_texture_states = std::move(other.m_scratch_current_texture_states);
    m_scratch_current_swapchain_states = std::move(other.m_scratch_current_swapchain_states);
}

void GraphicsPass::reset_rendering_info() {
    m_rendering_info = make_info<VkRenderingInfo>();
    m_color_attachments.clear();
    m_depth_attachment = std::nullopt;
    m_stencil_attachment = std::nullopt;
    m_rendering_info_dirty = true;
}

} // namespace inexor::vulkan_renderer::render_graph
