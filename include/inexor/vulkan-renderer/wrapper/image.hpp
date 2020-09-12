#pragma once

#include <vma/vk_mem_alloc.h>

#include <cassert>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @class Image
/// @brief RAII wrapper class for VkImage.
class Image {
private:
    const Device &m_device;
    VmaAllocation m_allocation;
    VmaAllocationInfo m_allocation_info;
    VkImage m_image;
    VkFormat m_format;
    VkImageView m_image_view;
    std::string m_name;

public:
    /// @brief Default constructor.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param format [in] The color format.
    /// @param image_usage [in] The image usage flags.
    /// @param aspect_flags [in] The aspect flags.
    /// @param sample_count [in] The sample count.
    /// @param name [in] The internal debug marker name of the VkImage.
    /// @param image_extent [in] The width and height of the image.
    Image(const Device &device, const VkFormat format, const VkImageUsageFlags image_usage,
          const VkImageAspectFlags aspect_flags, const VkSampleCountFlagBits sample_count, const std::string &name,
          const VkExtent2D image_extent);
    Image(const Image &) = delete;
    Image(Image &&) noexcept;
    ~Image();

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) = default;

    [[nodiscard]] VkFormat image_format() const {
        return m_format;
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_image_view;
    }

    [[nodiscard]] VkImage get() const {
        return m_image;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
