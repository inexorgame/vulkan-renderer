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

WriteDescriptorSetBuilder &
WriteDescriptorSetBuilder::add_uniform_buffer_update(const VkDescriptorSet descriptor_set,
                                                     const std::weak_ptr<Buffer> uniform_buffer) {

    if (!descriptor_set) {
        throw std::invalid_argument("[WriteDescriptorSetBuilder::add_uniform_buffer_update] Error: Parameter "
                                    "'descriptor_set' is invalid!");
    }
    if (uniform_buffer.lock()->m_buffer_type != BufferType::UNIFORM_BUFFER) {
        throw std::invalid_argument("[DescriptorSetUpdateBuilder::add_uniform_buffer_update] Error: Buffer " +
                                    uniform_buffer.lock()->m_name + " is not a uniform buffer!");
    }
    const auto &buffer = uniform_buffer.lock();
    if (!buffer->m_descriptor_buffer_info.buffer) {
        throw std::invalid_argument("[WriteDescriptorSetBuilder::add_uniform_buffer_update] Error: "
                                    "Buffer::m_descriptor_buffer_info.buffer' of uniform buffer '" +
                                    buffer->m_name + "' is invalid!");
    }
    m_write_descriptor_sets.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstSet = descriptor_set,
        .dstBinding = m_binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .pBufferInfo = &buffer->m_descriptor_buffer_info,
    }));

    m_binding++;
    return *this;
}

WriteDescriptorSetBuilder &
WriteDescriptorSetBuilder::add_combined_image_sampler_update(const VkDescriptorSet descriptor_set,
                                                             const std::weak_ptr<Texture> image_texture) {
    if (!descriptor_set) {
        throw std::invalid_argument("[WriteDescriptorSetBuilder::add_combined_image_sampler_update] Error: Parameter "
                                    "'descriptor_set' is invalid!");
    }
    if (image_texture.expired()) {
        throw std::invalid_argument(
            "[WriteDescriptorSetBuilder::add_combined_image_sampler_update] Error: Parameter 'texture' is invalid!");
    }

    const auto &texture = image_texture.lock();
    if (!texture->m_descriptor_img_info.imageView) {
        throw std::invalid_argument("[WriteDescriptorSetBuilder::add_combined_image_sampler_update] Error: "
                                    "'Texture::m_descriptor_img_info.imageView' of texture '" +
                                    texture->m_name + "' is invalid!");
    }
    if (!texture->m_descriptor_img_info.sampler) {
        throw std::invalid_argument("[WriteDescriptorSetBuilder::add_combined_image_sampler_update] Error: "
                                    "'Texture::m_descriptor_img_info.sampler' of texture '" +
                                    texture->m_name + "' is invalid!");
    }
    m_write_descriptor_sets.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstSet = descriptor_set,
        .dstBinding = m_binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &texture->m_descriptor_img_info,
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
