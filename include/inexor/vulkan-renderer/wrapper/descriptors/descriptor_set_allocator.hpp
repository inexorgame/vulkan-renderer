#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_pool_allocator.hpp"

#include <volk.h>

#include <array>
#include <vector>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
}

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Forward declaration
class DescriptorBuilder;

/// This classes manages descriptors by allocating VkDescriptorPools and VkDescriptorSetLayouts.
/// It is also responsible for caching VkDescriptorSetLayouts, meaning we do not create duplicates
/// For internal use inside of rendergraph only!
class DescriptorSetAllocator {
    friend DescriptorBuilder;

private:
    /// The device wrapper
    const Device &m_device;
    // The descriptor pool currently in use (handled by a DescriptorPool instance)
    VkDescriptorPool m_current_pool{VK_NULL_HANDLE};
    /// The descriptor pool allocator
    DescriptorPoolAllocator m_descriptor_pool_allocator;

public:
    /// Default constructor
    /// @note This is private because descriptor allocators are for internal use in rendergraph only!
    /// @param device The device wrapper
    DescriptorSetAllocator(const Device &device);

    /// Allocate a new descriptor set
    /// @note We are currently not batching calls vkAllocateDescriptorSets, which would allow multiple descriptor sets
    /// to be allcoated in one vkAllocateDescriptorSets call. The problem is that batching could lead to running out of
    /// memory in the VkDescriptorPool, so a new descriptor pool would be created.
    /// @return The descriptor set which was allocated
    [[nodiscard]] VkDescriptorSet allocate_descriptor_set(VkDescriptorSetLayout descriptor_set_layout);

    DescriptorSetAllocator(const DescriptorSetAllocator &) = delete;
    DescriptorSetAllocator(DescriptorSetAllocator &&) noexcept;
    ~DescriptorSetAllocator() = default;

    DescriptorSetAllocator &operator=(const DescriptorSetAllocator &) = delete;
    DescriptorSetAllocator &operator=(DescriptorSetAllocator &&) = delete;
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
