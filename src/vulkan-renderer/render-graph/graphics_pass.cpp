#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

GraphicsPass::GraphicsPass(std::string name,
                           std::function<void(const CommandBuffer &)> on_record_cmd_buffer,
                           std::vector<Attachment> color_attachments,
                           Attachment depth_attachment,
                           Attachment stencil_attachment)
    : m_name(std::move(name)), m_on_record_cmd_buffer(std::move(on_record_cmd_buffer)),
      m_color_attachments(std::move(color_attachments)), m_depth_attachment(std::move(depth_attachment)),
      m_stencil_attachment(std::move(stencil_attachment)) {}

GraphicsPass::GraphicsPass(GraphicsPass &&other) noexcept {
    // TODO: Fix me!
    m_name = std::move(other.m_name);
    m_on_record_cmd_buffer = std::move(other.m_on_record_cmd_buffer);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, nullptr);
    m_descriptor_set = std::exchange(other.m_descriptor_set, VK_NULL_HANDLE);
}

} // namespace inexor::vulkan_renderer::render_graph
