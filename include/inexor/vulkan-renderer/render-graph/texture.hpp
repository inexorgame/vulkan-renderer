#pragma once

#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <volk.h>

#include <functional>
#include <memory>
#include <optional>
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
class Texture {
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
    std::optional<std::function<void()>> m_on_update{[]() {}};

public:
    /// Default constructor
    /// @param name The internal denug name of the texture inside of the rendergraph (must not be empty)
    /// @param usage The internal usage of the texture inside of the rendergraph
    /// @param format The format of the texture
    /// @param on_update An optional update function (``std::nullopt`` by default, meaning no updates to this buffer)
    Texture(std::string name, TextureUsage usage, VkFormat format, std::optional<std::function<void()>> on_update);

    Texture(const Texture &) = delete;
    Texture(Texture &&other) noexcept;
    ~Texture() = default;

    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = delete;

    // TODO: Implement
    void request_update(const void *texture_src_data, const std::size_t texture_src_data_size) {
        // TODO: Check if source data is nullptr
        // TODO: Check if size is 0
    }
};

} // namespace inexor::vulkan_renderer::render_graph
