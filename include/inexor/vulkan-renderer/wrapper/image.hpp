#pragma once

#include <vk_mem_alloc.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declarations
class Device;

/// RAII wrapper class for VkImage
class Image {
    const Device &m_device;
    VkFormat m_format{VK_FORMAT_UNDEFINED};
    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info{};
    VkImage m_img{VK_NULL_HANDLE};
    VkImageView m_img_view{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param img_ci The image create info
    /// @param img_view_ci The image view create info
    /// @param alloc_ci The allocation create info
    /// @param name The internal name of the image and the image view
    Image(const Device &device, const VkImageCreateInfo &img_ci, const VmaAllocationCreateInfo &alloc_ci,
          const VkImageViewCreateInfo &img_view_ci, std::string name);
    Image(const Image &) = delete;
    Image(Image &&) noexcept;
    ~Image();

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) = delete;

    [[nodiscard]] VkFormat image_format() const {
        return m_format;
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_img_view;
    }

    [[nodiscard]] VkImage image() const {
        return m_img;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
