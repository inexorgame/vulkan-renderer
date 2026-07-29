#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"

#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorSetLayoutBuilder::DescriptorSetLayoutBuilder(const core::Device &device)
    : m_device(device), m_descriptor_set_layout_cache(device) {}

VkDescriptorSetLayout DescriptorSetLayoutBuilder::build(std::string name) {
    const auto descriptor_set_layout_ci = tools::make_info<VkDescriptorSetLayoutCreateInfo>({
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
