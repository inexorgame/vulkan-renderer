#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_update_frequency.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

TextureResource::TextureResource(std::string name, const TextureUsage usage, const VkFormat format)
    : m_usage(usage), m_format(format), m_name(std::move(name)) {}

TextureResource::TextureResource(TextureResource &&other) noexcept {
    m_name = std::move(other.m_name);
    m_usage = other.m_usage;
    m_format = other.m_format;
    m_texture = std::exchange(other.m_texture, nullptr);
}

} // namespace inexor::vulkan_renderer::render_graph
