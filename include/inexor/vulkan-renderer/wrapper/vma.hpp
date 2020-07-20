#pragma once

#include <vma/vk_mem_alloc.h>

namespace inexor::vulkan_renderer::wrapper {

class VulkanMemoryAllocator {
private:
    VmaAllocator vma_allocator;

public:
    // Delete the copy constructor so Vulkan memory allocator wrappers are move-only objects.
    VulkanMemoryAllocator(const VulkanMemoryAllocator &) = delete;
    VulkanMemoryAllocator(VulkanMemoryAllocator &&other) noexcept;

    /// Delete the copy assignment operator so  Vulkan memory allocator wrappers are move-only objects.
    VulkanMemoryAllocator &operator=(const VulkanMemoryAllocator &) = delete;
    VulkanMemoryAllocator &operator=(VulkanMemoryAllocator &&) noexcept = default;

    /// @brief Creates an instance of Vulkan memory allocator.
    /// @param instance [in] The Vulkan instance associated with the allocator.
    /// @param device [in] The Vulkan device associated with the allocator.
    /// @param graphics_card [in] The graphics card associated with the allocator.
    VulkanMemoryAllocator(const VkInstance instance, const VkDevice device, const VkPhysicalDevice graphics_card);

    ~VulkanMemoryAllocator();

    [[nodiscard]] VmaAllocator get_allocator() const {
        return vma_allocator;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
