#pragma once

#include "inexor/vulkan-renderer/render-graph/render_resource.hpp"

namespace inexor::vulkan_renderer::render_graph {

/// Specifies the use of the texture inside of the rendergraph
enum class TextureUsage {
    /// Specifies that this texture is the output of the render graph
    BACK_BUFFER,
    /// Specifies that this texture is a combined depth/stencil buffer
    DEPTH_STENCIL_BUFFER,
    /// Specifies that this texture isn't used for any special purpose
    NORMAL,
};

/// Wrapper for texture resources in the rendergraph
/// @note The constructor is private and only ``RenderGraph`` can use it because it is declared as friend class
class TextureResource {
    // TODO:
    /// @note The constructor is private and only ``RenderGraph`` can use it because it is declared as friend class

private:
    std::string m_name;
    TextureUsage m_usage;
    VkFormat m_format{VK_FORMAT_UNDEFINED};

    // TODO: Should we take width and height into account?

public:
    /// Default constructor
    /// @param usage The usage of the texture in the rendergraph
    /// @param format The format of the texture
    /// @param name The internal debug name of the texture resource
    TextureResource(TextureUsage usage, VkFormat format, std::string name);
    TextureResource(const TextureResource &) = delete;
    TextureResource(TextureResource &&other) noexcept;
    ~TextureResource() = default;

    TextureResource &operator=(const TextureResource &) = delete;
    TextureResource &operator=(TextureResource &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
