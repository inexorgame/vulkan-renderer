#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_pool_allocator.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorPoolAllocator::DescriptorPoolAllocator(const core::Device &device,
                                                 std::vector<VkDescriptorPoolSize> pool_sizes,
                                                 std::uint32_t max_descriptor_count)
    : m_device(device), m_pool_sizes(std::move(pool_sizes)), m_max_descriptor_count(max_descriptor_count) {
    if (m_pool_sizes.empty()) {
        m_pool_sizes = {
            {
                .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .descriptorCount = 4096,
            },
            {
                .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .descriptorCount = 4096,
            },
        };
    }
}

DescriptorPoolAllocator::DescriptorPoolAllocator(DescriptorPoolAllocator &&other) noexcept : m_device(other.m_device) {
    m_pools = std::move(other.m_pools);
    m_pool_sizes = std::move(other.m_pool_sizes);
    m_max_descriptor_count = other.m_max_descriptor_count;
}

VkDescriptorPool DescriptorPoolAllocator::request_new_descriptor_pool() {
    // This might fail because there's not enough memory left for creating the new descriptor pool
    // In this case, DescriptorPool wrapper will throw a VulkanException
    return m_pools.emplace_back(m_device, m_pool_sizes, m_max_descriptor_count, "descriptor pool").descriptor_pool();
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
