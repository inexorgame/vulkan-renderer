#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(const Device &device,
                                                       DescriptorSetLayoutCache &descriptor_set_layout_cache)
    : m_device(device), m_descriptor_set_layout_cache(descriptor_set_layout_cache) {}

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(DescriptorSetLayoutBuilder &&other) noexcept
    : m_device(other.m_device), m_descriptor_set_layout_cache(other.m_descriptor_set_layout_cache) {
    m_bindings = std::move(other.m_bindings);
    m_binding = other.m_binding;
}

DescriptorSetLayoutBuilder &DescriptorSetLayoutBuilder::add_uniform_buffer(const VkShaderStageFlags shader_stage) {
    m_bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = m_binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = shader_stage,
    });

    // Let's automatically increase the binding index because bindings are unique
    m_binding++;
    return *this;
}

DescriptorSetLayoutBuilder &
DescriptorSetLayoutBuilder::add_combined_image_sampler(const VkShaderStageFlags shader_stage) {
    m_bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = m_binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = shader_stage,
    });

    // Let's automatically increase the binding index because bindings are unique
    m_binding++;
    return *this;
}

VkDescriptorSetLayout DescriptorSetLayoutBuilder::build() {
    const auto descriptor_set_layout_ci = make_info<VkDescriptorSetLayoutCreateInfo>({
        .bindingCount = static_cast<std::uint32_t>(m_bindings.size()),
        .pBindings = m_bindings.data(),
    });

    // Create the descriptor set layout using the descriptor set layout cache
    const auto descriptor_set_layout =
        m_descriptor_set_layout_cache.create_descriptor_set_layout(descriptor_set_layout_ci);

    // Clear the builder's data so the builder can be re-used
    m_bindings.clear();
    m_binding = 0;

    // Return the created descriptor set and the descriptor set layout as a pair
    return descriptor_set_layout;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
