#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer {

///
///
///
struct DescriptorPool {

    /// Force use of the overloaded constructor!
    DescriptorPool() = delete;

    ///
    DescriptorPool(const std::string &internal_descriptor_pool_name, const std::vector<VkDescriptorPoolSize> &pool_sizes)
        : name(internal_descriptor_pool_name), sizes(pool_sizes) {}

    const std::string name;

    const std::vector<VkDescriptorPoolSize> sizes;

    VkDescriptorPool pool;
};

} // namespace inexor::vulkan_renderer
