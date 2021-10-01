#pragma once

#include <vk_mem_alloc.h>

#include <cassert>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

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

    /// @brief Call vmaCreateImage and assign an internal debug marker name.
    /// @param image_ci The image create info
    void create_image(VkImageCreateInfo image_ci);

    /// @brief Call vkCreateImageView and assign an internal debug marker name.
    /// @param image_view_ci The image view create info
    void create_image_view(VkImageViewCreateInfo image_view_ci);

public:
    /// @brief This is the most verbose constructor. It offers all create structures as parameters.
    /// @note More constructors with increasing amount of abstraction can be found below. Not that this constructor is
    /// the most explicit because you have to fill out ``VkImageCreateInfo`` and ``VkImageViewCreateInfo`` manually.
    /// @param device The device wrapper
    /// @param image_ci The image create info
    /// @param image_view_ci The image view create info
    /// @param name The internal debug marker name of the image
    /// @note std::move(name) is used since we must take ownership of the memory for assigning a Vulkan debug marker
    Image(const Device &device, VkImageCreateInfo image_ci, VkImageViewCreateInfo image_view_ci, std::string name);

    /// @brief Call the constructor above, but expose most parameters to the programmer.
    /// @param device The device wrapper
    /// @param flags The image creation flags
    /// @param image_type The type of the image
    /// @param format The image format
    /// @param width The width of the image in pixels
    /// @param height The height of the image in pixels
    /// @param miplevel_count The number of mip levels
    /// @param layer_count The number of layers in the image
    /// @param sample_count The number of samples per pixel
    /// @param usage_flags The image usage flags
    /// @param view_type The view type of the image view
    /// @param view_components The component bits of the image view
    /// @param aspect_mask The aspect mask of the image view
    /// @param name The internal debug marker name of the image
    /// @note std::move(name) is used since we must take ownership of the memory for assigning a Vulkan debug marker
    Image(const Device &device, VkImageCreateFlags flags, VkImageType image_type, VkFormat format, std::uint32_t width,
          std::uint32_t height, std::uint32_t miplevel_count, std::uint32_t layer_count,
          VkSampleCountFlagBits sample_count, VkImageUsageFlags usage_flags, VkImageViewType view_type,
          VkComponentMapping view_components, VkImageAspectFlags aspect_mask, std::string name);

    /// @brief Call constructor #2, but don't expose ``view_components``.
    /// @param device The device wrapper
    /// @param flags The image creation flags
    /// @param image_type The type of the image
    /// @param format The image format
    /// @param width The width of the image in pixels
    /// @param height The height of the image in pixels
    /// @param miplevel_count The number of mip levels
    /// @param layer_count The number of layers in the image
    /// @param sample_count The number of samples per pixel
    /// @param usage_flags The image usage flags
    /// @param view_type The view type of the image view
    /// @param aspect_mask The aspect mask of the image view
    /// @param name The internal debug marker name of the image
    /// @note std::move(name) is used since we must take ownership of the memory for assigning a Vulkan debug marker
    Image(const Device &device, VkImageCreateFlags flags, VkImageType image_type, VkFormat format, std::uint32_t width,
          std::uint32_t height, std::uint32_t miplevel_count, std::uint32_t layer_count,
          VkSampleCountFlagBits sample_count, VkImageUsageFlags usage_flags, VkImageViewType view_type,
          VkImageAspectFlags aspect_mask, std::string name);

    /// @brief Call constructor #2, but don't expose ``flags``.
    /// @param device The device wrapper
    /// @param image_type The type of the image
    /// @param format The image format
    /// @param width The width of the image in pixels
    /// @param height The height of the image in pixels
    /// @param miplevel_count The number of mip levels
    /// @param layer_count The number of layers in the image
    /// @param sample_count The number of samples per pixel
    /// @param usage_flags The image usage flags
    /// @param view_type The view type of the image view
    /// @param aspect_mask The aspect mask of the image view
    /// @param name The internal debug marker name of the image
    /// @note std::move(name) is used since we must take ownership of the memory for assigning a Vulkan debug marker
    Image(const Device &device, VkImageType image_type, VkFormat format, std::uint32_t width, std::uint32_t height,
          std::uint32_t miplevel_count, std::uint32_t layer_count, VkSampleCountFlagBits sample_count,
          VkImageUsageFlags usage_flags, VkImageViewType view_type, VkImageAspectFlags aspect_mask, std::string name);

    /// @brief Call constructor #2, but don't expose ``image_type``, ``miplevel_count``, ``layer_count``,
    /// ``sample_count``, and ``view_type``.
    /// @param device The device wrapper
    /// @param format The image format
    /// @param width The width of the image in pixels
    /// @param height The height of the image in pixels
    /// @param usage_flags The image usage flags
    /// @param aspect_mask The aspect mask of the image view
    /// @param name The internal debug marker name of the image
    /// @note std::move(name) is used since we must take ownership of the memory for assigning a Vulkan debug marker
    Image::Image(const Device &device, VkFormat format, std::uint32_t width, std::uint32_t height,
                 VkImageUsageFlags usage_flags, VkImageAspectFlags aspect_mask, std::string name);

    Image(const Image &) = delete;
    Image(Image &&) noexcept;

    ~Image();

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) = delete;

    [[nodiscard]] VkFormat image_format() const {
        return m_format;
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_image_view;
    }

    [[nodiscard]] VkImage image() const {
        return m_image;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
