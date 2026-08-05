#include "inexor/vulkan-renderer/render-graph/resource_descriptor_manager.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <iterator>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

ResourceDescriptorManager::ResourceDescriptorManager(Device &device)
    : m_device(device), m_descriptor_set_layout_builder(device), m_descriptor_set_allocator(device),
      m_write_descriptor_set_builder(device) {}

std::weak_ptr<wrapper::descriptors::PerFrameDescriptorSets>
ResourceDescriptorManager::add_resource_descriptor(std::string name,
                                                   OnBuildDescriptorSetLayout on_build_descriptor_set_layout,
                                                   OnBuildWriteDescriptorSet on_build_write_descriptor_set) {
    auto resource = std::make_shared<wrapper::descriptors::PerFrameDescriptorSets>();
    resource->set_frame_context(m_frame_slot_count, m_current_frame_slot);

    m_resource_descriptors.emplace_back(ResourceDescriptor{
        .name = std::move(name),
        .resource = resource,
        .on_build_descriptor_set_layout = std::move(on_build_descriptor_set_layout),
        .on_build_write_descriptor_sets = std::move(on_build_write_descriptor_set),
    });

    sync_descriptor_resource_dirty_tracking();
    for (auto &slot_dirty : m_descriptor_resource_slot_dirty) {
        if (!slot_dirty.empty()) {
            slot_dirty.back() = true;
        }
    }
    mark_descriptor_sets_dirty();

    return resource;
}

void ResourceDescriptorManager::set_frame_context(const std::size_t frame_slot_count,
                                                  const std::size_t current_frame_slot) {
    m_frame_slot_count = std::max<std::size_t>(1, frame_slot_count);
    m_current_frame_slot = std::min(current_frame_slot, m_frame_slot_count - 1);

    if (m_descriptor_sets_slot_dirty.size() != m_frame_slot_count) {
        m_descriptor_sets_slot_dirty.resize(m_frame_slot_count, true);
    }

    sync_descriptor_resource_dirty_tracking();

    for (const auto &descriptor : m_resource_descriptors) {
        descriptor.resource->set_frame_context(m_frame_slot_count, m_current_frame_slot);
    }
}

void ResourceDescriptorManager::create_descriptor_set_layouts() {
    spdlog::trace("Creating {} descriptor set layouts", m_resource_descriptors.size());
    for (auto &descriptor : m_resource_descriptors) {
        descriptor.descriptor_set_layout =
            std::invoke(descriptor.on_build_descriptor_set_layout, m_descriptor_set_layout_builder);
        descriptor.resource->set_layout(descriptor.descriptor_set_layout);
    }
}

void ResourceDescriptorManager::mark_descriptor_sets_dirty() {
    if (m_descriptor_sets_slot_dirty.size() != m_frame_slot_count) {
        m_descriptor_sets_slot_dirty.assign(m_frame_slot_count, true);
    } else {
        std::fill(m_descriptor_sets_slot_dirty.begin(), m_descriptor_sets_slot_dirty.end(), true);
    }

    sync_descriptor_resource_dirty_tracking();
    for (auto &slot_dirty : m_descriptor_resource_slot_dirty) {
        std::fill(slot_dirty.begin(), slot_dirty.end(), true);
    }

    m_descriptor_sets_dirty = true;
}
bool ResourceDescriptorManager::update_write_descriptor_sets() {
    sync_descriptor_resource_dirty_tracking();
    m_write_descriptor_sets.clear();

    const auto slot_index = m_current_frame_slot;
    auto &slot_resource_dirty = m_descriptor_resource_slot_dirty.at(slot_index);

    std::size_t dirty_descriptor_count = 0;
    for (const auto is_dirty : slot_resource_dirty) {
        if (is_dirty) {
            ++dirty_descriptor_count;
        }
    }

    spdlog::trace("Updating descriptor sets [slot={}, total={}, dirty={}]", slot_index, m_resource_descriptors.size(),
                  dirty_descriptor_count);

    bool any_descriptor_changes = false;
    for (std::size_t descriptor_index = 0; descriptor_index < m_resource_descriptors.size(); ++descriptor_index) {
        auto &descriptor = m_resource_descriptors[descriptor_index];
        descriptor.resource->set_frame_context(m_frame_slot_count, m_current_frame_slot);

        if (descriptor.resource->descriptor_set(slot_index) == VK_NULL_HANDLE) {
            descriptor.resource->set_descriptor_set(
                slot_index,
                m_descriptor_set_allocator.allocate(descriptor.name + "[slot " + std::to_string(slot_index) + "]",
                                                    descriptor.descriptor_set_layout));
            slot_resource_dirty[descriptor_index] = true;
            any_descriptor_changes = true;
            spdlog::trace("Descriptor set allocated [{}, slot={}]", descriptor.name, slot_index);
        }

        if (!slot_resource_dirty[descriptor_index]) {
            continue;
        }

        auto write_descriptor_sets =
            std::invoke(descriptor.on_build_write_descriptor_sets, m_write_descriptor_set_builder,
                        descriptor.resource->descriptor_set(slot_index));

        spdlog::trace("Descriptor writes built [{}, slot={}, writes={}]", descriptor.name, slot_index,
                      write_descriptor_sets.size());

        std::move(write_descriptor_sets.begin(), write_descriptor_sets.end(),
                  std::back_inserter(m_write_descriptor_sets));

        slot_resource_dirty[descriptor_index] = false;
        any_descriptor_changes = true;
    }

    if (!m_write_descriptor_sets.empty()) {
        spdlog::trace("Descriptor writes submitted [slot={}, writes={}]", slot_index, m_write_descriptor_sets.size());
        m_device.update_descriptor_sets(m_write_descriptor_sets);
    }

    m_descriptor_sets_slot_dirty.at(slot_index) = VK_FALSE;

    m_descriptor_sets_dirty = std::any_of(m_descriptor_sets_slot_dirty.begin(), m_descriptor_sets_slot_dirty.end(),
                                          [](const VkBool32 dirty) { return dirty != VK_FALSE; });
    return any_descriptor_changes;
}

void ResourceDescriptorManager::clear() {
    m_write_descriptor_sets.clear();
    m_resource_descriptors.clear();
    m_descriptor_sets_slot_dirty.clear();
    m_descriptor_resource_slot_dirty.clear();
    m_descriptor_sets_dirty = true;
    m_frame_slot_count = 1;
    m_current_frame_slot = 0;
}

void ResourceDescriptorManager::sync_descriptor_resource_dirty_tracking() {
    const auto resource_count = m_resource_descriptors.size();

    if (m_descriptor_resource_slot_dirty.size() != m_frame_slot_count) {
        m_descriptor_resource_slot_dirty.assign(m_frame_slot_count, std::vector<VkBool32>(resource_count, VK_TRUE));
        return;
    }

    for (auto &slot_dirty : m_descriptor_resource_slot_dirty) {
        if (slot_dirty.size() < resource_count) {
            const auto previous_size = slot_dirty.size();
            slot_dirty.resize(resource_count, VK_TRUE);
            std::fill(slot_dirty.begin() + static_cast<std::ptrdiff_t>(previous_size), slot_dirty.end(), VK_TRUE);
        } else if (slot_dirty.size() > resource_count) {
            slot_dirty.resize(resource_count);
        }
    }
}

} // namespace inexor::vulkan_renderer::render_graph
