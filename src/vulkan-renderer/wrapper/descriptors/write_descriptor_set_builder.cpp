#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/core/device.hpp"

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::descriptors {

WriteDescriptorSetBuilder::WriteDescriptorSetBuilder(const core::Device &device) : m_device(device) {}

std::vector<VkWriteDescriptorSet> WriteDescriptorSetBuilder::build() {
    std::vector<VkWriteDescriptorSet> write_descriptor_sets;
    write_descriptor_sets.swap(m_write_descriptor_sets);
    return write_descriptor_sets;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
