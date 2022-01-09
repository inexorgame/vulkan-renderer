#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

ResourceDescriptor::ResourceDescriptor(const Device &device, const VkDescriptorPool pool,
                                       std::vector<VkDescriptorSetLayoutBinding> layout_bindings,
                                       std::vector<VkWriteDescriptorSet> descriptor_writes, const std::string name)
    : m_device(device), m_name(name), m_descriptor_pool(pool) {

    assert(!m_name.empty());
    assert(!layout_bindings.empty());
    assert(!descriptor_writes.empty());
    assert(layout_bindings.size() == descriptor_writes.size());

    auto descriptor_set_layout_ci = make_info<VkDescriptorSetLayoutCreateInfo>();
    descriptor_set_layout_ci.bindingCount = static_cast<std::uint32_t>(layout_bindings.size());
    descriptor_set_layout_ci.pBindings = layout_bindings.data();

    if (const auto result =
            vkCreateDescriptorSetLayout(device.device(), &descriptor_set_layout_ci, nullptr, &m_descriptor_set_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDescriptorSetLayout failed for descriptor " + m_name + " !", result);
    }

    m_device.set_debug_marker_name(m_descriptor_set_layout, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT,
                                   m_name);

    auto descriptor_set_ai = wrapper::make_info<VkDescriptorSetAllocateInfo>();
    descriptor_set_ai.descriptorPool = m_descriptor_pool;
    descriptor_set_ai.descriptorSetCount = 1;
    descriptor_set_ai.pSetLayouts = &m_descriptor_set_layout;

    if (const auto result = vkAllocateDescriptorSets(device.device(), &descriptor_set_ai, &m_descriptor_set);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkAllocateDescriptorSets failed for descriptor " + m_name + " !", result);
    }

    m_device.set_debug_marker_name(m_descriptor_set, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, m_name);

    for (std::size_t i = 0; i < descriptor_writes.size(); i++) {
        descriptor_writes[i].dstSet = m_descriptor_set;
    }

    vkUpdateDescriptorSets(device.device(), static_cast<std::uint32_t>(descriptor_writes.size()),
                           descriptor_writes.data(), 0, nullptr);

    m_descriptor_set_count = static_cast<std::uint32_t>(descriptor_writes.size());
}

ResourceDescriptor::ResourceDescriptor(ResourceDescriptor &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_descriptor_pool = other.m_descriptor_pool;
    m_descriptor_set_layout = other.m_descriptor_set_layout;
    m_descriptor_set = other.m_descriptor_set;
}

ResourceDescriptor::~ResourceDescriptor() {
    vkDestroyDescriptorSetLayout(m_device.device(), m_descriptor_set_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
