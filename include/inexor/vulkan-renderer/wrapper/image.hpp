#pragma once

#include <vk_mem_alloc.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// @brief RAII wrapper class for VkImage.
class Image {
    const Device &m_device;
    VmaAllocation m_allocation{VK_NULL_HANDLE};
    VmaAllocationInfo m_allocation_info{};
    VkImage m_image{VK_NULL_HANDLE};
    VkFormat m_format{VK_FORMAT_UNDEFINED};
    VkImageView m_image_view{VK_NULL_HANDLE};
    std::string m_name;

public:
    /// @brief Default constructor.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param format The color format.
    /// @param image_usage The image usage flags.
    /// @param aspect_flags The aspect flags.
    /// @param sample_count The sample count.
    /// @param name The internal debug marker name of the VkImage.
    /// @param image_extent The width and height of the image.
    Image(const Device &device, VkFormat format, VkImageUsageFlags image_usage, VkImageAspectFlags aspect_flags,
          VkSampleCountFlagBits sample_count, const std::string &name, VkExtent2D image_extent);

    Image(const Image &) = delete;
    Image(Image &&) noexcept;

    ~Image();

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) = delete;

    [[nodiscard]] VkImage image() const {
        return m_image;
    }

    [[nodiscard]] VkFormat image_format() const {
        return m_format;
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_image_view;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
