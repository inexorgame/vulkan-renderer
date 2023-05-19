#pragma once

#include <vk_mem_alloc.h>
#include <volk.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// RAII wrapper class for VkImage
class Image {
    const Device &m_device;
    std::string m_name;

    VkFormat m_format{VK_FORMAT_UNDEFINED};
    VmaAllocation m_alloc{VK_NULL_HANDLE};
    VmaAllocationInfo m_alloc_info{};
    VkImage m_img{VK_NULL_HANDLE};
    VkImageView m_img_view{VK_NULL_HANDLE};

public:
    /// Constructor 1 (the most powerful constructor which exposes all parameters, rarely used)
    /// @param device The device wrapper
    /// @param img_ci The image create info
    /// @param img_view_ci The image view create info
    /// @note The parameters .image and .format will be filled automatically by this constructor
    /// @param name The internal debug name of the image and the image view (must not be empty)
    /// @exception std::invalid_argument The internal debug name is empty
    /// @exception VulkanException vmaCreateImage call failed
    /// @exception VulkanException vkCreateImageView call failed
    Image(const Device &device, const VkImageCreateInfo &img_ci, const VkImageViewCreateInfo &img_view_ci,
          const VmaAllocationCreateInfo &alloc_ci, std::string name);

    /// Constructor 2 (calls constructor 1 internally)
    /// @param device The device wrapper
    /// @param img_ci The image create info
    /// @param img_view_ci The image view create info
    /// @note The .image parameter will be filled automatically to the image that was created
    /// @param name The internal debug name of the image and the image view (must not be empty)
    /// @exception std::invalid_argument The internal debug name is empty
    /// @exception VulkanException vmaCreateImage call failed
    /// @exception VulkanException vkCreateImageView call failed
    Image(const Device &device, const VkImageCreateInfo &img_ci, const VkImageViewCreateInfo &img_view_ci,
          std::string name);

    /// Constructor 3 (calls constructor 2 internally)
    /// @note This constructor does not expose image aspect flags (use ``VK_IMAGE_ASPECT_COLOR_BIT`` as default)
    /// @param img_ci The image create info
    Image(const Device &device, const VkImageCreateInfo &img_ci, VkImageAspectFlags aspect_flags, std::string name);

    /// Constructor 4 (calls constructor 3 internally)
    /// @param device The device wrapper
    /// @param img_ci The image create info
    /// @param name The internal debug name of the image and the image view (must not be empty)
    /// @exception std::invalid_argument The internal debug name is empty
    /// @exception VulkanException vmaCreateImage call failed
    /// @exception VulkanException vkCreateImageView call failed
    Image(const Device &device, const VkImageCreateInfo &img_ci, std::string name);

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
