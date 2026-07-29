#pragma once

#include <vk_mem_alloc.h>

#include <memory>
#include <string>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::render_graph {
// Forward declarations
class RenderGraph;
class Texture;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::images {

// Using declarations
using render_graph::Texture;
using wrapper::commands::CommandBuffer;
using wrapper::core::Device;

/// RAII wrapper for VkImage and VkImageView
class Image {
private:
    friend Texture;

    /// The device wrapper
    const Device &m_device;
    /// The internal debug name of the image
    std::string m_name;

    VkImage m_img{VK_NULL_HANDLE};
    VkImageCreateInfo m_img_ci{};

    VkImageView m_img_view{VK_NULL_HANDLE};
    VkImageViewCreateInfo m_img_view_ci{};

    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info{};

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

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) = delete;

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

} // namespace inexor::vulkan_renderer::wrapper::images
