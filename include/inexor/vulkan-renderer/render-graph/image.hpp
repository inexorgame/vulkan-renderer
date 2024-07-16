#pragma once

#include <vk_mem_alloc.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
class Sampler;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

// Forward declarations
class RenderGraph;
class Texture;

// Using declaration
using render_graph::RenderGraph;
using render_graph::Texture;
using wrapper::Device;
using wrapper::Sampler;

// NOTE: Originally we did not want to have a RAII wrapper for VkImage and VkImageView and put this into the Texture
// wrapper directly, but since the Texture wrapper contains 2 Images depending on whether MSAA is enabled or not, we
// have chosen to use the Image RAII wrapper.

/// RAII wrapper for VkImage and VkImageView
/// @note Multisample anti-aliasing (MSAA) can be enabled on a per-texture basis
class Image {
    friend class RenderGraph;
    friend class Texture;

private:
    /// The device wrapper
    const Device &m_device;
    /// The internal debug name of the image
    std::string m_name;

    VkImage m_img{VK_NULL_HANDLE};
    VkImageView m_img_view{VK_NULL_HANDLE};
    VmaAllocation m_alloc{VK_NULL_HANDLE};

    VmaAllocationInfo m_alloc_info{};

    const VmaAllocationCreateInfo m_alloc_ci{
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    /// The combined image sampler for the texture
    /// This is only relevant if the texture is used as TextureUsage::NORMAL
    std::unique_ptr<Sampler> m_sampler;

    /// Create the image and the image view
    /// @param img_ci The image create info
    /// @param img_view_ci The image view create info
    void create(VkImageCreateInfo img_ci, VkImageViewCreateInfo img_view_ci);

    /// Destroy the image view, the image, and the sampler
    void destroy();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The name of the Image
    Image(const Device &device, std::string name);

    Image(const Image &) = delete;
    Image(Image &&) noexcept;
    ~Image();

    Image &operator=(const Image &other) = delete;
    Image &operator=(Image &&other) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
