#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_cache.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(const Device &device) : m_device(device) {}

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(DescriptorSetLayoutBuilder &&other) noexcept
    : m_device(other.m_device) {
    m_bindings = std::move(other.m_bindings);
    m_binding = other.m_binding;
}

DescriptorSetLayoutBuilder &DescriptorSetLayoutBuilder::add_uniform_buffer(const VkShaderStageFlags shader_stage,
                                                                           const std::uint32_t count) {
    m_bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = m_binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = count,
        .stageFlags = shader_stage,
    });
    // NOTE: Even if 'count' is larger than 1, the binding is incremented by only 1
    m_binding++;
    return *this;
}

DescriptorSetLayoutBuilder &
DescriptorSetLayoutBuilder::add_combined_image_sampler(const VkShaderStageFlags shader_stage,
                                                       const std::uint32_t count) {
    m_bindings.emplace_back(VkDescriptorSetLayoutBinding{
        .binding = m_binding,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = count,
        .stageFlags = shader_stage,
    });
    // NOTE: Even if 'count' is larger than 1, the binding is incremented by only 1
    m_binding++;
    return *this;
}

VkDescriptorSetLayout DescriptorSetLayoutBuilder::build(std::string name) {
    const auto descriptor_set_layout_ci = make_info<VkDescriptorSetLayoutCreateInfo>({
        .bindingCount = static_cast<std::uint32_t>(m_bindings.size()),
        .pBindings = m_bindings.data(),
    });

    // Create the descriptor set layout using the descriptor set layout cache
    const auto descriptor_set_layout =
        m_descriptor_set_layout_cache.create_descriptor_set_layout(descriptor_set_layout_ci, std::move(name));

    // Reset all the data of the builder so the builder can be re-used
    m_bindings.clear();
    m_binding = 0;

    // Return the descriptor set layout that was created
    return descriptor_set_layout;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
