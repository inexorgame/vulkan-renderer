#pragma once

#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <volk.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::render_graph {

/// Specifies the use of the texture inside of the rendergraph
enum class TextureUsage {
    /// Specifies that this texture is the output of the render graph
    BACK_BUFFER,
    MSAA_BACK_BUFFER,
    /// Specifies that this texture is a combined depth/stencil buffer
    DEPTH_STENCIL_BUFFER,
    MSAA_DEPTH_STENCIL_BUFFER,
    /// Specifies that this texture isn't used for any special purpose
    NORMAL,
};

// Forward declaration
class RenderGraph;

/// Wrapper for texture resources in the rendergraph
class Texture {
private:
    friend RenderGraph;

    std::string m_name;
    TextureUsage m_usage;
    VkFormat m_format{VK_FORMAT_UNDEFINED};
    std::unique_ptr<wrapper::Image> m_texture;

    void *m_data{nullptr};
    std::size_t m_data_size{0};
    std::uint32_t m_width{0};
    std::uint32_t m_height{0};
    std::uint32_t m_channels{0};
    std::uint32_t m_mip_levels{0};

    std::optional<std::function<void()>> m_on_init;
    std::optional<std::function<void()>> m_on_update;

    // TODO: Implement!
    void create_texture();

public:
    /// Default constructor
    /// @param name The internal debug name of the texture inside of the rendergraph (must not be empty)
    /// @param usage The internal usage of the texture inside of the rendergraph
    /// @param format The format of the texture
    /// @param on_init The init function of the texture (``std::nullopt`` by default)
    /// @note There are several ways a texture can be initialized inside of rendergraph. A depth buffer for example does
    /// not require an on_init function, as rendergraph creates it internally. A static textures requires an on_init
    /// function, but no on_update function. A dynamic texture requires on_init and on_update.
    /// @param on_update An optional update function of the texture (``std::nullopt`` by default)
    Texture(std::string name,
            TextureUsage usage,
            VkFormat format,
            std::optional<std::function<void()>> on_init = std::nullopt,
            std::optional<std::function<void()>> on_update = std::nullopt);

    Texture(const Texture &) = delete;
    Texture(Texture &&other) noexcept;
    ~Texture() = default;

    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = delete;

    // TODO: Implement (In what API style though?)
    void request_update(const void *src_data, const std::size_t src_data_size) {
        // TODO: Check if source data is nullptr
        // TODO: Check if size is 0
    }
};

} // namespace inexor::vulkan_renderer::render_graph
