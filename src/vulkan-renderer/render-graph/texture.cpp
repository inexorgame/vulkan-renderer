#include "inexor/vulkan-renderer/render-graph/texture.hpp"

namespace inexor::vulkan_renderer::render_graph {

Texture::Texture(const Device &device, std::string name, TextureType type,
                 std::optional<std::function<void()>> on_update)
    : m_device(device), m_name(std::move(name)), m_type(type), m_on_update(std::move(on_update)) {}

} // namespace inexor::vulkan_renderer::render_graph
