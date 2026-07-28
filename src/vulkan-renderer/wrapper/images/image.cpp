#include "inexor/vulkan-renderer/wrapper/images/image.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::images {

Image::Image(const Device &device, std::string name) : m_device(device), m_name(std::move(name)) {}

Image::Image(Image &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_img = std::exchange(other.m_img, VK_NULL_HANDLE);
    m_img_view = std::exchange(other.m_img_view, VK_NULL_HANDLE);
    m_alloc = std::exchange(other.m_alloc, VK_NULL_HANDLE);
    m_alloc_info = other.m_alloc_info;
}

Image::~Image() {
    destroy();
}

void Image::create(VkImageCreateInfo img_ci, VkImageViewCreateInfo img_view_ci) {
    m_img_ci = std::move(img_ci);
    m_img_view_ci = std::move(img_view_ci);

    float priority = 0.7f;
    if ((m_img_ci.usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                           VK_IMAGE_USAGE_STORAGE_BIT)) != 0u) {
        priority = 1.0f;
    }

    const VmaAllocationCreateInfo alloc_ci{
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .priority = priority,
    };

    // Create the image
    if (const auto result = vmaCreateImage(m_device.allocator(), &m_img_ci, &alloc_ci, &m_img, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed!", result, m_name);
    }
    m_device.set_debug_name(m_img, m_name);

    // Set the internal debug name of the image in Vulkan Memory Allocator (VMA)
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());

    // Set the image in the VkImageViewCreateInfo
    m_img_view_ci.image = m_img;

    // Create the image view
    if (const auto result = vkCreateImageView(m_device.device(), &m_img_view_ci, nullptr, &m_img_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed!", result, m_name);
    }
    m_device.set_debug_name(m_img_view, m_name);
}

void Image::destroy() {
    if (m_img_view == VK_NULL_HANDLE && m_img == VK_NULL_HANDLE && m_alloc == VK_NULL_HANDLE) {
        return;
    }

    vkDestroyImageView(m_device.device(), m_img_view, nullptr);
    m_img_view = VK_NULL_HANDLE;
    vmaDestroyImage(m_device.allocator(), m_img, m_alloc);
    m_img = VK_NULL_HANDLE;
    m_alloc = VK_NULL_HANDLE;
}

} // namespace inexor::vulkan_renderer::wrapper::images
