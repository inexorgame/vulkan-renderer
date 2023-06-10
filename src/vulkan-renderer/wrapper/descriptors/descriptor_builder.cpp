#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorBuilder::DescriptorBuilder(const Device &device, DescriptorSetAllocator &descriptor_set_allocator,
                                     DescriptorSetLayoutCache &descriptor_set_layout_cache)
    : m_device(device), m_descriptor_set_allocator(descriptor_set_allocator),
      m_descriptor_set_layout_cache(descriptor_set_layout_cache) {}

DescriptorBuilder::DescriptorBuilder(DescriptorBuilder &&other) noexcept
    : m_device(other.m_device), m_descriptor_set_allocator(other.m_descriptor_set_allocator),
      m_descriptor_set_layout_cache(other.m_descriptor_set_layout_cache) {
    m_writes = std::move(other.m_writes);
    m_bindings = std::move(other.m_bindings);
    m_binding = other.m_binding;
}

DescriptorBuilder &DescriptorBuilder::bind_uniform_buffer(const VkDescriptorBufferInfo *buffer_info,
                                                          const VkShaderStageFlags shader_stage) {
    m_bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = m_binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = shader_stage,
    });

    m_writes.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstSet = nullptr, // This will be set in the build() method
        .dstBinding = m_binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        // It is the responsibility of the caller to keep buffer_info a valid pointer
        .pBufferInfo = buffer_info,
    }));

    // Let's automatically increase the binding index because bindings are unique
    m_binding++;
    return *this;
}

DescriptorBuilder &DescriptorBuilder::bind_image(const VkDescriptorImageInfo *image_info,
                                                 const VkShaderStageFlags shader_stage) {
    m_bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = m_binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = shader_stage,
    });

    m_writes.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstSet = nullptr, // This will be set in the build() method
        .dstBinding = m_binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        // It is the responsibility of the caller to keep image_info a valid pointer
        .pImageInfo = image_info,
    }));

    // Let's automatically increase the binding index because bindings are unique
    m_binding++;
    return *this;
}

std::pair<VkDescriptorSet, VkDescriptorSetLayout> DescriptorBuilder::build() {
    const auto descriptor_set_layout_ci = make_info<VkDescriptorSetLayoutCreateInfo>({
        .bindingCount = static_cast<std::uint32_t>(m_bindings.size()),
        .pBindings = m_bindings.data(),
    });

    // Create the descriptor set layout using the descriptor set layout cache
    const auto descriptor_set_layout =
        m_descriptor_set_layout_cache.create_descriptor_set_layout(descriptor_set_layout_ci);

    // Allocate the descriptor set using the descriptor set allocator
    const auto descriptor_set = m_descriptor_set_allocator.allocate_descriptor_set(descriptor_set_layout);

    // Set the write descriptor sets
    for (auto &write_descriptor : m_writes) {
        write_descriptor.dstSet = descriptor_set;
    }

    vkUpdateDescriptorSets(m_device.device(), static_cast<std::uint32_t>(m_writes.size()), m_writes.data(), 0, nullptr);

    // Clear the builder's data so the builder can be re-used
    m_bindings.clear();
    m_writes.clear();
    m_binding = 0;

    // Return the created descriptor set and the descriptor set layout as a pair
    return std::make_pair(descriptor_set, descriptor_set_layout);
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
