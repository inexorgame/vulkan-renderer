#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::descriptors {

WriteDescriptorSetBuilder::WriteDescriptorSetBuilder(const Device &device) : m_device(device) {}

std::vector<VkWriteDescriptorSet> WriteDescriptorSetBuilder::build() {
    auto write_descriptor_sets = std::move(m_write_descriptor_sets);
    // The C++ standard guarantees that the memory which has been moved from is in a valid but unspecified state.
    // By invoking clear() on the vector, we bring the memory back into a specified state.
    m_write_descriptor_sets.clear();
    return write_descriptor_sets;
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
