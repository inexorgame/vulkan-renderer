#include "inexor/vulkan-renderer/descriptor_manager.hpp"

namespace inexor::vulkan_renderer {

VkResult DescriptorManager::init(const VkDevice &device, const std::size_t number_of_images_in_swapchain,
                                 const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager) {
    assert(!descriptor_manager_initialised);
    assert(device);
    assert(debug_marker_manager);
    assert(number_of_images_in_swapchain > 0);

    this->device = device;
    this->debug_marker_manager = debug_marker_manager;
    this->number_of_images_in_swapchain = number_of_images_in_swapchain;

    descriptor_manager_initialised = true;

    return VK_SUCCESS;
}

VkResult DescriptorManager::create_descriptor_pool(const std::string &internal_descriptor_pool_name, const std::vector<VkDescriptorPoolSize> &pool_sizes,
                                                   std::shared_ptr<DescriptorPool> &descriptor_pool) {
    assert(descriptor_manager_initialised);
    assert(number_of_images_in_swapchain > 0);
    assert(!internal_descriptor_pool_name.empty());
    assert(!pool_sizes.empty());
    assert(device);

    // Make sure to call the correct template base class method here!
    // Otherwise, we would look up the descriptor pool name in descriptor set manager!
    if (ManagerClassTemplate<DescriptorPool>::does_key_exist(internal_descriptor_pool_name)) {
        spdlog::error("A descriptor pool with internal name '{}' already exists!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    descriptor_pool = std::make_shared<DescriptorPool>(internal_descriptor_pool_name, pool_sizes);

    spdlog::debug("Creating new descriptor pool.");

    VkDescriptorPoolCreateInfo pool_create_info = {};

    pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.poolSizeCount = static_cast<std::uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes = pool_sizes.data();
    pool_create_info.maxSets = static_cast<std::uint32_t>(number_of_images_in_swapchain);

    VkResult result = vkCreateDescriptorPool(device, &pool_create_info, nullptr, &descriptor_pool->pool);
    vulkan_error_check(result);

    std::string debug_marker_name = "Descriptor pool '" + internal_descriptor_pool_name + "'.";

    // Assign an appropriate debug marker name to this descriptor pool.
    debug_marker_manager->set_object_name(device, (std::uint64_t)(descriptor_pool->pool), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT,
                                          debug_marker_name.c_str());

    // Keep this descriptor pool stored in the manager base class.
    ManagerClassTemplate<DescriptorPool>::add_entry(internal_descriptor_pool_name, descriptor_pool);

    return VK_SUCCESS;
}

VkResult DescriptorManager::create_descriptor_bundle(const std::string &internal_descriptor_name, std::shared_ptr<DescriptorPool> &descriptor_pool,
                                                     std::shared_ptr<DescriptorBundle> &descriptor_bundle) {
    assert(descriptor_manager_initialised);
    assert(number_of_images_in_swapchain > 0);
    assert(!internal_descriptor_name.empty());
    assert(descriptor_pool);
    assert(!descriptor_bundle);

    if (ManagerClassTemplate<DescriptorBundle>::does_key_exist(internal_descriptor_name)) {
        spdlog::error("A descriptor set with internal name '{}' already exists!");
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    spdlog::debug("Starting to build a new descriptor called '{}'.", internal_descriptor_name);

    // Allocate a shared pointer for the new descriptor.
    // The internal name and descriptor pool of the new descriptor can't be changed anymore after this.
    descriptor_bundle = std::make_shared<DescriptorBundle>(internal_descriptor_name, descriptor_pool);

    // The new descriptor set will only be added to the base class storage after building it.

    return VK_SUCCESS;
}

VkResult DescriptorManager::add_descriptor_set_layout_binding(std::shared_ptr<DescriptorBundle> descriptor_bundle,
                                                              const VkDescriptorSetLayoutBinding &descriptor_set_layout_binding) {
    assert(descriptor_manager_initialised);
    assert(descriptor_bundle);

    spdlog::debug("Adding descriptor set layout to '{}'.", descriptor_bundle->name);

    // Add the new descriptor set layout binding to the descriptor bundle.
    descriptor_bundle->descriptor_set_layout_bindings.push_back(descriptor_set_layout_binding);

    return VK_SUCCESS;
}

VkResult DescriptorManager::add_write_descriptor_set(std::shared_ptr<DescriptorBundle> descriptor_bundle, const VkWriteDescriptorSet &write_descriptor_set) {
    assert(descriptor_manager_initialised);
    assert(descriptor_bundle);

    spdlog::debug("Adding write descriptor set to '{}'.", descriptor_bundle->name);

    // Add the new write descriptor set to the descriptor bundle.
    descriptor_bundle->write_descriptor_sets.push_back(write_descriptor_set);

    return VK_SUCCESS;
}

VkResult DescriptorManager::create_descriptor_set_layouts(std::shared_ptr<DescriptorBundle> descriptor_bundle) {
    assert(descriptor_manager_initialised);
    assert(descriptor_bundle);
    assert(!descriptor_bundle->descriptor_set_layout_bindings.empty());
    assert(device);

    spdlog::debug("Creating descriptor set layout for '{}'.", descriptor_bundle->name);

    VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info = {};

    descriptor_set_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_set_layout_create_info.bindingCount = static_cast<std::uint32_t>(descriptor_bundle->descriptor_set_layout_bindings.size());
    descriptor_set_layout_create_info.pBindings = descriptor_bundle->descriptor_set_layout_bindings.data();

    VkResult result = vkCreateDescriptorSetLayout(device, &descriptor_set_layout_create_info, nullptr, &descriptor_bundle->descriptor_set_layout);
    vulkan_error_check(result);

    std::string debug_marker_name = "Descriptor set layout for descriptor bundle '" + descriptor_bundle->name + "'.";

    // Assign an appropriate debug marker name to this descriptor pool.
    debug_marker_manager->set_object_name(device, (std::uint64_t)(descriptor_bundle->descriptor_set_layout),
                                          VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, debug_marker_name.c_str());

    return VK_SUCCESS;
}

VkResult DescriptorManager::create_descriptor_sets(std::shared_ptr<DescriptorBundle> descriptor_bundle) {
    assert(descriptor_manager_initialised);
    assert(descriptor_bundle);
    assert(!descriptor_bundle->write_descriptor_sets.empty());
    assert(device);

    spdlog::debug("Creating descriptor sets for '{}'.", descriptor_bundle->name);

    const std::vector<VkDescriptorSetLayout> descriptor_set_layouts(number_of_images_in_swapchain, descriptor_bundle->descriptor_set_layout);

    VkDescriptorSetAllocateInfo descriptor_set_alloc_info = {};

    descriptor_set_alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    descriptor_set_alloc_info.descriptorPool = descriptor_bundle->associated_descriptor_pool->pool;
    descriptor_set_alloc_info.descriptorSetCount = static_cast<std::uint32_t>(number_of_images_in_swapchain);
    descriptor_set_alloc_info.pSetLayouts = descriptor_set_layouts.data();

    // TODO: Do we need this for recreation of swapchain?
    descriptor_bundle->descriptor_sets.clear();

    descriptor_bundle->descriptor_sets.resize(number_of_images_in_swapchain);

    VkResult result = vkAllocateDescriptorSets(device, &descriptor_set_alloc_info, descriptor_bundle->descriptor_sets.data());
    vulkan_error_check(result);

    std::string debug_marker_name = "Descriptor sets for bundle '{" + descriptor_bundle->name + "}'.";

    // Iterate through all descriptor sets.
    for (auto &descriptor_set : descriptor_bundle->descriptor_sets) {
        // Assign an appropriate debug marker name to these descriptor sets.
        debug_marker_manager->set_object_name(device, (std::uint64_t)(descriptor_set), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT,
                                              debug_marker_name.c_str());
    }

    for (std::size_t i = 0; i < number_of_images_in_swapchain; i++) {
        spdlog::debug("Updating descriptor set '{}' #{}", descriptor_bundle->name, i);

        for (std::size_t j = 0; j < descriptor_bundle->write_descriptor_sets.size(); j++) {
            descriptor_bundle->write_descriptor_sets[j].dstBinding = static_cast<std::uint32_t>(j);
            descriptor_bundle->write_descriptor_sets[j].dstSet = descriptor_bundle->descriptor_sets[i];
        }

        vkUpdateDescriptorSets(device, static_cast<std::uint32_t>(descriptor_bundle->write_descriptor_sets.size()),
                               descriptor_bundle->write_descriptor_sets.data(), 0, nullptr);
    }

    spdlog::debug("Storing descriptor bundle '{}'.", descriptor_bundle->name);

    // Store the descriptor bundle!
    ManagerClassTemplate<DescriptorBundle>::add_entry(descriptor_bundle->name, descriptor_bundle);

    return VK_SUCCESS;
}

std::optional<std::shared_ptr<DescriptorBundle>> DescriptorManager::get_descriptor_bundle(const std::string &internal_descriptor_name) {
    assert(descriptor_manager_initialised);

    if (ManagerClassTemplate<DescriptorBundle>::does_key_exist(internal_descriptor_name)) {
        auto descriptor_bundle = ManagerClassTemplate<DescriptorBundle>::get_entry(internal_descriptor_name);

        return descriptor_bundle;
    }

    return std::nullopt;
}

VkResult DescriptorManager::shutdown_descriptors(bool clear_descriptor_layout_bindings) {
    assert(descriptor_manager_initialised);
    assert(device);

    spdlog::debug("Destroying descriptors sets and descriptor pools.");

    auto descriptor_bundles = ManagerClassTemplate<DescriptorBundle>::get_all_values();

    for (auto &descriptor_bundle : descriptor_bundles) {
        vkDestroyDescriptorSetLayout(device, descriptor_bundle->descriptor_set_layout, nullptr);
        descriptor_bundle->descriptor_set_layout = VK_NULL_HANDLE;

        if (descriptor_bundle->associated_descriptor_pool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, descriptor_bundle->associated_descriptor_pool->pool, nullptr);
            descriptor_bundle->associated_descriptor_pool->pool = VK_NULL_HANDLE;
        }

        if (clear_descriptor_layout_bindings) {
            spdlog::debug("Destroying descriptor set layout bindings.");
            descriptor_bundle->descriptor_set_layout_bindings.clear();
        }
    }

    ManagerClassTemplate<DescriptorBundle>::delete_all_entries();
    ManagerClassTemplate<DescriptorPool>::delete_all_entries();

    return VK_SUCCESS;
}

} // namespace inexor::vulkan_renderer
