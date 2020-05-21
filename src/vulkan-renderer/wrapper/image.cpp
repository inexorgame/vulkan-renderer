#include "inexor/vulkan-renderer/wrapper/image.hpp"

namespace inexor::vulkan_renderer::wrapper {

Image::Image(Image &&other) noexcept
    : device(other.device), vma_allocator(other.vma_allocator), allocation(other.allocation), allocation_info(other.allocation_info), image(other.image),
      format(other.format), image_view(other.image_view), name(std::move(other.name)) {}

Image::Image(const VkDevice device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator, const VkFormat format,
             const VkImageUsageFlags image_usage, const VkImageAspectFlags aspect_flags, const VkSampleCountFlagBits sample_count, const std::string &name,
             const VkExtent2D image_extent)
    : device(device), vma_allocator(vma_allocator), format(format), name(name) {
    assert(device);
    assert(graphics_card);
    assert(vma_allocator);
    assert(image_extent.width > 0);
    assert(image_extent.height > 0);
    assert(!name.empty());

    VkImageCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    create_info.imageType = VK_IMAGE_TYPE_2D;
    create_info.extent.width = image_extent.width;
    create_info.extent.height = image_extent.height;
    create_info.extent.depth = 1;
    create_info.mipLevels = 1;
    create_info.arrayLayers = 1;
    create_info.format = format;
    create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    create_info.usage = image_usage;
    create_info.samples = sample_count;
    create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocation_create_info = {};
    allocation_create_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

#if VMA_RECORDING_ENABLED
    allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    allocation_create_info.pUserData = this->name.data();
#else
    allocation_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif

    if (vmaCreateImage(vma_allocator, &create_info, &allocation_create_info, &image, &allocation, &allocation_info) != VK_SUCCESS) {
        throw std::runtime_error("Error: vmaCreateImage failed for depth buffer images!");
    }

    // TODO: Assign an internal name using Vulkan debug markers.

    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspect_flags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &view_info, nullptr, &image_view) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateImageView failed!");
    }

    // TODO: Assign an internal name using Vulkan debug markers.
}

Image::~Image() {
    vkDestroyImageView(device, image_view, nullptr);
    vmaDestroyImage(vma_allocator, image, allocation);
}

} // namespace inexor::vulkan_renderer::wrapper
