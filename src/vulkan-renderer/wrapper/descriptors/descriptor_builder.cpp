#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

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
}

DescriptorBuilder &DescriptorBuilder::bind_uniform_buffer(const VkDescriptorBufferInfo *buffer_info,
                                                          const std::uint32_t binding,
                                                          const VkShaderStageFlags shader_stage) {
    m_bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = shader_stage,
    });

    // The dstSet member of VkWriteDescriptorSet will be set in the build method
    m_writes.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = buffer_info,
    }));

    return *this;
}

DescriptorBuilder &DescriptorBuilder::bind_image(const VkDescriptorImageInfo *image_info, const std::uint32_t binding,
                                                 const VkShaderStageFlags shader_stage) {
    m_bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = shader_stage,
    });

    // The dstSet member of VkWriteDescriptorSet will be set in the build method
    m_writes.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstBinding = binding,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = image_info,
    }));

    return *this;
}

VkDescriptorSet DescriptorBuilder::build() {
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
    return descriptor_set;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
