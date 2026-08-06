#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_pool.hpp"

#include <volk.h>

#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Forward declaration
class DescriptorSetAllocator;

/// Allocator for the DescriptorPool class
class DescriptorPoolAllocator {
    friend DescriptorSetAllocator;

private:
    /// The device wrapper
    const core::Device &m_device;
    /// The descriptor pools
    std::vector<DescriptorPool> m_pools;
    /// The default pool sizes
    std::vector<VkDescriptorPoolSize> m_pool_sizes;
    /// The maximum number of descriptor sets per pool
    std::uint32_t m_max_descriptor_count;

    /// Default constructor
    /// @param device The device wrapper
    /// @param pool_sizes The descriptor pool sizes to use when creating a new pool
    /// @param max_descriptor_count The maximum number of descriptor sets that can be allocated from a single pool
    explicit DescriptorPoolAllocator(const core::Device &device, std::vector<VkDescriptorPoolSize> pool_sizes = {},
                                     std::uint32_t max_descriptor_count = 4096);

    /// Return a descriptor pool from ``m_pools`` and in case all pools are used up, create a new one
    /// @note If we run out of descriptor pools, we simply create one new descriptor pool (not multiple ones!)
    /// @return A new descriptor pool that has not been used yet
    [[nodiscard]] VkDescriptorPool request_new_descriptor_pool();

public:
    DescriptorPoolAllocator(const DescriptorPoolAllocator &) = delete;
    DescriptorPoolAllocator(DescriptorPoolAllocator &&) noexcept;

    ~DescriptorPoolAllocator() = default;

    DescriptorPoolAllocator &operator=(const DescriptorPoolAllocator &) = delete;
    DescriptorPoolAllocator &operator=(DescriptorPoolAllocator &&) = delete;
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
