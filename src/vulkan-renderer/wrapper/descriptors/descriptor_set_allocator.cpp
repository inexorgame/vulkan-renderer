#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorSetAllocator::DescriptorSetAllocator(const Device &device)
    : m_device(device), m_descriptor_pool_allocator(device) {
    m_current_pool = m_descriptor_pool_allocator.request_new_descriptor_pool();
    if (m_current_pool == VK_NULL_HANDLE) {
        throw std::runtime_error("Error: Could not create initial descriptor pool!");
    }
}

DescriptorSetAllocator::DescriptorSetAllocator(DescriptorSetAllocator &&other) noexcept
    : m_device(other.m_device), m_descriptor_pool_allocator(std::move(other.m_descriptor_pool_allocator)) {
    m_current_pool = std::exchange(other.m_current_pool, VK_NULL_HANDLE);
}

VkDescriptorSet DescriptorSetAllocator::allocate_descriptor_set(const VkDescriptorSetLayout descriptor_set_layout) {
    auto descriptor_set_ai = make_info<VkDescriptorSetAllocateInfo>({
        .descriptorPool = m_current_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptor_set_layout,
    });

    // Attempt to allocate a new descriptor set with the current pool
    VkDescriptorSet new_descriptor_set = VK_NULL_HANDLE;
    auto result = vkAllocateDescriptorSets(m_device.device(), &descriptor_set_ai, &new_descriptor_set);
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        // The allocation failed in the first attempt because we did run out of descriptor pool memory!
        // Create a new descriptor pool and then try again!
        m_current_pool = m_descriptor_pool_allocator.request_new_descriptor_pool();
        // Don't forget we are using the new descriptor pool here!
        descriptor_set_ai.descriptorPool = m_current_pool;
        // Try again with the new descriptor pool that was just created
        result = vkAllocateDescriptorSets(m_device.device(), &descriptor_set_ai, &new_descriptor_set);
    }

    // At this point, the allocation should have worked either because we had enough memory in the first attempt
    // or because we created a new descriptor pool and it worked on the second attempt
    if (result != VK_SUCCESS) {
        // The allocation failed, but not because we did run out of descriptor pool memory
        // If this happens, we have a huge problem and here's nothing we can do anymore
        throw VulkanException("Error: vkAllocateDescriptorSet failed!", result);
    }
    return new_descriptor_set;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
