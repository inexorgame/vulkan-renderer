#include "inexor/vulkan-renderer/render-graph/texture.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_update_frequency.hpp"

#include <stdexcept>
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
    // TODO: Check me!
    m_name = std::move(other.m_name);
    m_usage = other.m_usage;
    m_format = other.m_format;
    m_texture = std::exchange(other.m_texture, nullptr);
    m_texture_data = std::exchange(other.m_texture_data, nullptr);
    m_texture_data_size = other.m_texture_data_size;
    m_width = other.m_width;
    m_height = other.m_height;
    m_channels = other.m_channels;
    m_mip_levels = other.m_mip_levels;
    m_on_init = std::move(other.m_on_init);
    m_on_update = std::move(other.m_on_update);
}

void Texture::create_texture() {
    // TODO: Implement me!
}

void Texture::request_update(void *texture_src_data, const std::size_t src_texture_data_size) {
    if (texture_src_data == nullptr) {
        throw std::invalid_argument("[Texture::request_update] Error: Parameter 'texture_src_data' is nullptr!");
    }
    if (src_texture_data_size == 0) {
        throw std::invalid_argument("[Texture::request_update] Error: Parameter 'src_texture_data_size' is 0!");
    }
    // NOTE: It is the responsibility of the programmer to make sure the memory this pointer points to is still
    // valid when the actual copy operation for the update is carried out!
    m_texture_data = texture_src_data;
    m_texture_data_size = src_texture_data_size;
}

} // namespace inexor::vulkan_renderer::render_graph
