#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

GraphicsPass::GraphicsPass(
    std::string name,
    std::vector<std::pair<std::weak_ptr<BufferResource>, std::optional<VkShaderStageFlagBits>>> buffer_reads,
    std::vector<std::pair<std::weak_ptr<TextureResource>, std::optional<VkShaderStageFlagBits>>> texture_reads,
    std::vector<std::weak_ptr<TextureResource>> texture_writes,
    std::function<void(const wrapper::CommandBuffer &)> on_record, std::optional<VkClearValue> clear_values)
    : m_name(std::move(name)), m_buffer_reads(std::move(buffer_reads)), m_texture_reads(std::move(texture_reads)),
      m_texture_writes(std::move(texture_writes)), m_on_record(std::move(on_record)),
      m_clear_values(std::move(clear_values)) {}

GraphicsPass::GraphicsPass(GraphicsPass &&other) noexcept {}

} // namespace inexor::vulkan_renderer::render_graph
