#include "inexor/vulkan-renderer/wrapper/resource_descriptor.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

ResourceDescriptor::ResourceDescriptor(ResourceDescriptor &&other) noexcept
    : m_device(other.m_device), m_name(std::move(other.m_name)),
      m_descriptor_pool(std::exchange(other.m_descriptor_pool, nullptr)),
      m_descriptor_set_layout(std::exchange(other.m_descriptor_set_layout, nullptr)),
      m_descriptor_set_layout_bindings(std::move(other.m_descriptor_set_layout_bindings)),
      m_write_descriptor_sets(std::move(other.m_write_descriptor_sets)),
      m_descriptor_sets(std::move(other.m_descriptor_sets)), m_swapchain_image_count(other.m_swapchain_image_count) {}

ResourceDescriptor::ResourceDescriptor(const Device &device, std::uint32_t swapchain_image_count,
                                       std::initializer_list<VkDescriptorType> pool_types,
                                       const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings,
                                       const std::vector<VkWriteDescriptorSet> &descriptor_writes,
                                       const std::string &name)
    : m_device(device), m_name(name), m_write_descriptor_sets(descriptor_writes),
      m_descriptor_set_layout_bindings(layout_bindings), m_swapchain_image_count(swapchain_image_count) {
    assert(device.device());
    assert(!layout_bindings.empty());
    assert(swapchain_image_count > 0);
    assert(!m_write_descriptor_sets.empty());
    assert(layout_bindings.size() == m_write_descriptor_sets.size());

    // TODO() Check if descriptor writes match descriptor set layout!

    std::vector<VkDescriptorPoolSize> pool_sizes;

    for (const auto &descriptor_pool_type : pool_types) {
        pool_sizes.emplace_back(VkDescriptorPoolSize{descriptor_pool_type, swapchain_image_count});
    }

    spdlog::debug("Creating new descriptor pool.");

    auto descriptor_pool_ci = wrapper::make_info<VkDescriptorPoolCreateInfo>();
    descriptor_pool_ci.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
    descriptor_pool_ci.pPoolSizes = pool_sizes.data();
    descriptor_pool_ci.maxSets = static_cast<std::uint32_t>(swapchain_image_count);

    if (vkCreateDescriptorPool(device.device(), &descriptor_pool_ci, nullptr, &m_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateDescriptorPool failed for descriptor " + m_name + " !");
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_descriptor_pool, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, m_name);

    spdlog::debug("Created descriptor pool for descriptor {} successfully.", m_name);

    spdlog::debug("Creating descriptor set layout for descriptor '{}'.", m_name);

    auto descriptor_set_layout_ci = make_info<VkDescriptorSetLayoutCreateInfo>();
    descriptor_set_layout_ci.bindingCount = static_cast<std::uint32_t>(m_descriptor_set_layout_bindings.size());
    descriptor_set_layout_ci.pBindings = m_descriptor_set_layout_bindings.data();

    if (vkCreateDescriptorSetLayout(device.device(), &descriptor_set_layout_ci, nullptr, &m_descriptor_set_layout) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateDescriptorSetLayout failed for descriptor " + m_name + " !");
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

    m_descriptor_sets.resize(swapchain_image_count);

    if (vkAllocateDescriptorSets(device.device(), &descriptor_set_ai, m_descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkAllocateDescriptorSets failed for descriptor " + m_name + " !");
    }

    for (const auto &descriptor_set : m_descriptor_sets) {
        // Assign an internal name using Vulkan debug markers.
        m_device.set_debug_marker_name(descriptor_set, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, m_name);
    }

    for (std::size_t k = 0; k < swapchain_image_count; k++) {
        for (std::size_t j = 0; j < m_write_descriptor_sets.size(); j++) {
            m_write_descriptor_sets[j].dstBinding = static_cast<std::uint32_t>(j);
            m_write_descriptor_sets[j].dstSet = m_descriptor_sets[k];
        }

        spdlog::debug("Updating descriptor set '{}' #{}", m_name, k);

        vkUpdateDescriptorSets(device.device(), static_cast<std::uint32_t>(m_write_descriptor_sets.size()),
                               m_write_descriptor_sets.data(), 0, nullptr);
    }

    spdlog::debug("Created descriptor sets for descriptor {} successfully.", m_name);
}

ResourceDescriptor::~ResourceDescriptor() {
    if (m_descriptor_set_layout != nullptr) {
        spdlog::trace("Destroying resource descriptor set layout {}.", m_name);
        vkDestroyDescriptorSetLayout(m_device.device(), m_descriptor_set_layout, nullptr);
    }

    if (m_descriptor_pool != nullptr) {
        spdlog::trace("Destroying resource descriptor pool {}.", m_name);
        vkDestroyDescriptorPool(m_device.device(), m_descriptor_pool, nullptr);
    }
}

} // namespace inexor::vulkan_renderer::wrapper
