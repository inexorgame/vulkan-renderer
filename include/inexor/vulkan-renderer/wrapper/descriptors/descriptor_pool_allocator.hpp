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

    /// Default constructor
    /// @param device The device wrapper
    explicit DescriptorPoolAllocator(const core::Device &device);

    /// Return a descriptor pool from ``m_pools`` and in case all pools are used up, create a new one
    /// @note If we run out of descriptor pools, we simply create one new descriptor pool (not multiple ones!)
    /// @return A new descriptor pool that has not been used yet
    [[nodiscard]] VkDescriptorPool request_new_descriptor_pool();

public:
    DescriptorPoolAllocator(const DescriptorPoolAllocator &) = delete;
    DescriptorPoolAllocator(DescriptorPoolAllocator &&) noexcept;

    ~DescriptorPoolAllocator() = default;

    DescriptorPoolAllocator &operator=(const DescriptorPoolAllocator &) = delete;
    // TODO: Implement me!
    DescriptorPoolAllocator &operator=(DescriptorPoolAllocator &&) noexcept;
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
