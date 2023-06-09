#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_pool.hpp"

#include <volk.h>

#include <vector>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
}

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Forward declaration
class DescriptorSetAllocator;

/// Allocator for DescriptorPool
class DescriptorPoolAllocator {
    friend DescriptorSetAllocator;

private:
    /// The device wrapper
    const Device &m_device;
    /// The descriptor pools
    std::vector<DescriptorPool> m_pools;
    /// We count how many pools are in use and it we run out of pools, we just allocate more!
    std::size_t m_pool_use_counter{0};

    /// Default constructor
    DescriptorPoolAllocator(const Device &device);

    /// Return a descriptor pool from ``m_pools``. If all pools are used up, create a new one
    /// @note If we run out of descriptor pools, we simply create one new descriptor pool (not multiple ones!)
    /// @return A descriptor pool that has not been used yet
    [[nodiscard]] VkDescriptorPool request_descriptor_pool();

public:
    DescriptorPoolAllocator(const DescriptorPoolAllocator &) = delete;
    DescriptorPoolAllocator(DescriptorPoolAllocator &&) noexcept;
    ~DescriptorPoolAllocator() = default;

    DescriptorPoolAllocator &operator=(const DescriptorPoolAllocator &) = delete;
    DescriptorPoolAllocator &operator=(DescriptorPoolAllocator &&) noexcept;
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
