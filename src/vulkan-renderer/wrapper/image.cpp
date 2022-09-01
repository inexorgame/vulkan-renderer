#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Image::Image(const Device &device, const VkFormat format, const VkImageUsageFlags image_usage,
             const VkImageAspectFlags aspect_flags, const VkSampleCountFlagBits sample_count, const std::string &name,
             const VkExtent2D image_extent)
    : m_device(device), m_format(format), m_name(name) {
    assert(device.device());
    assert(device.physical_device());
    assert(device.allocator());
    assert(image_extent.width > 0);
    assert(image_extent.height > 0);
    assert(!name.empty());

    auto image_ci = make_info<VkImageCreateInfo>();
    image_ci.imageType = VK_IMAGE_TYPE_2D;
    image_ci.extent.width = image_extent.width;
    image_ci.extent.height = image_extent.height;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = 1;
    image_ci.arrayLayers = 1;
    image_ci.format = format;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_ci.usage = image_usage;
    image_ci.samples = sample_count;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo vma_allocation_ci{};
    vma_allocation_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    vma_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;

    if (const auto result = vmaCreateImage(m_device.allocator(), &image_ci, &vma_allocation_ci, &m_image, &m_allocation,
                                           &m_allocation_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_name);

    auto image_view_ci = make_info<VkImageViewCreateInfo>();
    image_view_ci.image = m_image;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = format;
    image_view_ci.subresourceRange.aspectMask = aspect_flags;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount = 1;

    m_device.create_image_view(image_view_ci, &m_image_view, m_name);
}

Image::Image(Image &&other) noexcept : m_device(other.m_device) {
    m_allocation = other.m_allocation;
    m_allocation_info = other.m_allocation_info;
    m_image = other.m_image;
    m_format = other.m_format;
    m_image_view = other.m_image_view;
    m_name = std::move(other.m_name);
}

Image::~Image() {
    vkDestroyImageView(m_device.device(), m_image_view, nullptr);
    vmaDestroyImage(m_device.allocator(), m_image, m_allocation);
}

} // namespace inexor::vulkan_renderer::wrapper
