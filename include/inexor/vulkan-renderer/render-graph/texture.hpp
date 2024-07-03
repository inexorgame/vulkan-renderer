#pragma once

#include <vk_mem_alloc.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

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

using wrapper::Device;

/// RAII wrapper for texture resources in the rendergraph
class Texture {
private:
    friend RenderGraph;

    const Device &m_device;
    std::string m_name;
    TextureUsage m_usage;
    VkFormat m_format{VK_FORMAT_UNDEFINED};

    void *m_texture_data{nullptr};
    std::size_t m_texture_data_size{0};
    std::uint32_t m_width{0};
    std::uint32_t m_height{0};
    std::uint32_t m_channels{0};
    std::uint32_t m_mip_levels{0};

    std::optional<std::function<void()>> m_on_init;
    std::optional<std::function<void()>> m_on_update;

    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info{};
    VkImage m_img{VK_NULL_HANDLE};
    VkImageView m_img_view{VK_NULL_HANDLE};
    // TODO: Sampler here as well?

    /// Only RenderGraph is allowed to create the texture
    void create_texture(const VkImageCreateInfo &img_ci,
                        const VkImageViewCreateInfo &img_view_ci,
                        const VmaAllocationCreateInfo &alloc_ci);

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The internal debug name of the texture inside of the rendergraph (must not be empty!)
    /// @param usage The internal usage of the texture inside of the rendergraph
    /// @param format The format of the texture
    /// @param on_init The init function of the texture (``std::nullopt`` by default)
    /// @note There are several ways a texture can be initialized inside of rendergraph. A depth buffer for example does
    /// not require an on_init function, as rendergraph creates it internally. A static textures requires an on_init
    /// function, but no on_update function. A dynamic texture requires on_init and on_update.
    /// @param on_update An optional update function of the texture (``std::nullopt`` by default)
    Texture(const Device &device,
            std::string name,
            TextureUsage usage,
            VkFormat format,
            std::optional<std::function<void()>> on_init = std::nullopt,
            std::optional<std::function<void()>> on_update = std::nullopt);

    Texture(const Texture &) = delete;
    Texture(Texture &&other) noexcept;
    ~Texture() = default;

    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = delete;

    /// Request rendergraph to update the texture
    /// @param texture_src_data A pointer to the texture data
    /// @param src_texture_data_size The size of the texture data to which ``texture_src_data`` points to
    void request_update(void *texture_src_data, const std::size_t src_texture_data_size);
};

} // namespace inexor::vulkan_renderer::render_graph
