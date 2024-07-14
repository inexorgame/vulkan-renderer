#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

GraphicsPass::GraphicsPass(std::string name,
                           std::function<void(const CommandBuffer &)> on_record,
                           std::weak_ptr<Texture> color_attachment,
                           std::weak_ptr<Texture> depth_attachment,
                           std::weak_ptr<Texture> stencil_attachment,
                           std::weak_ptr<Texture> msaa_color_attachment,
                           std::weak_ptr<Texture> msaa_depth_attachment,
                           bool enable_msaa,
                           bool clear_color_attachment,
                           bool clear_stencil_attachment,
                           std::optional<VkClearValue> clear_values)
    : m_name(std::move(name)), m_on_record(std::move(on_record)), m_color_attachment(std::move(color_attachment)),
      m_depth_attachment(std::move(depth_attachment)), m_stencil_attachment(std::move(stencil_attachment)),
      m_msaa_color_attachment(std::move(msaa_color_attachment)), m_enable_msaa(enable_msaa),
      m_msaa_depth_attachment(msaa_depth_attachment), m_clear_color_attachment(clear_color_attachment),
      m_clear_stencil_attachment(clear_stencil_attachment), m_clear_values(std::move(clear_values)) {}

GraphicsPass::GraphicsPass(GraphicsPass &&other) noexcept {
    m_name = std::move(other.m_name);
    m_clear_values = other.m_clear_values;
    m_on_record = std::move(other.m_on_record);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, nullptr);
    m_descriptor_set = std::exchange(other.m_descriptor_set, VK_NULL_HANDLE);
    m_enable_msaa = other.m_enable_msaa;
}

} // namespace inexor::vulkan_renderer::render_graph
