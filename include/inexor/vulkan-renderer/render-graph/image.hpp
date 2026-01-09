#pragma once

#include <vk_mem_alloc.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::render_graph {
// Forward declarations
class RenderGraph;
class Texture;
} // namespace inexor::vulkan_renderer::render_graph

// Using declaration
using inexor::vulkan_renderer::wrapper::Device;
using inexor::vulkan_renderer::wrapper::commands::CommandBuffer;

namespace inexor::vulkan_renderer::render_graph {

// NOTE: Originally we did not want to have a RAII wrapper for VkImage and VkImageView and put this into the Texture
// wrapper directly, but since the Texture wrapper contains 2 Images depending on whether MSAA is enabled or not, we
// have chosen to use the Image RAII wrapper.

// TODO: Move this to wrapper/ folder again (it is not really part of rendergraph)

/// RAII wrapper for VkImage and VkImageView
/// @note Multisample anti-aliasing (MSAA) can be enabled on a per-texture basis
class Image {
    friend CommandBuffer;
    friend render_graph::RenderGraph;
    friend render_graph::Texture;

private:
    /// The device wrapper
    const Device &m_device;
    /// The internal debug name of the image
    std::string m_name;

    VkImageCreateInfo m_img_ci{};
    VkImageViewCreateInfo m_img_view_ci{};

    VkImage m_img{VK_NULL_HANDLE};
    VkImageView m_img_view{VK_NULL_HANDLE};
    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info{};

    const VmaAllocationCreateInfo m_alloc_ci{
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

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

    [[nodiscard]] auto height() const {
        return m_img_ci.extent.height;
    }

    [[nodiscard]] auto image() const {
        return m_img;
    }

    [[nodiscard]] auto image_view() const {
        return m_img_view;
    }

    [[nodiscard]] auto width() const {
        return m_img_ci.extent.width;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
