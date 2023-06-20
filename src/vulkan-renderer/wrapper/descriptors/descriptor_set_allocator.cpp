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

    // Attempt to allocate a new descriptor set from the current descriptor pool
    VkDescriptorSet new_descriptor_set = VK_NULL_HANDLE;
    auto result = vkAllocateDescriptorSets(m_device.device(), &descriptor_set_ai, &new_descriptor_set);

    // Do not throw an exception rightaway if this attempt to allocate failed
    // It might be the case that we simply ran out of pool memory for the allocation
    if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        // The allocation failed in the first attempt because we did run out of descriptor pool memory!
        // We still have a chance to recover from this: Create a new descriptor pool and then try again!
        m_current_pool = m_descriptor_pool_allocator.request_new_descriptor_pool();
        // Don't forget we are using the new descriptor pool here that we just created
        descriptor_set_ai.descriptorPool = m_current_pool;
        // Try again with the new descriptor pool that was just created
        result = vkAllocateDescriptorSets(m_device.device(), &descriptor_set_ai, &new_descriptor_set);
    }

    if (result != VK_SUCCESS) {
        // All attempts failed, but it's not because we did run out of descriptor pool memory
        // If this happens, we have a huge problem and here's nothing we can do anymore
        // This is a hint that there is something fundamentally wrong with our descriptor management in the engine!
        throw VulkanException("Error: All attempts to call vkAllocateDescriptorSets failed!", result);
    }

    // At this point, the allocation did work successfully either because we had enough memory in the first attempt to
    // call vkAllocateDescriptorSets or it worked on the second attempt because we created a new descriptor pool
    return new_descriptor_set;
}

[[nodiscard]] std::vector<VkDescriptorSet>
DescriptorSetAllocator::allocate_descriptor_sets(const std::vector<VkDescriptorSetLayout> &descriptor_set_layouts) {
    if (descriptor_set_layouts.empty()) {
        throw std::invalid_argument("Error: descriptor_set_layouts must not be an empty vector!");
    }
    const auto descriptor_set_ai = make_info<VkDescriptorSetAllocateInfo>({
        .descriptorPool = m_current_pool,
        .descriptorSetCount = static_cast<std::uint32_t>(descriptor_set_layouts.size()),
        .pSetLayouts = descriptor_set_layouts.data(),
    });

    // Attempt to batch calls to vkAllocateDescriptorSets!
    std::vector<VkDescriptorSet> new_descriptor_sets(descriptor_set_layouts.size());
    const auto result = vkAllocateDescriptorSets(m_device.device(), &descriptor_set_ai, new_descriptor_sets.data());
    if (result == VK_SUCCESS) {
        // The allocation worked and we didn't run out of descriptor pool memory because of our batched call
        // No new descriptor pool was created
        return new_descriptor_sets;
    }

    // Batching might not have worked because if we try to allocate all required descriptor sets by using the rest of
    // the memory from the current descriptor pool, we run out of descriptor pool memory
    // Since this might not have been the issue if we wouldn't have batched the call to vkAllocateDescriptorSets, we
    // create each descriptor set separately as a fallback solution
    // This will handle out of pool memory errors by attempting to allocate with a new descriptor pool (see
    // allocate_descriptor_set)
    else if (result == VK_ERROR_OUT_OF_POOL_MEMORY || result == VK_ERROR_FRAGMENTED_POOL) {
        spdlog::warn("Attempt to batch call to vkAllocateDescriptorSets with {} descriptor set layouts failed because "
                     "there is not enough memory in the descriptor pool left to do a batched allocation",
                     descriptor_set_layouts.size());
        spdlog::warn("Attempting to create each of the {} descriptor set separately", descriptor_set_layouts.size());

        for (const auto &descriptor_set_layout : descriptor_set_layouts) {
            // Create each descriptor set separately and handle out of pool errors by allocating a new descriptor pool
            new_descriptor_sets.push_back(allocate_descriptor_set(descriptor_set_layout));
        }
    } else {
        // Something has gone horribly wrong!
        // If this happens, we have a huge problem and here's nothing we can do anymore
        // This is a hint that there is something fundamentally wrong with our descriptor management in the engine!
        throw VulkanException("Error: Attempt to batch call to vkAllocateDescriptorSets failed, but not because of "
                              "VK_ERROR_OUT_OF_POOL_MEMORY or VK_ERROR_FRAGMENTED_POOL!",
                              result);
    }

    // Just to be absolutely on the safe side here (this should never happen, but you never know)
    if (new_descriptor_sets.size() != descriptor_set_layouts.size()) {
        std::runtime_error("Error: For unknown reasons, the size of the vector of created descriptor sets does not "
                           "match the size of specified descriptor set layouts!");
    }

    // We managed to create all required descriptor sets, but we were not able to batch the call to
    // vkAllocateDescriptorSets It is very likely at least one new descriptor pool has been created at this point
    // because of this
    return new_descriptor_sets;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
