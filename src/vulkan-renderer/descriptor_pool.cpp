#include "inexor/vulkan-renderer/descriptor_pool.hpp"

namespace inexor::vulkan_renderer {
DescriptorPool::DescriptorPool(const std::string &internal_descriptor_pool_name, const std::vector<VkDescriptorPoolSize> &pool_sizes)
    : name(internal_descriptor_pool_name), sizes(pool_sizes) {}
} // namespace inexor::vulkan_renderer
