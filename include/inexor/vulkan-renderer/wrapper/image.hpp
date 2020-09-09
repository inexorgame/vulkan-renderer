#pragma once

#include <vma/vk_mem_alloc.h>

#include <cassert>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

class Image {
private:
    const wrapper::Device &m_device;
    VmaAllocation m_allocation;
    VmaAllocationInfo m_allocation_info;
    VkImage m_image;
    VkFormat m_format;
    VkImageView m_image_view;
    std::string m_name;

public:
    /// @brief Creates an image and a corresponding image view.
    /// @param device [in] A reference to the device wrapper.
    /// @param format [in] The image format.
    /// @param image_usage [in] The image usage flags.
    /// @param aspect_flags [in] The image aspect flags for the image view.
    /// @param sample_count [in] The sample count, mostly 1 if multisampling for this image is disabled.
    /// @param name [in] The internal name of this image.
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
