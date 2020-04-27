#pragma once

#include <vma/vma_usage.h>

#include <optional>

namespace inexor::vulkan_renderer {

struct ImageBuffer {
    VmaAllocation allocation = VK_NULL_HANDLE;

    VmaAllocationInfo allocation_info = {};

    VmaAllocationCreateInfo allocation_create_info = {};

    VkImage image = VK_NULL_HANDLE;

    VkImageView image_view = VK_NULL_HANDLE;

    VkFormat format;
};

} // namespace inexor::vulkan_renderer
