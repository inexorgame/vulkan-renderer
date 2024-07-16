#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_update_builder.hpp"

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorSetUpdateBuilder::DescriptorSetUpdateBuilder(const Device &device) : m_device(device) {}

DescriptorSetUpdateBuilder::DescriptorSetUpdateBuilder(DescriptorSetUpdateBuilder &&other) noexcept
    : m_device(other.m_device) {
    // TODO: Implement me!
}

DescriptorSetUpdateBuilder &DescriptorSetUpdateBuilder::add_uniform_buffer_update(const VkDescriptorSet descriptor_set,
                                                                                  std::weak_ptr<Buffer> buffer) {
    if (buffer.lock()->m_buffer_type != BufferType::UNIFORM_BUFFER) {
        throw std::invalid_argument("[DescriptorSetUpdateBuilder::add_uniform_buffer_update] Error: Buffer " +
                                    buffer.lock()->m_name + " is not a uniform buffer!");
    }
    m_write_sets.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstSet = descriptor_set,
        .dstBinding = m_binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &buffer.lock()->m_descriptor_buffer_info,
    }));

    m_binding++;
    return *this;
}

DescriptorSetUpdateBuilder &
DescriptorSetUpdateBuilder::add_combined_image_sampler_update(const VkDescriptorSet descriptor_set,
                                                              std::weak_ptr<Texture> texture) {
    m_write_sets.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstSet = descriptor_set,
        .dstBinding = m_binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &texture.lock()->m_descriptor_img_info,
    }));

    m_binding++;
    return *this;
}

void DescriptorSetUpdateBuilder::update() {
    vkUpdateDescriptorSets(m_device.device(), static_cast<std::uint32_t>(m_write_sets.size()), m_write_sets.data(), 0,
                           nullptr);
    m_write_sets.clear();
    m_binding = 0;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
