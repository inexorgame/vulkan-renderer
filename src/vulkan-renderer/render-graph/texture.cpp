#include "inexor/vulkan-renderer/render-graph/texture.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using tools::InexorException;

Texture::Texture(const Device &device, std::string name, const TextureUsage usage, const VkFormat format,
                 const std::uint32_t width, const std::uint32_t height, const std::uint32_t channels,
                 const VkSampleCountFlagBits samples, std::function<void()> on_update)
    : m_device(device), m_name(std::move(name)), m_format(format), m_width(width), m_height(height),
      m_channels(channels), m_samples(samples), m_on_check_for_updates(std::move(on_update)), m_type(usage) {
    if (m_name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
    m_image = std::make_shared<render_graph::Image>(m_device, m_name);
    m_default_sampler = std::make_unique<Sampler>(m_device, "Default Sampler");
}

void Texture::request_update(void *src_texture_data, const std::size_t src_texture_data_size) {
    if (src_texture_data == nullptr || src_texture_data_size == 0) {
        return;
    }
    m_src_texture_data = src_texture_data;
    m_src_texture_data_size = src_texture_data_size;
    m_update_requested = true;
}

} // namespace inexor::vulkan_renderer::render_graph
