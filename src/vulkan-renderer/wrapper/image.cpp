#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

void Image::create_image(const VkImageCreateInfo image_ci) {
    VmaAllocationCreateInfo vma_allocation_ci{};
    vma_allocation_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

#if VMA_RECORDING_ENABLED
    vma_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    vma_allocation_ci.pUserData = m_name.data();
#else
    vma_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif

    if (const auto result = vmaCreateImage(m_device.allocator(), &image_ci, &vma_allocation_ci, &m_image, &m_allocation,
                                           &m_allocation_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateImage failed for image " + m_name + "!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_image, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, m_name);
}

void Image::create_image_view(const VkImageViewCreateInfo image_view_ci) {
    if (const auto result = vkCreateImageView(m_device.device(), &image_view_ci, nullptr, &m_image_view);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateImageView failed for image view " + m_name + "!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_image_view, VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, m_name);
}

Image::Image(const Device &device, const VkImageCreateInfo image_ci, const VkImageViewCreateInfo image_view_ci,
             std::string name)

    : m_device(device), m_format(image_ci.format), m_name(std::move(name)) {

    assert(device.device());
    assert(device.physical_device());
    assert(device.allocator());

    create_image(image_ci);
    create_image_view(image_view_ci);
}

Image::Image(const Device &device, const VkImageCreateFlags flags, const VkImageType image_type, const VkFormat format,
             const std::uint32_t width, const std::uint32_t height, const std::uint32_t miplevel_count,
             const std::uint32_t layer_count, const VkSampleCountFlagBits sample_count,
             const VkImageUsageFlags usage_flags, const VkImageViewType view_type,
             const VkComponentMapping view_components, const VkImageAspectFlags aspect_mask, std::string name)

    : m_device(device), m_format(format), m_name(std::move(name)) {

    assert(device.device());
    assert(device.physical_device());
    assert(device.allocator());
    assert(layer_count > 0);
    assert(miplevel_count > 0);
    assert(width > 0);
    assert(height > 0);
    assert(!m_name.empty());

    auto image_ci = make_info<VkImageCreateInfo>();
    image_ci.flags = flags;
    image_ci.imageType = image_type;
    image_ci.format = format;
    image_ci.extent.width = width;
    image_ci.extent.height = height;
    image_ci.extent.depth = 1;
    image_ci.mipLevels = miplevel_count;
    image_ci.arrayLayers = layer_count;
    image_ci.samples = sample_count;
    image_ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_ci.usage = usage_flags;
    image_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    image_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    create_image(image_ci);

    auto image_view_ci = make_info<VkImageViewCreateInfo>();
    image_view_ci.image = m_image;
    image_view_ci.viewType = view_type;
    image_view_ci.format = format;
    image_view_ci.components = view_components;
    image_view_ci.subresourceRange.aspectMask = aspect_mask;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = miplevel_count;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount = layer_count;

    create_image_view(image_view_ci);
}

Image::Image(const Device &device, const VkImageCreateFlags flags, const VkImageType image_type, const VkFormat format,
             const std::uint32_t width, const std::uint32_t height, const std::uint32_t miplevel_count,
             const std::uint32_t layer_count, const VkSampleCountFlagBits sample_count,
             const VkImageUsageFlags usage_flags, const VkImageViewType view_type, const VkImageAspectFlags aspect_mask,
             std::string name)
    : Image(device, flags, image_type, format, width, height, miplevel_count, layer_count, sample_count, usage_flags,
            view_type, {}, aspect_mask, name) {}

Image::Image(const Device &device, const VkImageType image_type, const VkFormat format, const std::uint32_t width,
             const std::uint32_t height, const std::uint32_t miplevel_count, const std::uint32_t layer_count,
             const VkSampleCountFlagBits sample_count, const VkImageUsageFlags usage_flags,
             const VkImageViewType view_type, const VkImageAspectFlags aspect_mask, std::string name)
    : Image(device, {}, image_type, format, width, height, miplevel_count, layer_count, sample_count, usage_flags,
            view_type, {}, aspect_mask, name) {}

Image::Image(const Device &device, const VkFormat format, const std::uint32_t width, const std::uint32_t height,
             const VkImageUsageFlags usage_flags, const VkImageAspectFlags aspect_mask, std::string name)
    : Image(device, {}, VK_IMAGE_TYPE_2D, format, width, height, 1, 1, VK_SAMPLE_COUNT_1_BIT, usage_flags,
            VK_IMAGE_VIEW_TYPE_2D, {}, aspect_mask, name) {}

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
