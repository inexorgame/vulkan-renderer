#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

WriteDescriptorSetBuilder::WriteDescriptorSetBuilder(const Device &device) : m_device(device), m_binding(0) {}

WriteDescriptorSetBuilder &WriteDescriptorSetBuilder::add(
    const VkDescriptorSet descriptor_set,
    std::variant<std::weak_ptr<TextureResource>, std::weak_ptr<BufferResource>> descriptor_data,
    std::uint32_t descriptor_count) {
    if (!descriptor_set) {
        throw InexorException("Error: Parameter 'descriptor_set' is invalid!");
    }
    auto write_descriptor_set = wrapper::make_info<VkWriteDescriptorSet>({
        .dstSet = descriptor_set,
        .dstBinding = m_binding,
        .dstArrayElement = 0,
        .descriptorCount = descriptor_count,
    });

    // This short code is presented to you by the magic of variant-based polymorphism :)
    // Handle the variant type (either Texture or Buffer)
    std::visit(
        [&write_descriptor_set](auto &&descriptor) {
            using T = std::decay_t<decltype(descriptor)>;
            if constexpr (std::is_same_v<T, std::weak_ptr<TextureResource>>) {
                if (auto texture = descriptor.lock(); texture) {
                    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                    write_descriptor_set.pImageInfo = &texture->m_descriptor_img_info;
                } else {
                    throw InexorException("Error: Texture is expired!");
                }
            } else if constexpr (std::is_same_v<T, std::weak_ptr<BufferResource>>) {
                if (auto buffer = descriptor.lock(); buffer) {
                    // TODO: Distinguish type by buffer->type! (uniform ? storage? ..)
                    write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                    write_descriptor_set.pBufferInfo = &buffer->m_descriptor_buffer_info;
                } else {
                    throw InexorException("Error: Buffer is expired!");
                }
            } else {
                // TODO: Support more descriptor types
                throw InexorException("Error: Invalid descriptor type in std::variant!");
            }
        },
        descriptor_data);

    m_write_descriptor_sets.push_back(write_descriptor_set);
    m_binding++;
    return *this;
}

std::vector<VkWriteDescriptorSet> WriteDescriptorSetBuilder::build() {
    auto write_descriptor_sets = std::move(m_write_descriptor_sets);
    m_write_descriptor_sets.clear();
    m_binding = 0;
    return write_descriptor_sets;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
