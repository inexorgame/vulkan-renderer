#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_allocator.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/per_frame_descriptor_sets.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/write_descriptor_set_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::render_graph {

class ResourceDescriptorManager {
public:
    using OnBuildDescriptorSetLayout =
        std::function<VkDescriptorSetLayout(wrapper::descriptors::DescriptorSetLayoutBuilder &)>;
    using OnBuildWriteDescriptorSet = std::function<std::vector<VkWriteDescriptorSet>(
        wrapper::descriptors::WriteDescriptorSetBuilder &, VkDescriptorSet)>;

private:
    struct ResourceDescriptor {
        std::string name;
        std::shared_ptr<wrapper::descriptors::PerFrameDescriptorSets> resource;
        OnBuildDescriptorSetLayout on_build_descriptor_set_layout;
        OnBuildWriteDescriptorSet on_build_write_descriptor_sets;
        VkDescriptorSetLayout descriptor_set_layout{VK_NULL_HANDLE};
    };

    wrapper::Device &m_device;
    wrapper::descriptors::DescriptorSetLayoutBuilder m_descriptor_set_layout_builder;
    wrapper::descriptors::DescriptorSetAllocator m_descriptor_set_allocator;
    wrapper::descriptors::WriteDescriptorSetBuilder m_write_descriptor_set_builder;
    std::vector<VkWriteDescriptorSet> m_write_descriptor_sets;
    std::vector<ResourceDescriptor> m_resource_descriptors;
    std::vector<VkBool32> m_descriptor_sets_slot_dirty;
    std::vector<std::vector<VkBool32>> m_descriptor_resource_slot_dirty;
    bool m_descriptor_sets_dirty{true};
    std::size_t m_frame_slot_count{1};
    std::size_t m_current_frame_slot{0};

    void sync_descriptor_resource_dirty_tracking();

public:
    explicit ResourceDescriptorManager(wrapper::Device &device);

    [[nodiscard]] std::weak_ptr<wrapper::descriptors::PerFrameDescriptorSets>
    add_resource_descriptor(std::string name, OnBuildDescriptorSetLayout on_build_descriptor_set_layout,
                            OnBuildWriteDescriptorSet on_build_write_descriptor_set);

    void set_frame_context(std::size_t frame_slot_count, std::size_t current_frame_slot);

    void create_descriptor_set_layouts();

    void mark_descriptor_sets_dirty();

    [[nodiscard]] bool descriptor_sets_dirty() const {
        return m_descriptor_sets_dirty;
    }

    [[nodiscard]] bool update_write_descriptor_sets();

    void clear();
};

} // namespace inexor::vulkan_renderer::render_graph
