#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

BufferResource::BufferResource(const BufferUsage usage, const VkDeviceSize size, std::string name)
    : m_usage(usage), m_name(std::move(name)), m_buffer_size(size)) {}

BufferResource::BufferResource(BufferResource &&other) noexcept {
    m_name = std::move(other.m_name);
    m_usage = other.m_usage;
    m_buffer_size = other.m_buffer_size;
    m_requires_staging_buffer = other.m_requires_staging_buffer;
}

} // namespace inexor::vulkan_renderer::render_graph
