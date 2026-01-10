#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorSetAllocator::DescriptorSetAllocator(const Device &device)
    : m_device(device), m_descriptor_pool_allocator(device) {
    m_current_pool = m_descriptor_pool_allocator.request_new_descriptor_pool();
    if (m_current_pool == VK_NULL_HANDLE) {
        throw InexorException("Error: Failed to create descriptor pool!");
    }
}

DescriptorSetAllocator::DescriptorSetAllocator(DescriptorSetAllocator &&other) noexcept
    : m_device(other.m_device), m_descriptor_pool_allocator(std::move(other.m_descriptor_pool_allocator)) {
    m_current_pool = std::exchange(other.m_current_pool, VK_NULL_HANDLE);
}

VkDescriptorSet DescriptorSetAllocator::allocate(const std::string &name,
                                                 const VkDescriptorSetLayout descriptor_set_layout) {
    assert(descriptor_set_layout);
    auto descriptor_set_ai = make_info<VkDescriptorSetAllocateInfo>({
        .descriptorPool = m_current_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptor_set_layout,
    });
    // Attempt to allocate a new descriptor set from the current descriptor pool
    VkDescriptorSet new_descriptor_set = VK_NULL_HANDLE;
    auto result = vkAllocateDescriptorSets(m_device.device(), &descriptor_set_ai, &new_descriptor_set);

    // Do not throw an exception rightaway if this attempt to allocate failed
    // It might be the case that we simply ran out of pool memory for the allocation
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        spdlog::trace("Requesting new descriptor pool");
        // The allocation failed in the first attempt because we did run out of descriptor pool memory!
        // We still have a chance to recover from this: Create a new descriptor pool and then try again!
        m_current_pool = m_descriptor_pool_allocator.request_new_descriptor_pool();
        // Don't forget we are using the new descriptor pool here that we just created
        descriptor_set_ai.descriptorPool = m_current_pool;
        // Try again with the new descriptor pool that was just created
        result = vkAllocateDescriptorSets(m_device.device(), &descriptor_set_ai, &new_descriptor_set);
    }
    // This is true if either the first or the second attempt to call vkAllocateDescriptorSets failed
    if (result != VK_SUCCESS) {
        // All attempts failed, but it's not because we did run out of descriptor pool memory
        // If this happens, we have a huge problem and here's nothing we can do anymore
        // This is a hint that there is something fundamentally wrong with our descriptor management in the engine!
        throw VulkanException("Error: All attempts to call vkAllocateDescriptorSets failed!", result);
    }
    // Assign an internal debug name to the descriptor set that was just created
    m_device.set_debug_name(new_descriptor_set, name);
    // At this point, the allocation did work successfully either because we had enough memory in the first attempt to
    // call vkAllocateDescriptorSets or it worked on the second attempt because we created a new descriptor pool
    return new_descriptor_set;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
