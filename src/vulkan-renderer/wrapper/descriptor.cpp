#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

ResourceDescriptor::ResourceDescriptor(const Device &device, std::uint32_t swapchain_image_count,
                                       VkDescriptorSetLayoutBinding layout_binding,
                                       VkWriteDescriptorSet descriptor_write, std::string name)
    : m_device(device), m_name(name), m_write_descriptor_set(descriptor_write),
      m_descriptor_set_layout_binding(layout_binding), m_swapchain_image_count(swapchain_image_count) {
    assert(device.device());
    assert(swapchain_image_count > 0);

    VkDescriptorPoolSize pool_sizes({layout_binding.descriptorType, swapchain_image_count});

    spdlog::debug("Creating new descriptor pool.");

    auto descriptor_pool_ci = wrapper::make_info<VkDescriptorPoolCreateInfo>();
    descriptor_pool_ci.poolSizeCount = 1;
    descriptor_pool_ci.pPoolSizes = &pool_sizes;
    descriptor_pool_ci.maxSets = static_cast<std::uint32_t>(swapchain_image_count);

    if (const auto result = vkCreateDescriptorPool(device.device(), &descriptor_pool_ci, nullptr, &m_descriptor_pool);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDescriptorPool failed for descriptor " + m_name + " !", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_descriptor_pool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, m_name);

    spdlog::debug("Created descriptor pool for descriptor {} successfully.", m_name);

    spdlog::debug("Creating descriptor set layout for descriptor '{}'.", m_name);

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

    spdlog::debug("Created descriptor sets for descriptor {} successfully.", m_name);

    spdlog::debug("Creating descriptor sets for '{}'.", m_name);

    const std::vector<VkDescriptorSetLayout> descriptor_set_layouts(swapchain_image_count, m_descriptor_set_layout);

    auto descriptor_set_ai = wrapper::make_info<VkDescriptorSetAllocateInfo>();
    descriptor_set_ai.descriptorPool = m_descriptor_pool;
    descriptor_set_ai.descriptorSetCount = static_cast<std::uint32_t>(swapchain_image_count);
    descriptor_set_ai.pSetLayouts = descriptor_set_layouts.data();

    if (const auto result = vkAllocateDescriptorSets(device.device(), &descriptor_set_ai, &m_descriptor_set);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkAllocateDescriptorSets failed for descriptor " + m_name + " !", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_descriptor_set, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, m_name);

    for (std::size_t k = 0; k < swapchain_image_count; k++) {
        m_write_descriptor_set.dstBinding = 0;
        m_write_descriptor_set.dstSet = m_descriptor_set;

        spdlog::debug("Updating descriptor set '{}' #{}", m_name, k);
        vkUpdateDescriptorSets(device.device(), 1, &m_write_descriptor_set, 0, nullptr);
    }

    spdlog::debug("Created descriptor sets for descriptor {} successfully.", m_name);
}

ResourceDescriptor::~ResourceDescriptor() {
    vkDestroyDescriptorSetLayout(m_device.device(), m_descriptor_set_layout, nullptr);
    vkDestroyDescriptorPool(m_device.device(), m_descriptor_pool, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
