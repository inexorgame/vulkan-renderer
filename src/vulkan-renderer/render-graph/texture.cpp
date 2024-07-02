#include "inexor/vulkan-renderer/render-graph/texture.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_update_frequency.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Texture::Texture(std::string name,
                 const TextureUsage usage,
                 const VkFormat format,
                 std::optional<std::function<void()>> on_init,
                 std::optional<std::function<void()>> on_update)
    : m_name(std::move(name)), m_usage(usage), m_format(format), m_on_init(std::move(on_init)),
      m_on_update(std::move(on_update)) {}

Texture::Texture(Texture &&other) noexcept {
    m_name = std::move(other.m_name);
    m_usage = other.m_usage;
    m_format = other.m_format;
    m_texture = std::exchange(other.m_texture, nullptr);
    m_data = std::exchange(other.m_data, nullptr);
    m_data_size = other.m_data_size;
    m_width = other.m_width;
    m_height = other.m_height;
    m_channels = other.m_channels;
    m_mip_levels = other.m_mip_levels;
    m_on_update = std::move(other.m_on_update);
}

void Texture::create_texture() {
    // TODO: Implement me!
}

} // namespace inexor::vulkan_renderer::render_graph
