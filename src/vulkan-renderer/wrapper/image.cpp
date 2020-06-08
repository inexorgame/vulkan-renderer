#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

Image::Image(Image &&other) noexcept
    : device(other.device), vma_allocator(other.vma_allocator), allocation(other.allocation),
      allocation_info(other.allocation_info), image(other.image), format(other.format), image_view(other.image_view),
      name(std::move(other.name)) {}

Image::Image(const VkDevice device, const VkPhysicalDevice graphics_card, const VmaAllocator vma_allocator,
             const VkFormat format, const VkImageUsageFlags image_usage, const VkImageAspectFlags aspect_flags,
             const VkSampleCountFlagBits sample_count, const std::string &name, const VkExtent2D image_extent)
    : device(device), vma_allocator(vma_allocator), format(format), name(name) {
    assert(device);
    assert(graphics_card);
    assert(vma_allocator);
    assert(image_extent.width > 0);
    assert(image_extent.height > 0);
    assert(!name.empty());

    VkImageCreateInfo image_ci = {};
    image_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
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

    VmaAllocationCreateInfo vma_allocation_ci = {};
    vma_allocation_ci.usage = VMA_MEMORY_USAGE_GPU_ONLY;

#if VMA_RECORDING_ENABLED
    vma_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT;
    vma_allocation_ci.pUserData = this->name.data();
#else
    vma_allocation_ci.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
#endif

    if (vmaCreateImage(vma_allocator, &image_ci, &vma_allocation_ci, &image, &allocation, &allocation_info) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: vmaCreateImage failed for depth buffer images!");
    }

    // TODO: Assign an internal name using Vulkan debug markers.

    VkImageViewCreateInfo image_view_ci = {};
    image_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    image_view_ci.image = image;
    image_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    image_view_ci.format = format;
    image_view_ci.subresourceRange.aspectMask = aspect_flags;
    image_view_ci.subresourceRange.baseMipLevel = 0;
    image_view_ci.subresourceRange.levelCount = 1;
    image_view_ci.subresourceRange.baseArrayLayer = 0;
    image_view_ci.subresourceRange.layerCount = 1;

    if (vkCreateImageView(device, &image_view_ci, nullptr, &image_view) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateImageView failed!");
    }

    // TODO: Assign an internal name using Vulkan debug markers.
}

Image::~Image() {
    spdlog::trace("Destroying image view {} .", name);
    vkDestroyImageView(device, image_view, nullptr);

    spdlog::trace("Destroying image {} .", name);
    vmaDestroyImage(vma_allocator, image, allocation);
}

} // namespace inexor::vulkan_renderer::wrapper
