#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_pool_allocator.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorPoolAllocator::DescriptorPoolAllocator(const Device &device) : m_device(device) {}

DescriptorPoolAllocator::DescriptorPoolAllocator(DescriptorPoolAllocator &&other) noexcept : m_device(other.m_device) {
    m_pools = std::move(other.m_pools);
}

VkDescriptorPool DescriptorPoolAllocator::request_new_descriptor_pool() {
    // When creating a new descriptor pool, we specify a maximum of 1024 descriptor sets to be used
    const std::uint32_t DEFAULT_MAX_DESCRIPTOR_COUNT{1024};

    /// When creating a new descriptor pool we use these sizes
    /// TODO: Adapt to other pool types as needed in the future!
    const std::vector<VkDescriptorPoolSize> DEFAULT_POOL_SIZES{
        {
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1024,
        },
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1024,
        },
    };

    // Note that this emplace back may fail because there's not enough memory left for creating the new descriptor
    // pool In that case, DescriptorPool wrapper will throw a VulkanException
    m_pools.emplace_back(m_device, DEFAULT_POOL_SIZES, DEFAULT_MAX_DESCRIPTOR_COUNT, "descriptor pool");
    return m_pools.back().descriptor_pool();
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
