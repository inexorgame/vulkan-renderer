#pragma once

#include <vk_mem_alloc.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// RAII wrapper class for VkImage and the associated VmaAllocation, VmaAllocationInfo, and VkImageView
class Image {
private:
    std::string m_name;
    const Device &m_device;

    VmaAllocation m_allocation{VK_NULL_HANDLE};
    VmaAllocationInfo m_allocation_info{};

    VkImage m_image{VK_NULL_HANDLE};
    VkImageView m_image_view{VK_NULL_HANDLE};
    VkImageCreateInfo m_image_ci;
    VkImageViewCreateInfo m_image_view_ci;

    void create_image();
    void create_image_view();

public:
    // TODO: Make a private member and add get method!
    VkDescriptorImageInfo descriptor_image_info;

    Image(const Device &device, VkImageCreateInfo image_ci, VkImageViewCreateInfo image_view_ci, std::string name);

    Image(const Image &) = delete;
    Image(Image &&) noexcept;
    ~Image();

    Image &operator=(const Image &) = delete;
    Image &operator=(Image &&) noexcept = default;

    [[nodiscard]] VkFormat format() const {
        return m_image_ci.format;
    }

    [[nodiscard]] VkImageView image_view() const {
        return m_image_view;
    }

    [[nodiscard]] VkImage image() const {
        return m_image;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
