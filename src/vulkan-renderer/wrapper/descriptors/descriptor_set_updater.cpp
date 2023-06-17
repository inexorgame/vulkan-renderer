#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_updater.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::descriptors {

DescriptorSetUpdater::DescriptorSetUpdater(const Device &device) : m_device(device) {}

DescriptorSetUpdater &DescriptorSetUpdater::add_uniform_buffer_update(const VkDescriptorSet descriptor_set,
                                                                      const VkDescriptorBufferInfo *buffer_info,
                                                                      const std::uint32_t descriptor_count,
                                                                      const std::uint32_t dst_array_element) {
    if (buffer_info == nullptr) {
        throw std::invalid_argument("Error: descriptor buffer info is nullptr!");
    }

    m_write_sets.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstSet = descriptor_set,
        .dstBinding = m_binding,
        .dstArrayElement = dst_array_element,
        .descriptorCount = descriptor_count,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        // It is the responsibility of the caller to keep buffer_info a valid pointer
        .pBufferInfo = buffer_info,
    }));

    m_binding++;
    return *this;
}

DescriptorSetUpdater &DescriptorSetUpdater::add_combined_image_sampler_update(const VkDescriptorSet descriptor_set,
                                                                              const VkDescriptorImageInfo *img_info,
                                                                              const std::uint32_t descriptor_count,
                                                                              const std::uint32_t dst_array_element) {
    if (img_info == nullptr) {
        throw std::invalid_argument("Error: descriptor image info is nullptr!");
    }

    m_write_sets.emplace_back(wrapper::make_info<VkWriteDescriptorSet>({
        .dstSet = descriptor_set,
        .dstBinding = m_binding,
        .dstArrayElement = dst_array_element,
        .descriptorCount = descriptor_count,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        // It is the responsibility of the caller to keep image_info a valid pointer
        .pImageInfo = img_info,
    }));

    m_binding++;
    return *this;
}

void DescriptorSetUpdater::update_descriptor_sets() {
    if (!m_write_sets.empty()) {
        vkUpdateDescriptorSets(m_device.device(), static_cast<std::uint32_t>(m_write_sets.size()), m_write_sets.data(),
                               0, nullptr);
        m_write_sets.clear();
        m_binding = 0;
    }
}

} // namespace inexor::vulkan_renderer::wrapper::descriptors
