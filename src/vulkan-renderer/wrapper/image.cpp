#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Image::Image(const Device &device, const VkImageCreateInfo &img_ci, const VkImageViewCreateInfo &img_view_ci,
             const VmaAllocationCreateInfo &alloc_ci, std::string name)
    : m_device(device), m_format(img_ci.format), m_name(std::move(name)) {
    if (m_name.empty()) {
        throw std::invalid_argument("Error: image name must not be empty!");
    }

    if (const auto result = vmaCreateImage(m_device.allocator(), &img_ci, &alloc_ci, &m_img, &m_alloc, &m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }
    m_device.set_debug_marker_name(&m_img, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_name);
    vmaSetAllocationName(m_device.allocator(), m_alloc, m_name.c_str());

    // Fill in the image that was created and the format of the image
    auto filled_img_view_ci = img_view_ci;
    filled_img_view_ci.image = m_img;
    filled_img_view_ci.format = img_ci.format;

    if (const auto result = vkCreateImageView(m_device.device(), &filled_img_view_ci, nullptr, &m_img_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + m_name + "!", result);
    }
    m_device.set_debug_marker_name(&m_img_view, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, m_name);
}

Image::Image(const Device &device, const VkImageCreateInfo &img_ci, const VkImageViewCreateInfo &img_view_ci,
             std::string name)
    : Image(device, img_ci, img_view_ci,
            {
                .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
                .usage = VMA_MEMORY_USAGE_GPU_ONLY,
            },
            name) {}

Image::Image(const Device &device, const VkImageCreateInfo &img_ci, const VkImageAspectFlags aspect_flags,
             std::string name)
    : Image(device, img_ci,
            make_info<VkImageViewCreateInfo>({
                .image = m_img,
                .viewType = VK_IMAGE_VIEW_TYPE_2D,
                .format = img_ci.format,
                .subresourceRange{
                    .aspectMask = aspect_flags,
                    .levelCount = 1,
                    .layerCount = 1,
                },
            }),
            std::move(name)) {}

Image::Image(const Device &device, const VkImageCreateInfo &img_ci, std::string name)
    : Image(device, img_ci, VK_IMAGE_ASPECT_COLOR_BIT, std::move(name)) {}

Image::Image(Image &&other) noexcept : m_device(other.m_device) {
    m_format = other.m_format;
    m_alloc = other.m_alloc;
    m_alloc_info = other.m_alloc_info;
    m_img = std::exchange(other.m_img, VK_NULL_HANDLE);
    m_img_view = std::exchange(other.m_img_view, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
}

Image::~Image() {
    vkDestroyImageView(m_device.device(), m_img_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_img, m_alloc);
}

} // namespace inexor::vulkan_renderer::wrapper
