#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_pool_allocator.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorPoolAllocator::DescriptorPoolAllocator(const Device &device) : m_device(device) {}

DescriptorPoolAllocator::DescriptorPoolAllocator(DescriptorPoolAllocator &&other) noexcept : m_device(other.m_device) {
    m_pools = std::move(other.m_pools);
    m_pool_use_counter = other.m_pool_use_counter;
}

VkDescriptorPool DescriptorPoolAllocator::request_descriptor_pool() {
    // Are all the descriptor pools which are currently available used up?
    if (m_pool_use_counter == m_pools.size()) {
        // We did run out of pools to use, so create new ones!
        spdlog::trace("Out of descriptor pools ({} in use)! Creating one new descriptor pool", m_pool_use_counter);

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
    }
    m_pool_use_counter++;
    return m_pools[m_pool_use_counter].descriptor_pool();
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
