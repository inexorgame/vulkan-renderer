#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Using declaration
using wrapper::InexorException;

WriteDescriptorSetBuilder::WriteDescriptorSetBuilder(const Device &device) : m_device(device) {}

std::vector<VkWriteDescriptorSet> WriteDescriptorSetBuilder::build() {
    auto write_descriptor_sets = std::move(m_write_descriptor_sets);
    reset();
    return write_descriptor_sets;
}

void WriteDescriptorSetBuilder::reset() {
    m_write_descriptor_sets.clear();
    m_binding = 0;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
