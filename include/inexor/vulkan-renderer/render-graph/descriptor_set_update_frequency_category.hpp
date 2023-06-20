#pragma once

namespace inexor::vulkan_renderer::render_graph {

/// In Vulkan, it is recommended to keep VkDescriptorSets grouped by their update frequency.
enum class DescriptorSetUpdateFrequencyCategory {
    STATIC,    // Descriptor sets with infrequent updates
    PER_BATCH, // Descriptor sets updated per batch
    PER_FRAME, // Descriptor sets updated once per frame
    DYNAMIC,   // Descriptor sets updated frequently (multiple times per frame)
};

} // namespace inexor::vulkan_renderer::render_graph
