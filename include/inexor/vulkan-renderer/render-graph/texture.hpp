#pragma once

#include <functional>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::Device;

/// Specifies the use of the texture
enum class TextureType {
    COLOR_ATTACHMENT,
    DEPTH_ATTACHMENT,
    STENCIL_ATTACHMENT,
    // @TODO Support further texture types (cubemaps...)
};

class Texture {
private:
    // The device wrapper
    const Device &m_device;
    // The texture name
    std::string m_name;
    // The texture type
    const TextureType m_type;
    // The texture update function
    std::optional<std::function<void()>> m_on_update;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The texture name
    /// @param type The texture type
    /// @param on_update The texture update function
    Texture(const Device &device, std::string name, TextureType type, std::optional<std::function<void()>> on_update);

    Texture(const Texture &) = delete;
    Texture(Texture &&) noexcept;

    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
