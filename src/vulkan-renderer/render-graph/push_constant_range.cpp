#include "inexor/vulkan-renderer/render-graph/push_constant_range.hpp"

namespace inexor::vulkan_renderer::render_graph {

PushConstantRange::PushConstantRange(const VkPushConstantRange push_constant, const void *push_constant_data,
                                     std::function<void()> on_update)
    : m_push_constant(push_constant), m_push_constant_data(push_constant_data), m_on_update(std::move(on_update)) {}

PushConstantRange::PushConstantRange(PushConstantRange &&other) noexcept {
    m_push_constant = std::move(other.m_push_constant);
    m_on_update = std::move(other.m_on_update);
    m_push_constant_data = std::exchange(other.m_push_constant_data, nullptr);
}

} // namespace inexor::vulkan_renderer::render_graph
