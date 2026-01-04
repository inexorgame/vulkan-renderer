#include "inexor/vulkan-renderer/render-graph/buffer.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Buffer::Buffer(const Device &device, std::string name, const BufferType type, std::function<void()> on_update)
    : m_device(device), m_name(std::move(name)), m_type(type), m_on_update(std::move(on_update)) {}

} // namespace inexor::vulkan_renderer::render_graph
