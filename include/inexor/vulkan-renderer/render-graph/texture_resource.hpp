#pragma once

#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <volk.h>

#include <functional>
#include <memory>
#include <string>

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class RenderGraph;
enum class DescriptorSetUpdateFrequencyCategory;

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
class TextureResource {
    friend RenderGraph;

private:
    std::string m_name;
    TextureUsage m_usage;
    DescriptorSetUpdateFrequencyCategory m_update_frequency;
    VkFormat m_format{VK_FORMAT_UNDEFINED};
    std::function<void()> m_on_update{[]() {}};
    std::unique_ptr<wrapper::Image> m_texture;

    void *m_data{nullptr};
    std::size_t m_data_size{0};
    std::uint32_t m_width{0};
    std::uint32_t m_height{0};
    std::uint32_t m_channels{0};
    std::uint32_t m_mip_levels{0};

public:
    /// Default constructor
    /// @param name The internal denug name of the texture inside of the rendergraph (must not be empty)
    /// @param usage The internal usage of the texture inside of the rendergraph
    /// @param format The format of the texture
    /// @param update_frequency The update frequency of the texture
    /// @param on_update The update function of the texture
    TextureResource(std::string name, TextureUsage usage, VkFormat format,
                    DescriptorSetUpdateFrequencyCategory update_frequency, std::function<void()> on_update);

    TextureResource(const TextureResource &) = delete;
    TextureResource(TextureResource &&other) noexcept;
    ~TextureResource() = default;

    TextureResource &operator=(const TextureResource &) = delete;
    TextureResource &operator=(TextureResource &&) = delete;

    /// Announce an update for the texture
    /// @param data A pointer to the texture data
    /// @param data_size The size of the texture data
    /// @param width The width of the texture
    /// @param height The height of the texture
    /// @param channels The channel count
    /// @param mip_levels The mip level count
    void announce_update(void *data, std::size_t data_size, std::uint32_t width, std::uint32_t height,
                         std::uint32_t channels, std::uint32_t mip_levels);
};

} // namespace inexor::vulkan_renderer::render_graph
