#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>
#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper {

Descriptor::Descriptor(Descriptor &&other) noexcept
    : m_name(std::move(other.m_name)), m_number_of_images_in_swapchain(other.m_number_of_images_in_swapchain),
      m_descriptor_sets(std::move(other.m_descriptor_sets)),
      m_write_descriptor_sets(std::move(other.m_write_descriptor_sets)),
      m_descriptor_set_layout_bindings(std::move(other.m_descriptor_set_layout_bindings)),
      m_descriptor_set_layout(std::exchange(other.m_descriptor_set_layout, nullptr)),
      m_descriptor_pool(std::exchange(other.m_descriptor_pool, nullptr)), m_device(other.m_device) {}

Descriptor::Descriptor(const VkDevice device, const std::uint32_t number_of_images_in_swapchain,
                       const std::string &name)
    : m_device(device), m_number_of_images_in_swapchain(number_of_images_in_swapchain), m_name(name) {}

void Descriptor::create_descriptor_pool(const std::initializer_list<VkDescriptorType> descriptor_pool_types) {
    assert(m_device);
    assert(m_number_of_images_in_swapchain > 0);

    std::vector<VkDescriptorPoolSize> pool_sizes;

    for (const auto &descriptor_pool_type : descriptor_pool_types) {
        VkDescriptorPoolSize new_descriptor_pool_entry;

        new_descriptor_pool_entry.type = descriptor_pool_type;
        new_descriptor_pool_entry.descriptorCount = m_number_of_images_in_swapchain;

        // TODO: Simplify this.
        pool_sizes.emplace_back(new_descriptor_pool_entry);
    }

    spdlog::debug("Creating new descriptor pool.");

    auto descriptor_pool_ci = make_info<VkDescriptorPoolCreateInfo>();
    descriptor_pool_ci.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
    descriptor_pool_ci.pPoolSizes = pool_sizes.data();
    descriptor_pool_ci.maxSets = static_cast<std::uint32_t>(m_number_of_images_in_swapchain);

    if (vkCreateDescriptorPool(m_device, &descriptor_pool_ci, nullptr, &m_descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateDescriptorPool failed for descriptor " + m_name + " !");
    }

    // TODO: Assign name using debug markers!

    spdlog::debug("Created descriptor pool for descriptor {} successfully.", m_name);
}

void Descriptor::create_descriptor_set_layouts(
    const std::vector<VkDescriptorSetLayoutBinding> &descriptor_set_layout_bindings) {
    assert(m_device);
    assert(!m_name.empty());
    assert(m_descriptor_pool);
    assert(!descriptor_set_layout_bindings.empty());

    this->m_descriptor_set_layout_bindings = descriptor_set_layout_bindings;

    spdlog::debug("Creating descriptor set layout for descriptor '{}'.", m_name);

    auto descriptor_set_layout_ci = make_info<VkDescriptorSetLayoutCreateInfo>();
    descriptor_set_layout_ci.bindingCount = static_cast<std::uint32_t>(descriptor_set_layout_bindings.size());
    descriptor_set_layout_ci.pBindings = descriptor_set_layout_bindings.data();

    if (vkCreateDescriptorSetLayout(m_device, &descriptor_set_layout_ci, nullptr, &m_descriptor_set_layout) !=
        VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateDescriptorSetLayout failed for descriptor " + m_name + " !");
    }

    // TODO: Assign name using debug makers!

    spdlog::debug("Created descriptor sets for descriptor {} successfully.", m_name);
}

void Descriptor::reset(const bool clear_descriptor_layout_bindings) {
    vkDestroyDescriptorSetLayout(m_device, m_descriptor_set_layout, nullptr);
    m_descriptor_set_layout = VK_NULL_HANDLE;

    vkDestroyDescriptorPool(m_device, m_descriptor_pool, nullptr);
    m_descriptor_pool = VK_NULL_HANDLE;

    // Only clear descriptor layout bindings when application is shut down.
    // Do not clear descriptor layout bindings when swapchain needs to be recreated.
    if (clear_descriptor_layout_bindings) {
        m_descriptor_set_layout_bindings.clear();
    }
}

void Descriptor::add_descriptor_writes(const std::vector<VkWriteDescriptorSet> &descriptor_writes) {
    assert(m_device);
    assert(m_descriptor_pool);
    assert(!m_name.empty());
    assert(!descriptor_writes.empty());

    // We need a descriptor write for every descriptor set layout binding!
    assert(m_descriptor_set_layout_bindings.size() == descriptor_writes.size());

    this->m_write_descriptor_sets = descriptor_writes;
}

void Descriptor::create_descriptor_sets() {
    assert(m_device);
    assert(m_descriptor_pool);
    assert(!m_name.empty());
    assert(!m_descriptor_set_layout_bindings.empty());
    assert(!m_write_descriptor_sets.empty());

    // We need a descriptor write for every descriptor set layout binding!
    assert(m_descriptor_set_layout_bindings.size() == m_write_descriptor_sets.size());

    spdlog::debug("Creating descriptor sets for '{}'.", m_name);

    const std::vector<VkDescriptorSetLayout> descriptor_set_layouts(m_number_of_images_in_swapchain,
                                                                    m_descriptor_set_layout);

    auto descriptor_set_ai = make_info<VkDescriptorSetAllocateInfo>();
    descriptor_set_ai.descriptorPool = m_descriptor_pool;
    descriptor_set_ai.descriptorSetCount = static_cast<std::uint32_t>(m_number_of_images_in_swapchain);
    descriptor_set_ai.pSetLayouts = descriptor_set_layouts.data();

    // TODO: Do we need this for recreation of swapchain?
    m_descriptor_sets.clear();

    m_descriptor_sets.resize(m_number_of_images_in_swapchain);

    if (vkAllocateDescriptorSets(m_device, &descriptor_set_ai, m_descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkAllocateDescriptorSets failed for descriptor " + m_name + " !");
    }

    // TODO: Assign name using debug makers!

    for (std::size_t k = 0; k < m_number_of_images_in_swapchain; k++) {
        spdlog::debug("Updating descriptor set '{}' #{}", m_name, k);

        for (std::size_t j = 0; j < m_write_descriptor_sets.size(); j++) {
            m_write_descriptor_sets[j].dstBinding = static_cast<std::uint32_t>(j);
            m_write_descriptor_sets[j].dstSet = m_descriptor_sets[k];
        }

        vkUpdateDescriptorSets(m_device, static_cast<std::uint32_t>(m_write_descriptor_sets.size()),
                               m_write_descriptor_sets.data(), 0, nullptr);
    }

    spdlog::debug("Created descriptor sets for descriptor {} successfully.", m_name);
}

Descriptor::~Descriptor() {
    assert(m_device);
    spdlog::trace("Destroying descriptor {}.", m_name);

    reset(true);
}

} // namespace inexor::vulkan_renderer::wrapper
