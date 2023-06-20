#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"

#include "inexor/vulkan-renderer/render-graph/descriptor_set_update_frequency_category.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

TextureResource::TextureResource(std::string name, const TextureUsage usage, const VkFormat format,
                                 const DescriptorSetUpdateFrequencyCategory update_frequency,
                                 std::function<void()> on_update)
    : m_usage(usage), m_format(format), m_name(std::move(name)), m_on_update(std::move(on_update)),
      m_update_frequency(update_frequency) {}

TextureResource::TextureResource(TextureResource &&other) noexcept {
    m_name = std::move(other.m_name);
    m_usage = other.m_usage;
    m_format = other.m_format;
    m_on_update = std::move(other.m_on_update);
    m_texture = std::exchange(other.m_texture, nullptr);
}

void TextureResource::announce_update(void *data, const std::size_t data_size, const std::uint32_t width,
                                      const std::uint32_t height, const std::uint32_t channels,
                                      const std::uint32_t mip_levels) {
    m_data = data;
    m_data_size = data_size;
    m_width = width;
    m_height = height;
    m_channels = channels;
    m_mip_levels = mip_levels;
}

} // namespace inexor::vulkan_renderer::render_graph
