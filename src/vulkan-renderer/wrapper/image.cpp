#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

using tools::VulkanException;

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

    const auto image_ci = make_info<VkImageCreateInfo>({
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent{
            .width = image_extent.width,
            .height = image_extent.height,
            .depth = 1,
        },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = sample_count,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = image_usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    });

    const VmaAllocationCreateInfo vma_allocation_ci{
        .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_GPU_ONLY,
    };

    if (const auto result = vmaCreateImage(m_device.allocator(), &image_ci, &vma_allocation_ci, &m_image, &m_allocation,
                                           &m_allocation_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }

    vmaSetAllocationName(m_device.allocator(), m_allocation, m_name.c_str());

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_name);

    m_device.create_image_view(make_info<VkImageViewCreateInfo>({
                                   .image = m_image,
                                   .viewType = VK_IMAGE_VIEW_TYPE_2D,
                                   .format = format,
                                   .subresourceRange{
                                       .aspectMask = aspect_flags,
                                       .baseMipLevel = 0,
                                       .levelCount = 1,
                                       .baseArrayLayer = 0,
                                       .layerCount = 1,
                                   },
                               }),
                               &m_image_view, m_name);
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
