#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

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
                           std::function<void(const CommandBuffer &)> on_record_cmd_buffer,
                           std::vector<std::weak_ptr<GraphicsPass>> graphics_pass_reads,
                           std::vector<RenderingAttachment> write_attachments,
                           const DebugLabelColor color) {
    m_name = std::move(name);
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    m_debug_label_color = get_debug_label_color(color);

    // As internal validation, we check if each graphics pass has only one depth attachment at maximum and only one
    // stencil attachment at maximum. Note that the graphcis pass is allowed to have as many color attachments as it
    // wants to.
    bool depth_attachment_found{false};

    // Sort the attachment writes by type and make sure depth buffer is not specified more than once at maximum
    for (auto &write_attachment : write_attachments) {
        switch (write_attachment.first.lock()->m_usage) {
        case TextureUsage::NORMAL:
        case TextureUsage::BACK_BUFFER: {
            m_color_attachments.emplace_back(std::move(write_attachment));
            break;
        }
        case TextureUsage::DEPTH_STENCIL_BUFFER: {
            if (depth_attachment_found) {
                throw std::runtime_error(
                    "[GraphicsPass::GraphicsPass] Error: Duplicate depth stencil buffer specified!");
            }
            depth_attachment_found = true;
            m_depth_attachment = std::move(write_attachment);
            break;
        }
        }
    }
    // TODO: Support for stencil buffers
}

GraphicsPass::GraphicsPass(GraphicsPass &&other) noexcept {
    m_name = std::move(other.m_name);
    m_on_record_cmd_buffer = std::move(other.m_on_record_cmd_buffer);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, nullptr);
    m_descriptor_set = std::exchange(other.m_descriptor_set, VK_NULL_HANDLE);
    m_rendering_info = std::move(other.m_rendering_info);
    m_color_attachment_infos = std::move(other.m_color_attachment_infos);
    m_depth_attachment_info = std::move(other.m_depth_attachment_info);
    m_stencil_attachment_info = std::move(other.m_stencil_attachment_info);
    m_debug_label_color = other.m_debug_label_color;
}

} // namespace inexor::vulkan_renderer::render_graph
