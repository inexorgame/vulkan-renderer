#pragma once

#include "inexor/vulkan-renderer/descriptor_pool.hpp"

#include <vulkan/vulkan.h>

#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer {

///
///
///
struct InexorDescriptorBundle {
public:
    /// Force use of the overloaded constructor!
    InexorDescriptorBundle() = delete;

    /// @brief Allow name and descriptor pool to be set in constructor only!
    InexorDescriptorBundle(const std::string &internal_descriptor_set_name, std::shared_ptr<InexorDescriptorPool> descriptor_pool)
        : name(internal_descriptor_set_name), associated_descriptor_pool(descriptor_pool) {}

    const std::string name;

    const std::shared_ptr<InexorDescriptorPool> associated_descriptor_pool;

    VkDescriptorSetLayout descriptor_set_layout;

    std::vector<VkDescriptorSet> descriptor_sets;

    std::vector<VkWriteDescriptorSet> write_descriptor_sets;

    std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings;
};

} // namespace inexor::vulkan_renderer
