#pragma once

#include <vma/vma_usage.h>

#include <cstddef>
#include <string>

namespace inexor::vulkan_renderer {

/// @class Buffer
/// @brief An abstraction class for the creation of staging buffers.
/// @note We can't add a std::mutex in here because that would force Buffer to be uncopyable!
struct Buffer {
    // TODO: Inherit to make access unified in syntax!
    // TODO: Map/Unmap memory.

    std::string name;

    VkBuffer buffer = VK_NULL_HANDLE;

    VmaAllocation allocation = VK_NULL_HANDLE;

    VmaAllocationInfo allocation_info = {};

    VkBufferCreateInfo create_info = {};

    VmaAllocationCreateInfo allocation_create_info = {};
};

} // namespace inexor::vulkan_renderer
