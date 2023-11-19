#pragma once

#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <volk.h>

#include <functional>
#include <memory>
#include <string>

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class RenderGraph;
enum class DescriptorSetUpdateFrequencyGroup;

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

// TODO: Implement texture updates and use DescriptorSetUpdateFrequencyGroup

/// Wrapper for texture resources in the rendergraph
class TextureResource {
    friend RenderGraph;

private:
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

public:
    /// Default constructor
    /// @param usage The internal usage of the texture inside of the rendergraph
    /// @param format The format of the texture
    /// @param name The internal denug name of the texture inside of the rendergraph (must not be empty)
    TextureResource(std::string name, TextureUsage usage, VkFormat format);

    TextureResource(const TextureResource &) = delete;
    TextureResource(TextureResource &&other) noexcept;
    ~TextureResource() = default;

    TextureResource &operator=(const TextureResource &) = delete;
    TextureResource &operator=(TextureResource &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
