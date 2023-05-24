#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

TextureResource::TextureResource(const TextureUsage usage, const VkFormat format, std::string name)
    : m_usage(usage), m_format(format), m_name(std::move(name)) {}

TextureResource::TextureResource(TextureResource &&other) noexcept {
    m_name = std::move(other.m_name);
    m_usage = other.m_usage;
    m_format = other.m_format;
}

} // namespace inexor::vulkan_renderer::render_graph
