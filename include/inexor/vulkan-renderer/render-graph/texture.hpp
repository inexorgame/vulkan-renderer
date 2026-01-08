#pragma once

#include "inexor/vulkan-renderer/render-graph/image.hpp"
#include "inexor/vulkan-renderer/wrapper/sampler.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
class Sampler;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::Device;
using wrapper::Sampler;

/// Specifies the use of the texture
enum class TextureUsage {
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
    const TextureUsage m_type;
    // The texture update function
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

    /// The image of the texture
    std::shared_ptr<render_graph::Image> m_image;

    // TODO: MSAA support?

    // This is used for initializing textures and for updating dynamic textures
    bool m_update_requested{true};
    void *m_src_texture_data{nullptr};
    std::size_t m_src_texture_data_size{0};

    /// By definition, if this is not std::nullopt, this is a dynamic texture
    std::function<void()> m_on_check_for_updates;

    // The staging buffer for updating the texture data
    VkBuffer m_staging_buffer{VK_NULL_HANDLE};
    VmaAllocation m_staging_buffer_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_staging_buffer_alloc_info{};

    /// This part of the image wrapper is for external use outside of rendergraph
    /// The descriptor image info required for descriptor updates
    VkDescriptorImageInfo m_descriptor_img_info{};

    // @TODO: create(), destroy() destroy_staging_buffer(), update()

public:
    // TODO: Think about overloading the constructor here

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
            std::function<void()> on_update);

    Texture(const Texture &) = delete;
    Texture(Texture &&) noexcept;

    Texture &operator=(const Texture &) = delete;
    Texture &operator=(Texture &&) = delete;

    [[nodiscard]] const auto *descriptor_image_info() const {
        return &m_descriptor_img_info;
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

    [[nodiscard]] const auto &name() const {
        return m_name;
    }

    void request_update(void *src_texture_data, std::size_t src_texture_data_size);
};

} // namespace inexor::vulkan_renderer::render_graph
