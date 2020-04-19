#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace inexor {
namespace vulkan_renderer {

///
///
///
struct InexorDescriptorPool {

    /// Force use of the overloaded constructor!
    InexorDescriptorPool() = delete;

    ///
    InexorDescriptorPool(const std::string &internal_descriptor_pool_name, const std::vector<VkDescriptorPoolSize> &pool_sizes)
        : name(internal_descriptor_pool_name), sizes(pool_sizes) {}

    const std::string name;

    const std::vector<VkDescriptorPoolSize> sizes;

    VkDescriptorPool pool;
};

}; // namespace vulkan_renderer
}; // namespace inexor
