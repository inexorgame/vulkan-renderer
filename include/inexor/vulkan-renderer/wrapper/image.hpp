#pragma once

#include <vk_mem_alloc.h>

#include <cassert>
#include <string>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for VkImage.
class Image {
private:
    const Device &m_device;
    VmaAllocation m_allocation{VK_NULL_HANDLE};
    VmaAllocationInfo m_allocation_info{};
    VkImage m_image{VK_NULL_HANDLE};
    VkFormat m_format{VK_FORMAT_UNDEFINED};
    VkImageView m_image_view{VK_NULL_HANDLE};
    VkImageLayout m_image_layout{VK_IMAGE_LAYOUT_UNDEFINED};
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
    Image(const Device &device, VkImageCreateFlags image_create_flags, VkImageType image_type, VkFormat format,
          std::uint32_t width, std::uint32_t height, std::uint32_t miplevel_count, std::uint32_t array_layer_count,
          VkSampleCountFlagBits sample_count, VkImageUsageFlags image_usage_flags, VkImageViewType image_view_type,
          VkComponentMapping view_components, VkImageAspectFlags image_aspect_flags, std::string name);

    /// @brief Call constructor #2, but don't expose ``view_components``.
    Image(const Device &device, VkImageCreateFlags image_create_flags, VkImageType image_type, VkFormat format,
          std::uint32_t width, std::uint32_t height, std::uint32_t miplevel_count, std::uint32_t array_layer_count,
          VkSampleCountFlagBits sample_count, VkImageUsageFlags image_usage_flags, VkImageViewType image_view_type,
          VkImageAspectFlags image_aspect_flags, std::string name);

    /// @brief Call constructor #2, but don't expose ``flags``.
    Image(const Device &device, VkImageType image_type, VkFormat format, std::uint32_t width, std::uint32_t height,
          std::uint32_t miplevel_count, std::uint32_t array_layer_count, VkSampleCountFlagBits sample_count_flags,
          VkImageUsageFlags image_usage_flags, VkImageViewType image_view_type, VkImageAspectFlags image_aspect_flags,
          std::string name);

    /// @brief Call constructor #2, but don't expose ``image_type``, ``miplevel_count``, ``layer_count``,
    /// ``sample_count``, and ``view_type``.
    Image(const Device &device, VkFormat format, std::uint32_t width, std::uint32_t height,
          VkImageUsageFlags image_usage_flags, VkImageAspectFlags image_aspect_flags, std::string name);

    ///
    Image(const Device &device, VkImageCreateFlags image_create_flags, VkFormat format, std::uint32_t width,
          std::uint32_t height, std::uint32_t miplevel_count, std::uint32_t array_layer_count,
          VkSampleCountFlagBits sample_count, VkImageUsageFlags image_usage, std::string name);

    Image(const Image &) = delete;
    Image(Image &&) noexcept;

    ~Image();

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) = delete;

    void transition_image_layout(VkCommandBuffer cmd_buf, VkImageLayout new_layout);
    void transition_image_layout(VkImageLayout new_layout);

    void place_pipeline_barrier(VkImageLayout new_layout, VkAccessFlags src_access_mask, VkAccessFlags dest_access_mask,
                                VkImageSubresourceRange subresource_range);

    /// @brief
    /// @param command_buffer
    /// @param new_layout
    /// @param src_access_mask
    /// @param dest_access_mask
    void place_pipeline_barrier(VkCommandBuffer command_buffer, VkImageLayout new_layout, VkAccessFlags src_access_mask,
                                VkAccessFlags dest_access_mask, VkImageSubresourceRange subresource_range);

    /// @brief
    /// @param command_buffer
    /// @param buffer
    /// @param width
    /// @param height
    void copy_from_buffer(VkCommandBuffer command_buffer, VkBuffer src_buffer, std::uint32_t width,
                          std::uint32_t height);

    /// @brief
    /// @param command_buffer
    /// @param target_image
    /// @param width
    /// @param height
    /// @param miplevel_count
    /// @param layer_count
    /// @param base_array_layer
    /// @param mip_level
    void copy_from_image(VkCommandBuffer command_buffer, Image &image, std::uint32_t width, std::uint32_t height,
                         std::uint32_t miplevel_count, std::uint32_t layer_count, std::uint32_t base_array_layer,
                         std::uint32_t mip_level);

    [[nodiscard]] VkFormat image_format() const {
        return m_format;
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_image_view;
    }

    [[nodiscard]] VkImage image() const {
        return m_image;
    }

    [[nodiscard]] VkImageLayout image_layout() const {
        return m_image_layout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
