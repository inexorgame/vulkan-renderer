#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::descriptors {

WriteDescriptorSetBuilder::WriteDescriptorSetBuilder(const Device &device) : m_device(device) {}

WriteDescriptorSetBuilder::WriteDescriptorSetBuilder(WriteDescriptorSetBuilder &&other) noexcept
    : m_device(other.m_device) {
    // TODO: Implement me!
}

WriteDescriptorSetBuilder &WriteDescriptorSetBuilder::add_uniform_buffer_update(const VkDescriptorSet descriptor_set,
                                                                                const std::weak_ptr<Buffer> buffer) {
    assert(descriptor_set);
    if (buffer.lock()->m_buffer_type != BufferType::UNIFORM_BUFFER) {
        throw std::invalid_argument("[DescriptorSetUpdateBuilder::add_uniform_buffer_update] Error: Buffer " +
                                    buffer.lock()->m_name + " is not a uniform buffer!");
    }
    m_write_descriptor_sets.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
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

WriteDescriptorSetBuilder &
WriteDescriptorSetBuilder::add_combined_image_sampler_update(const VkDescriptorSet descriptor_set,
                                                             const std::weak_ptr<Texture> texture) {
    assert(descriptor_set);
    m_write_descriptor_sets.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
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

std::vector<VkWriteDescriptorSet> WriteDescriptorSetBuilder::build() {
    auto write_descriptor_sets = std::move(m_write_descriptor_sets);
    reset();
    return write_descriptor_sets;
}

void WriteDescriptorSetBuilder::reset() {
    m_write_descriptor_sets.clear();
    m_binding = 0;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
