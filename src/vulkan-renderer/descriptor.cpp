#include "inexor/vulkan-renderer/descriptor.hpp"

namespace inexor::vulkan_renderer {

Descriptor::Descriptor(Descriptor &&other) noexcept : device(other.device) {}

Descriptor::Descriptor(const VkDevice device, const std::uint32_t number_of_images_in_swapchain, const std::string name)
    : device(device), number_of_images_in_swapchain(number_of_images_in_swapchain), name(name) {}

void Descriptor::create_descriptor_pool(const std::initializer_list<VkDescriptorType> descriptor_pool_types) {
    assert(device);
    assert(number_of_images_in_swapchain > 0);

    std::vector<VkDescriptorPoolSize> pool_sizes(descriptor_pool_types.size());

    for (const auto &descriptor_pool_type : descriptor_pool_types) {
        VkDescriptorPoolSize new_descriptor_pool_entry;

        new_descriptor_pool_entry.type = descriptor_pool_type;
        new_descriptor_pool_entry.descriptorCount = number_of_images_in_swapchain;

        // TODO: Simplify this.
        pool_sizes.emplace_back(new_descriptor_pool_entry);
    }

    spdlog::debug("Creating new descriptor pool.");

    VkDescriptorPoolCreateInfo pool_create_info = {};

    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();
    pool_create_info.maxSets = static_cast<std::uint32_t>(number_of_images_in_swapchain);

    if (vkCreateDescriptorPool(device, &pool_create_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateDescriptorPool failed for descriptor " + name + " !");
    }

    // TODO: Assign name using debug markers!

    spdlog::debug("Created descriptor pool for descriptor {} successfully.", name);
}

void Descriptor::create_descriptor_set_layouts(const std::vector<VkDescriptorSetLayoutBinding> &descriptor_set_layout_bindings) {
    assert(device);
    assert(!name.empty());
    assert(descriptor_pool);
    assert(!descriptor_set_layout_bindings.empty());

    this->descriptor_set_layout_bindings = descriptor_set_layout_bindings;

    spdlog::debug("Creating descriptor set layout for descriptor '{}'.", name);

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};

    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = static_cast<std::uint32_t>(descriptor_set_layout_bindings.size());
    descriptor_set_layout_create_info.pBindings = descriptor_set_layout_bindings.data();

    if (vkCreateDescriptorSetLayout(device, &descriptor_set_layout_create_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkCreateDescriptorSetLayout failed for descriptor " + name + " !");
    }

    // TODO: Assign name using debug makers!

    spdlog::debug("Created descriptor sets for descriptor {} successfully.", name);
}

void Descriptor::reset(const bool clear_descriptor_layout_bindings) {
    vkDestroyDescriptorSetLayout(device, descriptor_set_layout, nullptr);
    descriptor_set_layout = VK_NULL_HANDLE;

    vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
    descriptor_pool = VK_NULL_HANDLE;

    // Only clear descriptor layout bindings when application is shut down.
    // Do not clear descriptor layout bindings when swapchain needs to be recreated.
    if (clear_descriptor_layout_bindings) {
        descriptor_set_layout_bindings.clear();
    }
}

void Descriptor::add_descriptor_writes(const std::vector<VkWriteDescriptorSet> &descriptor_writes) {
    assert(device);
    assert(descriptor_pool);
    assert(!name.empty());
    assert(!descriptor_writes.empty());

    // We need a descriptor write for every descriptor set layout binding!
    assert(descriptor_set_layout_bindings.size() == descriptor_writes.size());

    this->write_descriptor_sets = descriptor_writes;
}

void Descriptor::create_descriptor_sets() {
    assert(device);
    assert(descriptor_pool);
    assert(!name.empty());
    assert(!descriptor_set_layout_bindings.empty());
    assert(!write_descriptor_sets.empty());

    // We need a descriptor write for every descriptor set layout binding!
    assert(descriptor_set_layout_bindings.size() == write_descriptor_sets.size());

    spdlog::debug("Creating descriptor sets for '{}'.", name);

    const std::vector<VkDescriptorSetLayout> descriptor_set_layouts(number_of_images_in_swapchain, descriptor_set_layout);

    VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};

    descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_alloc_info.descriptorPool = descriptor_pool;
    descriptor_set_alloc_info.descriptorSetCount = static_cast<std::uint32_t>(number_of_images_in_swapchain);
    descriptor_set_alloc_info.pSetLayouts = descriptor_set_layouts.data();

    // TODO: Do we need this for recreation of swapchain?
    descriptor_sets.clear();

    descriptor_sets.resize(number_of_images_in_swapchain);

    if (vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Error: vkAllocateDescriptorSets failed for descriptor " + name + " !");
    }

    // TODO: Assign name using debug makers!

    for (std::size_t i = 0; i < number_of_images_in_swapchain; i++) {
        spdlog::debug("Updating descriptor set '{}' #{}", name, i);

        for (std::size_t j = 0; j < write_descriptor_sets.size(); j++) {
            write_descriptor_sets[j].dstBinding = static_cast<std::uint32_t>(j);
            write_descriptor_sets[j].dstSet = descriptor_sets[i];
        }

        vkUpdateDescriptorSets(device, static_cast<std::uint32_t>(write_descriptor_sets.size()), write_descriptor_sets.data(), 0, nullptr);
    }

    spdlog::debug("Created descriptor sets for descriptor {} successfully.", name);
}

Descriptor::~Descriptor() {
    assert(device);

    reset(true);
}

} // namespace inexor::vulkan_renderer
