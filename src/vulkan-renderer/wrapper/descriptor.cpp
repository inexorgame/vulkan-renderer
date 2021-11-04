#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

ResourceDescriptor::ResourceDescriptor(const Device &device, const VkDescriptorPool descriptor_pool,
                                       const VkDescriptorSetLayoutBinding layout_binding,
                                       const VkWriteDescriptorSet descriptor_write, const std::string name)
    : m_device(device), m_name(name), m_write_descriptor_set(descriptor_write),
      m_descriptor_set_layout_binding(layout_binding) {
    assert(device.device());

    auto descriptor_set_layout_ci = make_info<VkDescriptorSetLayoutCreateInfo>();
    descriptor_set_layout_ci.bindingCount = 1;
    descriptor_set_layout_ci.pBindings = &m_descriptor_set_layout_binding;

    if (const auto result =
            vkCreateDescriptorSetLayout(device.device(), &descriptor_set_layout_ci, nullptr, &m_descriptor_set_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDescriptorSetLayout failed for descriptor " + m_name + " !", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_descriptor_set_layout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
                                   m_name);

    auto descriptor_set_ai = wrapper::make_info<VkDescriptorSetAllocateInfo>();
    descriptor_set_ai.descriptorPool = descriptor_pool;
    descriptor_set_ai.descriptorSetCount = 1;
    descriptor_set_ai.pSetLayouts = &m_descriptor_set_layout;

    if (const auto result = vkAllocateDescriptorSets(device.device(), &descriptor_set_ai, &m_descriptor_set);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkAllocateDescriptorSets failed for descriptor " + m_name + " !", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_descriptor_set, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, m_name);

    // Write in the allocated descriptor set.
    m_write_descriptor_set.dstSet = m_descriptor_set;
    vkUpdateDescriptorSets(device.device(), 1, &m_write_descriptor_set, 0, nullptr);
}

ResourceDescriptor::~ResourceDescriptor() {
    vkDestroyDescriptorSetLayout(m_device.device(), m_descriptor_set_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
