#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

DescriptorBuilder::DescriptorBuilder(DescriptorBuilder &&other) noexcept : m_device(other.m_device) {
    m_descriptor_pool = std::exchange(other.m_descriptor_pool, VK_NULL_HANDLE);
    m_binding = other.m_binding;
    m_layout_bindings = std::move(other.m_layout_bindings);
    m_write_sets = std::move(other.m_write_sets);
    m_descriptor_buffer_infos = std::move(other.m_descriptor_buffer_infos);
    m_descriptor_image_infos = std::move(other.m_descriptor_image_infos);
}

DescriptorBuilder::DescriptorBuilder(const Device &device) : m_device(device) {}

DescriptorBuilder &DescriptorBuilder::add_combined_image_sampler(const VkSampler image_sampler,
                                                                 const VkImageView image_view,
                                                                 const VkImageLayout image_layout,
                                                                 const VkShaderStageFlagBits shader_stage) {
    assert(image_sampler);
    assert(image_view);

    const VkDescriptorPoolSize pool_size = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
    m_pool_sizes.push_back(pool_size);

    auto layout_binding = VkDescriptorSetLayoutBinding{};
    layout_binding.binding = m_binding;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = shader_stage;
    layout_binding.pImmutableSamplers = nullptr;

    m_layout_bindings.push_back(std::move(layout_binding));

    auto image_info = std::make_unique<VkDescriptorImageInfo>();
    image_info->sampler = image_sampler;
    image_info->imageView = image_view;
    image_info->imageLayout = image_layout;

    m_descriptor_image_infos.push_back(std::move(image_info));

    auto descriptor_write = make_info<VkWriteDescriptorSet>();
    descriptor_write.dstBinding = m_binding;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pImageInfo = m_descriptor_image_infos.back().get();

    m_write_sets.push_back(std::move(descriptor_write));

    m_binding++;

    return *this;
}

DescriptorBuilder &DescriptorBuilder::add_combined_image_sampler(const texture::GpuTexture &texture) {
    return add_combined_image_sampler(texture.sampler(), texture.image_view(),
                                      texture.descriptor_image_info.imageLayout);
}

DescriptorBuilder &DescriptorBuilder::add_combined_image_sampler(const cubemap::GpuCubemap &cubemap) {
    return add_combined_image_sampler(cubemap.sampler(), cubemap.image_view(),
                                      cubemap.descriptor_image_info.imageLayout);
}

DescriptorBuilder &DescriptorBuilder::add_combined_image_samplers(const std::vector<texture::GpuTexture> &textures) {
    for (const auto &texture : textures) {
        const auto &result = add_combined_image_sampler(texture.sampler(), texture.image_view(),
                                                        texture.descriptor_image_info.imageLayout);
    }
    return *this;
}

std::unique_ptr<ResourceDescriptor> DescriptorBuilder::build(const std::string name, const std::uint32_t max_sets) {
    assert(!m_pool_sizes.empty());
    assert(!m_layout_bindings.empty());
    assert(!m_write_sets.empty());
    assert(!name.empty());

    auto generated_descriptor =
        std::make_unique<ResourceDescriptor>(m_device, m_pool_sizes, max_sets, m_layout_bindings, m_write_sets, name);

    // Reset the builder's members for the next use
    m_layout_bindings.clear();
    m_write_sets.clear();
    m_descriptor_buffer_infos.clear();
    m_descriptor_image_infos.clear();
    m_binding = 0;

    return std::move(generated_descriptor);
}

std::unique_ptr<ResourceDescriptor> DescriptorBuilder::build(const std::string name) {
    return std::move(build(name, static_cast<std::uint32_t>(m_pool_sizes.size())));
}

} // namespace inexor::vulkan_renderer::wrapper
