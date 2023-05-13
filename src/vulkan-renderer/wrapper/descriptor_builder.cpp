#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

DescriptorBuilder::DescriptorBuilder(const Device &device) : m_device(device) {
    assert(m_device.device());
}

ResourceDescriptor DescriptorBuilder::build(std::string name) {
    assert(!m_layout_bindings.empty());
    assert(!m_write_sets.empty());
    assert(m_write_sets.size() == m_layout_bindings.size());

    // Generate a new resource descriptor.
    ResourceDescriptor generated_descriptor(m_device, std::move(m_layout_bindings), std::move(m_write_sets),
                                            std::move(name));

    m_descriptor_buffer_infos.clear();
    m_descriptor_image_infos.clear();

    return std::move(generated_descriptor);
}

DescriptorBuilder &DescriptorBuilder::add_combined_image_sampler(const VkSampler image_sampler,
                                                                 const VkImageView image_view,
                                                                 const std::uint32_t binding,
                                                                 const VkShaderStageFlagBits shader_stage) {
    assert(image_sampler);
    assert(image_view);

    m_layout_bindings.push_back({
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = static_cast<VkShaderStageFlags>(shader_stage),
    });

    m_descriptor_image_infos.push_back({
        .sampler = image_sampler,
        .imageView = image_view,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    });

    m_write_sets.push_back(make_info<VkWriteDescriptorSet>({
        .dstSet = nullptr,
        .dstBinding = binding,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = &m_descriptor_image_infos.back(),
    }));

    return *this;
}

} // namespace inexor::vulkan_renderer::wrapper
