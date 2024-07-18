#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

// TODO: Move to device wrapper?
std::array<float, 4> get_debug_label_color(const DebugLabelColor color) {
    switch (color) {
    case DebugLabelColor::RED:
        return {0.98f, 0.60f, 0.60f, 1.0f};
    case DebugLabelColor::BLUE:
        return {0.68f, 0.85f, 0.90f, 1.0f};
    case DebugLabelColor::GREEN:
        return {0.73f, 0.88f, 0.73f, 1.0f};
    case DebugLabelColor::YELLOW:
        return {0.98f, 0.98f, 0.70f, 1.0f};
    case DebugLabelColor::PURPLE:
        return {0.80f, 0.70f, 0.90f, 1.0f};
    case DebugLabelColor::ORANGE:
        return {0.98f, 0.75f, 0.53f, 1.0f};
    case DebugLabelColor::MAGENTA:
        return {0.96f, 0.60f, 0.76f, 1.0f};
    case DebugLabelColor::CYAN:
        return {0.70f, 0.98f, 0.98f, 1.0f};
    case DebugLabelColor::BROWN:
        return {0.82f, 0.70f, 0.55f, 1.0f};
    case DebugLabelColor::PINK:
        return {0.98f, 0.75f, 0.85f, 1.0f};
    case DebugLabelColor::LIME:
        return {0.80f, 0.98f, 0.60f, 1.0f};
    case DebugLabelColor::TURQUOISE:
        return {0.70f, 0.93f, 0.93f, 1.0f};
    case DebugLabelColor::BEIGE:
        return {0.96f, 0.96f, 0.86f, 1.0f};
    case DebugLabelColor::MAROON:
        return {0.76f, 0.50f, 0.50f, 1.0f};
    case DebugLabelColor::OLIVE:
        return {0.74f, 0.75f, 0.50f, 1.0f};
    case DebugLabelColor::NAVY:
        return {0.53f, 0.70f, 0.82f, 1.0f};
    case DebugLabelColor::TEAL:
        return {0.53f, 0.80f, 0.75f, 1.0f};
    default:
        return {0.0f, 0.0f, 0.0f, 1.0f}; // Default to opaque black if the color is not recognized
    }
}

GraphicsPass::GraphicsPass(std::string name,
                           OnRecordCommandBufferForPass on_record_cmd_buffer,
                           std::vector<std::weak_ptr<GraphicsPass>> graphics_pass_reads,
                           std::vector<std::weak_ptr<Texture>> write_attachments,
                           std::vector<std::weak_ptr<Swapchain>> write_swapchains,
                           const DebugLabelColor pass_debug_label_color) {
    m_name = std::move(name);
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    m_debug_label_color = get_debug_label_color(pass_debug_label_color);
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
    m_color_attachment_infos = std::move(other.m_color_attachment_infos);
    m_depth_attachment_info = std::move(other.m_depth_attachment_info);
    m_stencil_attachment_info = std::move(other.m_stencil_attachment_info);
    m_graphics_pass_reads = std::move(other.m_graphics_pass_reads);
    m_debug_label_color = other.m_debug_label_color;
}

} // namespace inexor::vulkan_renderer::render_graph
