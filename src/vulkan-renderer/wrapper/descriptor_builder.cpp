#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

DescriptorBuilder::DescriptorBuilder(DescriptorBuilder &&other) noexcept : m_device(other.m_device) {
    m_descriptor_pool = std::exchange(other.m_descriptor_pool, nullptr);
    m_binding = other.m_binding;
    m_layout_bindings = std::move(other.m_layout_bindings);
    m_write_sets = std::move(other.m_write_sets);
    m_descriptor_buffer_infos = std::move(other.m_descriptor_buffer_infos);
    m_descriptor_image_infos = std::move(other.m_descriptor_image_infos);
}

DescriptorBuilder::DescriptorBuilder(const Device &device, const VkDescriptorPool descriptor_pool)
    : m_device(device), m_descriptor_pool(descriptor_pool) {
    assert(m_device.device());
}

std::unique_ptr<ResourceDescriptor> DescriptorBuilder::build(std::string name) {
    assert(!m_layout_bindings.empty());
    assert(!m_write_sets.empty());
    assert(!name.empty());
    assert(m_write_sets.size() == m_layout_bindings.size());

    std::unique_ptr<ResourceDescriptor> generated_descriptor =
        std::make_unique<ResourceDescriptor>(m_device, m_descriptor_pool, m_layout_bindings[0], m_write_sets[0], name);

    m_layout_bindings.clear();
    m_write_sets.clear();
    m_descriptor_buffer_infos.clear();
    m_descriptor_image_infos.clear();
    m_binding = 0;

    return std::move(generated_descriptor);
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
    descriptor_write.dstBinding = 0;
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
