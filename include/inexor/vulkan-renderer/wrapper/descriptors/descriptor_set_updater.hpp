#pragma once

#include <volk.h>

#include <vector>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
}

namespace inexor::vulkan_renderer::wrapper::descriptors {

/// A wrapper class for batching calls to vkUpdateDescriptorSets
class DescriptorSetUpdater {
private:
    const Device &m_device;
    std::vector<VkWriteDescriptorSet> m_write_sets;
    std::uint32_t m_binding{0};

public:
    /// Default constructor
    /// @param device The device wrapper
    DescriptorSetUpdater(const Device &device);

    DescriptorSetUpdater(const DescriptorSetUpdater &) = default;
    DescriptorSetUpdater(DescriptorSetUpdater &&) noexcept;
    ~DescriptorSetUpdater() = default;

    DescriptorSetUpdater &operator=(const DescriptorSetUpdater &) = default;
    DescriptorSetUpdater &operator=(DescriptorSetUpdater &&) noexcept;

    /// Add a write descriptor set for a uniform buffer
    /// @param descriptor_set The destination descriptor set
    /// @param buffer_info The descriptor buffer info
    /// @note It is the responsibility of the caller to keep the memory of the descriptor buffer info alive!
    /// @param descriptor_count The descriptor count (``0`` by default)
    /// @param dst_array_element The start byte offset within the binding (``0`` by default)
    /// @return A reference to the dereferenced this pointer
    DescriptorSetUpdater &add_uniform_buffer_update(VkDescriptorSet descriptor_set,
                                                    const VkDescriptorBufferInfo *buffer_info,
                                                    std::uint32_t descriptor_count = 1,
                                                    std ::uint32_t dst_array_element = 0);

    /// Add a write descriptor set for a combined image sampler
    /// @param descriptor_set The destination descriptor set
    /// @param img_info The descriptor image info
    /// @note It is the responsibility of the caller to keep the memory of the descriptor image info alive!
    /// @param descriptor_count The descriptor count (``0`` by default)
    /// @param dst_array_element The start byte offset within the binding (``0`` by default)
    /// @return A reference to the dereferenced this pointer
    DescriptorSetUpdater &add_combined_image_sampler_update(VkDescriptorSet descriptor_set,
                                                            const VkDescriptorImageInfo *img_info,
                                                            std::uint32_t descriptor_count = 1,
                                                            std ::uint32_t dst_array_element = 0);

    /// Call vkUpdateDescriptorSets
    void update_descriptor_sets();
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
