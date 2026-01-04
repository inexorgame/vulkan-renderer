#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorSetLayout::DescriptorSetLayout(const Device &device,
                                         const VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci,
                                         std::string name)
    : m_device(device), m_name(std::move(name)) {
    if (m_name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
    if (const auto result = vkCreateDescriptorSetLayout(m_device.device(), &descriptor_set_layout_ci, nullptr,
                                                        &m_descriptor_set_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDescriptorSetLayout failed!", result, m_name);
    }
    m_device.set_debug_name(m_descriptor_set_layout, m_name);
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, VK_NULL_HANDLE);
}

DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(m_device.device(), m_descriptor_set_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
