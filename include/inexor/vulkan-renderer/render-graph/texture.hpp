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

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward delcaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

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

using wrapper::Device;
using wrapper::commands::CommandBuffer;

/// RAII wrapper for texture resources in the rendergraph
class Texture {
private:
    friend class RenderGraph;

    const Device &m_device;
    std::string m_name;
    TextureUsage m_usage;
    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info{};
    VkImageCreateInfo m_img_ci{};
    VkImageViewCreateInfo m_img_view_ci{};
    VkFormat m_format{VK_FORMAT_UNDEFINED};

    VmaAllocationCreateInfo m_alloc_ci{
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    std::optional<std::function<void()>> m_on_init;
    std::optional<std::function<void()>> m_on_update;

    void *m_texture_data{nullptr};
    std::size_t m_texture_data_size{0};
    bool m_update_requested{false};
    VkImage m_img{VK_NULL_HANDLE};
    VkImageView m_img_view{VK_NULL_HANDLE};
    VkBuffer m_staging_buffer{VK_NULL_HANDLE};
    VmaAllocation m_staging_buffer_alloc{VK_NULL_HANDLE};
    // TODO: Sampler here as well? Like create_sampler();

    /// Only RenderGraph is allowed to create the texture
    void create_texture();

    /// Only RenderGraph is allowed to create the texture
    /// @param img_ci
    /// @param img_view_ci
    void create_texture(VkImageCreateInfo img_ci, VkImageViewCreateInfo img_view_ci);

    /// Call vkDestroyImageView and then vmaDestroyImage
    void destroy_texture();

    /// Upload the data into the texture
    /// @param cmd_buf The command buffer to record into
    void execute_update(const CommandBuffer &cmd_buf);

public:
    /// Constructor for external textures (created outside of rendergraph)
    /// @param device The device wrapper
    /// @param name The internal debug name of the texture inside of the rendergraph (must not be empty!)
    /// @param usage The internal usage of the texture inside of the rendergraph
    /// @param on_init The init function of the texture (``std::nullopt`` by default)
    /// @param on_update An optional update function of the texture (``std::nullopt`` by default)
    Texture(const Device &device,
            std::string name,
            TextureUsage usage,
            std::optional<std::function<void()>> on_init = std::nullopt,
            std::optional<std::function<void()>> on_update = std::nullopt);

    /// Constructor for internal textures (created inside of rendergraph)
    /// @param device The device wrapper
    /// @param name The internal debug name of the texture inside of the rendergraph (must not be empty!)
    /// @param usage The internal usage of the texture inside of the rendergraph
    /// @param format The texture format
    Texture(const Device &device, std::string name, TextureUsage usage, VkFormat format);

    Texture(const Texture &) = delete;
    Texture(Texture &&other) noexcept;
    ~Texture();

    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = delete;

    /// Request rendergraph to update the texture
    /// @param src_texture_data A pointer to the texture data
    /// @param src_texture_data_size The size of the texture data to which ``texture_src_data`` points to
    /// @param img_ci The image create info
    /// @param img_view_ci The image view create info
    void request_update(void *src_texture_data,
                        std::size_t src_texture_data_size,
                        VkImageCreateInfo img_ci,
                        VkImageViewCreateInfo img_view_ci);
};

} // namespace inexor::vulkan_renderer::render_graph
