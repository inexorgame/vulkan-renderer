#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

DescriptorBuilder::DescriptorBuilder(const Device &device, std::uint32_t swapchain_image_count)
    : m_device(device), m_swapchain_image_count(swapchain_image_count) {
    assert(m_device.device());
    assert(m_swapchain_image_count > 0);
}

ResourceDescriptor DescriptorBuilder::build(std::string name) {
    assert(!m_layout_bindings.empty());
    assert(!m_write_sets.empty());
    assert(m_write_sets.size() == m_layout_bindings.size());

    // Generate a new resource descriptor.
    ResourceDescriptor generated_descriptor(m_device, m_swapchain_image_count, std::move(m_layout_bindings),
                                            std::move(m_write_sets), std::move(name));

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

    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = 0;
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

    return *this;
}

} // namespace inexor::vulkan_renderer::wrapper
