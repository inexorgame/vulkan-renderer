#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

void ResourceDescriptor::create_descriptor_set_layout(
    const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings) const {
    // TODO: Apply make_info!
    VkDescriptorSetLayoutCreateInfo desc_set_layout_ci{};
    desc_set_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    desc_set_layout_ci.pBindings = layout_bindings.data();
    desc_set_layout_ci.bindingCount = static_cast<uint32_t>(layout_bindings.size());

    if (const auto result =
            vkCreateDescriptorSetLayout(m_device.device(), &desc_set_layout_ci, nullptr, &descriptor_set_layout);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateDescriptorSetLayout failed!", result);
    }
}

void ResourceDescriptor::allocate_descriptor_set(std::vector<VkWriteDescriptorSet> &desc_writes) const {
    // TODO: Apply make_info!
    VkDescriptorSetAllocateInfo desc_set_ai{};
    desc_set_ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    desc_set_ai.descriptorPool = m_descriptor_pool;
    desc_set_ai.pSetLayouts = &descriptor_set_layout;
    desc_set_ai.descriptorSetCount = 1;

    if (const auto result = vkAllocateDescriptorSets(m_device.device(), &desc_set_ai, &descriptor_set);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkAllocateDescriptorSets failed!", result);
    }

    for (auto &write : desc_writes) {
        write.dstSet = descriptor_set;
    }

    vkUpdateDescriptorSets(m_device.device(), static_cast<std::uint32_t>(desc_writes.size()), desc_writes.data(), 0, 0);
}

ResourceDescriptor::ResourceDescriptor(const Device &device, const std::vector<VkDescriptorPoolSize> &pool_sizes,
                                       const std::vector<VkDescriptorSetLayoutBinding> &layout_bindings,
                                       std::vector<VkWriteDescriptorSet> &desc_writes, std::string name)
    : m_device(device), DescriptorPool(device, pool_sizes, name) {

    create_descriptor_set_layout(layout_bindings);
    allocate_descriptor_set(desc_writes);
}

ResourceDescriptor::ResourceDescriptor(ResourceDescriptor &&other) noexcept
    : m_device(other.m_device), DescriptorPool(std::move(other)) {
    m_name = std::move(other.m_name);
    descriptor_set_layout = other.descriptor_set_layout;
    descriptor_set = other.descriptor_set;
}

ResourceDescriptor::~ResourceDescriptor() {
    vkDestroyDescriptorSetLayout(m_device.device(), descriptor_set_layout, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
