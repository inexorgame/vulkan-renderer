#pragma once

namespace inexor::vulkan_renderer::wrapper::descriptors {

/// In Vulkan, it is recommended to keep VkDescriptorSets grouped by their update frequency.
enum class DescriptorSetUpdateFrequency {
    STATIC,    // Descriptor sets with infrequent updates or no updates at all
    PER_BATCH, // Descriptor sets updated per batch
    PER_FRAME, // Descriptor sets updated once per frame
    DYNAMIC,   // Descriptor sets updated frequently (multiple times per frame)
};

} // namespace inexor::vulkan_renderer::render_graph
