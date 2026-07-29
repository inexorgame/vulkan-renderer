#pragma once

#include <vk_mem_alloc.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::images {
// Forward declarations
class Image;
class Sampler;
} // namespace inexor::vulkan_renderer::wrapper::images

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::synchronization {
/// Forward declaration
class PipelineBarrierBatchBuilder;
} // namespace inexor::vulkan_renderer::wrapper::synchronization

namespace inexor::vulkan_renderer::tools {
/// Forward declarations
class InexorException;
class VulkanException;
} // namespace inexor::vulkan_renderer::tools

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class RenderGraph;
class StagingBuffer;

// Using declarations
using tools::InexorException;
using tools::VulkanException;
using wrapper::core::Device;
using wrapper::images::Image;
using wrapper::images::Sampler;
using wrapper::synchronization::PipelineBarrierBatchBuilder;

/// Specifies the use of the texture
enum class TextureUsage {
    DEFAULT,
    COLOR_ATTACHMENT,
    DEPTH_ATTACHMENT,
    STENCIL_ATTACHMENT,
};

///
struct PendingTextureCopy {
    VkBuffer src_buffer{VK_NULL_HANDLE};
    VkImage dst_image{VK_NULL_HANDLE};
    VkBufferImageCopy region{};
    VkImageMemoryBarrier2 post_copy_barrier{};
};

class Texture {
private:
    friend class RenderGraph;

    // The device wrapper
    const Device &m_device;
    // The texture name
    std::string m_name;
    // The texture type
    const TextureUsage m_usage;
    /// By definition, if this is not std::nullopt, this is a dynamic texture
    std::optional<std::function<void()>> m_on_update;

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
    VkSampleCountFlagBits m_samples{VK_SAMPLE_COUNT_1_BIT};

    std::unique_ptr<Sampler> m_default_sampler;

    struct PerFrameTextureResources {
        std::shared_ptr<Image> m_image;
        std::shared_ptr<Image> m_msaa_image;
        VkDescriptorImageInfo m_descriptor_img_info{};
    };

    // This is used for initializing textures and for updating dynamic textures
    bool m_update_requested{true};
    void *m_src_texture_data{nullptr};
    std::size_t m_src_texture_data_size{0};
    std::vector<PerFrameTextureResources> m_per_frame_texture_resources{1};
    std::size_t m_frame_slot_count{1};
    std::size_t m_current_frame_slot{0};

    void create_all();

    void destroy_all();

    void create_per_frame_resources(PerFrameTextureResources &frame_resource, std::size_t frame_index);

    void destroy_per_frame_resources(PerFrameTextureResources &frame_resource);

    [[nodiscard]] PerFrameTextureResources &current_frame_resources();

    [[nodiscard]] const PerFrameTextureResources &current_frame_resources() const;

    void set_frame_context(std::size_t frame_slot_count, std::size_t current_frame_slot);

    /// Collect pending upload copy regions and post-copy barriers for this texture.
    /// @param upload_buffer The shared upload arena buffer
    /// @param upload_alloc The shared upload arena allocation
    /// @param upload_offset Current write offset inside the shared upload arena buffer
    /// @param pending_texture_copies Collected copy jobs and image barriers
    void collect_update_copies(StagingBuffer &staging_buffer, std::size_t &upload_offset,
                               std::vector<std::function<void()>> &pending_releases,
                               std::vector<PendingTextureCopy> &pending_texture_copies);

    /// Record the barriers that prepare the texture image for upload.
    void prepare_update_barriers(PipelineBarrierBatchBuilder &barrier_builder);

    /// Record initial image-layout barriers for newly created attachment textures.
    void prepare_initial_layout_barriers(PipelineBarrierBatchBuilder &barrier_builder);

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The texture name
    /// @param usage The texture usage
    /// @param format The image format
    /// @param width The texture width
    /// @param height The texture height
    /// @param channels The number of channels
    /// @param samples The number of samples
    /// @param on_update The texture update function
    Texture(const Device &device, std::string name, TextureUsage usage, VkFormat format, std::uint32_t width,
            std::uint32_t height, std::uint32_t channels, VkSampleCountFlagBits samples,
            std::optional<std::function<void()>> on_update);

    Texture(const Texture &) = delete;
    Texture(Texture &&) noexcept;

    ~Texture();

    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = delete;

    [[nodiscard]] const auto *descriptor_image_info() const {
        return &current_frame_resources().m_descriptor_img_info;
    }

    [[nodiscard]] VkExtent2D extent() const {
        return {
            .width = m_width,
            .height = m_height,
        };
    }

    [[nodiscard]] VkFormat format() const {
        return m_format;
    }

    [[nodiscard]] VkImageView image_view() const;

    [[nodiscard]] const auto &name() const {
        return m_name;
    }

    /// Resize this texture and request recreation on the next render-graph texture update.
    /// This is primarily used for attachment textures that must track swapchain resize.
    void request_resize(std::uint32_t width, std::uint32_t height);

    void request_update(void *src_texture_data, std::size_t src_texture_data_size);

    [[nodiscard]] auto usage() const {
        return m_usage;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
