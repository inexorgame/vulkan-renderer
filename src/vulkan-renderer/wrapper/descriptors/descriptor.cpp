#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::descriptors {

using tools::VulkanException;

ResourceDescriptor::ResourceDescriptor(ResourceDescriptor &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_descriptor_pool = std::exchange(other.m_descriptor_pool, nullptr);
    m_descriptor_set_layout = std::exchange(other.m_descriptor_set_layout, nullptr);
    m_descriptor_set_layout_bindings = std::move(other.m_descriptor_set_layout_bindings);
    m_write_descriptor_sets = std::move(other.m_write_descriptor_sets);
    m_descriptor_sets = std::move(other.m_descriptor_sets);
}

ResourceDescriptor::ResourceDescriptor(const Device &device,
                                       std::vector<VkDescriptorSetLayoutBinding> &&layout_bindings,
                                       std::vector<VkWriteDescriptorSet> &&descriptor_writes, std::string &&name)
    : m_device(device), m_name(name), m_write_descriptor_sets(descriptor_writes),
      m_descriptor_set_layout_bindings(layout_bindings) {
    assert(device.device());
    assert(!layout_bindings.empty());
    assert(!m_write_descriptor_sets.empty());
    assert(layout_bindings.size() == m_write_descriptor_sets.size());

    for (std::size_t i = 0; i < layout_bindings.size(); i++) {
        if (layout_bindings[i].descriptorType != descriptor_writes[i].descriptorType) {
            throw std::runtime_error(
                "Error: VkDescriptorType mismatch in descriptor set layout binding and write descriptor set!");
        }
    }

    std::vector<VkDescriptorPoolSize> pool_sizes;

    pool_sizes.reserve(layout_bindings.size());

    for (const auto &descriptor_pool_type : layout_bindings) {
        pool_sizes.emplace_back(VkDescriptorPoolSize{descriptor_pool_type.descriptorType, 1});
    }

    m_device.create_descriptor_pool(make_info<VkDescriptorPoolCreateInfo>({
                                        .maxSets = 1,
                                        .poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size()),
                                        .pPoolSizes = pool_sizes.data(),
                                    }),
                                    &m_descriptor_pool, m_name);

    m_device.create_descriptor_set_layout(
        make_info<VkDescriptorSetLayoutCreateInfo>({
            .bindingCount = static_cast<std::uint32_t>(m_descriptor_set_layout_bindings.size()),
            .pBindings = m_descriptor_set_layout_bindings.data(),
        }),
        &m_descriptor_set_layout, m_name);

    const std::vector<VkDescriptorSetLayout> descriptor_set_layouts(1, m_descriptor_set_layout);

    const auto descriptor_set_ai = make_info<VkDescriptorSetAllocateInfo>({
        .descriptorPool = m_descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = descriptor_set_layouts.data(),
    });

    m_descriptor_sets.resize(1);

    if (const auto result = vkAllocateDescriptorSets(device.device(), &descriptor_set_ai, m_descriptor_sets.data());
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkAllocateDescriptorSets failed for descriptor " + m_name + " !", result);
    }

    for (const auto &descriptor_set : m_descriptor_sets) {
        // Assign an internal name using Vulkan debug markers.
        m_device.set_debug_marker_name(descriptor_set, VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, m_name);
    }

    for (std::size_t j = 0; j < m_write_descriptor_sets.size(); j++) {
        m_write_descriptor_sets[j].dstBinding = static_cast<std::uint32_t>(j);
        m_write_descriptor_sets[j].dstSet = m_descriptor_sets[0];
    }

    vkUpdateDescriptorSets(device.device(), static_cast<std::uint32_t>(m_write_descriptor_sets.size()),
                           m_write_descriptor_sets.data(), 0, nullptr);
}

ResourceDescriptor::~ResourceDescriptor() {
    vkDestroyDescriptorSetLayout(m_device.device(), m_descriptor_set_layout, nullptr);
    vkDestroyDescriptorPool(m_device.device(), m_descriptor_pool, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
