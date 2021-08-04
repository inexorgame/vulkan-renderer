#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

DescriptorBuilder::DescriptorBuilder(const Device &device, const std::uint32_t swapchain_image_count)
    : m_device(device), m_swapchain_image_count(swapchain_image_count) {
    assert(m_device.device());
    assert(m_swapchain_image_count > 0);
}

std::vector<ResourceDescriptor> DescriptorBuilder::build(std::string name) {
    assert(!m_layout_bindings.empty());
    assert(!m_write_sets.empty());
    assert(!name.empty());
    assert(m_write_sets.size() == m_layout_bindings.size());

    std::vector<ResourceDescriptor> generated_descriptors;
    generated_descriptors.reserve(m_layout_bindings.size());

    for (std::size_t i = 0; i < m_layout_bindings.size(); i++) {
        generated_descriptors.emplace_back(m_device, m_swapchain_image_count, m_layout_bindings[i], m_write_sets[i],
                                           name);
    }

    m_layout_bindings.clear();
    m_write_sets.clear();
    m_descriptor_buffer_infos.clear();
    m_descriptor_image_infos.clear();
    m_binding = 0;

    return generated_descriptors;
}

DescriptorBuilder &DescriptorBuilder::add_combined_image_sampler(const VkSampler image_sampler,
                                                                 const VkImageView image_view,
                                                                 const VkShaderStageFlagBits shader_stage) {
    assert(image_sampler);
    assert(image_view);

    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = m_binding;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = shader_stage;

    m_layout_bindings.push_back(layout_binding);

    VkDescriptorImageInfo image_info{};
    image_info.sampler = image_sampler;
    image_info.imageView = image_view;
    image_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    m_descriptor_image_infos.push_back(image_info);

    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = nullptr;
    descriptor_write.dstBinding = m_binding;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = &m_descriptor_image_infos.back();

    m_write_sets.push_back(descriptor_write);

    m_binding++;

    return *this;
}

DescriptorBuilder &DescriptorBuilder::add_combined_image_sampler(const wrapper::GpuTexture &texture) {
    return add_combined_image_sampler(texture.sampler(), texture.image_view());
}

DescriptorBuilder &DescriptorBuilder::add_combined_image_samplers(const std::vector<wrapper::GpuTexture> &textures) {
    for (const auto &texture : textures) {
        const auto &result = add_combined_image_sampler(texture.sampler(), texture.image_view());
    }

    return *this;
}

} // namespace inexor::vulkan_renderer::wrapper
