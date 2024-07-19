#pragma once

#include <vk_mem_alloc.h>

#include "inexor/vulkan-renderer/render-graph/image.hpp"

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

namespace inexor::vulkan_renderer::wrapper::descriptors {
/// Forward declaration
class WriteDescriptorSetBuilder;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::render_graph {

/// Specifies the use of the texture
/// NOTE: All usages which are not TextureUsage::NORMAL are for internal usage inside of rendergraph only
enum class TextureUsage {
    NORMAL,
    COLOR_ATTACHMENT,
    DEPTH_ATTACHMENT,
    STENCIL_ATTACHMENT,
};

// Forward declaration
class GraphicsPass;

// Using declarations
using wrapper::Device;
using wrapper::commands::CommandBuffer;
using wrapper::descriptors::WriteDescriptorSetBuilder;

/// RAII wrapper for texture resources
class Texture {
    // These friend classes are allowed to access the private data of Texture
    friend class WriteDescriptorSetBuilder;
    friend class GraphicsPass;
    friend class RenderGraph;

private:
    /// The device wrapper
    const Device &m_device;
    /// The name of the texture
    std::string m_name;
    /// The usage of this texture
    TextureUsage m_usage;
    /// The format of the texture
    VkFormat m_format{VK_FORMAT_UNDEFINED};
    /// The width of the texture
    std::uint32_t m_width{0};
    /// The height of the texture
    std::uint32_t m_height{0};
    /// The channel count of the texture (4 by default)
    // TODO: Can we determine the number of channels based on the given format?
    std::uint32_t m_channels{4};
    /// The sample count of the MSAA image (if MSAA is enabled)
    VkSampleCountFlagBits m_sample_count;

    /// The image of the texture
    std::unique_ptr<Image> m_img;

    /// This is only used internally inside of rendergraph in case this texture used as a back buffer, depth buffer, or
    /// stencil buffer and MSAA is enabled.
    std::unique_ptr<Image> m_msaa_img;

    // This is used for initializing textures and for updating dynamic textures
    bool m_update_requested{false};
    void *m_src_texture_data{nullptr};
    std::size_t m_src_texture_data_size{0};

    /// This is for initialization of textures which are of TextureUsage::NORMAL
    std::optional<std::function<void()>> m_on_init{std::nullopt};
    /// By definition, if this is not std::nullopt, this is a dynamic texture
    std::optional<std::function<void()>> m_on_check_for_updates{std::nullopt};

    // The staging buffer for updating the texture data
    VkBuffer m_staging_buffer{VK_NULL_HANDLE};
    VmaAllocation m_staging_buffer_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_staging_buffer_alloc_info{};

    /// This part of the image wrapper is for external use outside of rendergraph
    /// The descriptor image info required for descriptor updates
    VkDescriptorImageInfo m_descriptor_img_info{};

    /// Create the texture (and the MSAA texture if specified)
    void create();

    /// Destroy the texture (and the MSAA texture if specified)
    void destroy();

    /// Destroy the staging buffer used for texture updates
    void destroy_staging_buffer();

    /// Upload the data into the texture
    /// @param cmd_buf The command buffer to record the commands into
    void update(const CommandBuffer &cmd_buf);

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The internal debug name of the texture
    /// @param usage The usage of the texture inside of rendergraph
    /// @param format The format of the texture
    /// @param width The width of the texture
    /// @param height The height of the texture
    /// @param sample_count The sample count of the texture
    /// @param on_init The initialization function of the texture
    /// @note If ``on_init`` is ``std::nullopt``, it means the texture will be initialized internally in rendergraph.
    /// @param on_check_for_updates The update function of the texture
    /// @note If ``on_check_for_updates`` is ``std::nullopt``, the texture will not be updated at all!
    Texture(const Device &device,
            std::string name,
            TextureUsage usage,
            VkFormat format,
            std::uint32_t width,
            std::uint32_t height,
            VkSampleCountFlagBits sample_count, // TODO: Channel count as well?
            std::optional<std::function<void()>> on_init = std::nullopt,
            std::optional<std::function<void()>> on_check_for_updates = std::nullopt);

    Texture(const Texture &) = delete;
    Texture(Texture &&other) noexcept;
    ~Texture();

    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = delete;

    /// Request rendergraph to update the texture
    /// @param src_texture_data A pointer to the source data
    /// @param src_texture_data_size The size of the source data
    /// @note It is the responsibility of the programmer to make sure the memory the pointer points to is still valid
    /// when rendergraph is carrying out the update!
    void request_update(void *src_texture_data, std::size_t src_texture_data_size);
};

} // namespace inexor::vulkan_renderer::render_graph
