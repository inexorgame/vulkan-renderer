#pragma once

#include <volk.h>

#include <algorithm>
#include <cstddef>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::descriptors {

class PerFrameDescriptorSets {
private:
    VkDescriptorSetLayout m_descriptor_set_layout{VK_NULL_HANDLE};
    VkPipelineLayout m_pipeline_layout{VK_NULL_HANDLE};
    std::vector<VkDescriptorSet> m_descriptor_sets{1, VK_NULL_HANDLE};
    std::size_t m_current_frame_slot{0};

public:
    void set_frame_context(const std::size_t frame_slot_count, const std::size_t current_frame_slot) {
        const auto slot_count = std::max<std::size_t>(1, frame_slot_count);
        if (m_descriptor_sets.size() != slot_count) {
            m_descriptor_sets.resize(slot_count, VK_NULL_HANDLE);
        }
        m_current_frame_slot = std::min(current_frame_slot, m_descriptor_sets.size() - 1);
    }

    void set_descriptor_set(const std::size_t slot_index, const VkDescriptorSet descriptor_set) {
        if (slot_index >= m_descriptor_sets.size()) {
            m_descriptor_sets.resize(slot_index + 1, VK_NULL_HANDLE);
        }
        m_descriptor_sets[slot_index] = descriptor_set;
    }

    [[nodiscard]] VkDescriptorSet descriptor_set(const std::size_t slot_index) const {
        return m_descriptor_sets.at(slot_index);
    }

    [[nodiscard]] VkDescriptorSet current_descriptor_set() const {
        return m_descriptor_sets.at(m_current_frame_slot);
    }

    [[nodiscard]] std::size_t frame_slot_count() const {
        return m_descriptor_sets.size();
    }

    // TODO: Remove me again!
    void set_layout(VkDescriptorSetLayout descriptor_set_layout) {
        m_descriptor_set_layout = descriptor_set_layout;
    }

    [[nodiscard]] auto layout() const {
        return m_descriptor_set_layout;
    }

    void set_pipeline_layout(const VkPipelineLayout pipeline_layout) {
        m_pipeline_layout = pipeline_layout;
    }

    [[nodiscard]] VkPipelineLayout pipeline_layout() const {
        return m_pipeline_layout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors