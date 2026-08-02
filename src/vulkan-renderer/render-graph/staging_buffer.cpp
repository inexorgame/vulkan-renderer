#include "inexor/vulkan-renderer/render-graph/staging_buffer.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

using tools::make_info;

StagingBuffer::StagingBuffer(const wrapper::core::Device &device, std::string name)
    : m_device(device), m_name(std::move(name)) {
    if (m_name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
}

StagingBuffer::~StagingBuffer() {
    reset();
}

void StagingBuffer::ensure_capacity(const std::size_t required_bytes,
                                    std::vector<std::function<void()>> &pending_releases) {
    if (required_bytes == 0) {
        return;
    }

    auto &resources = current_frame_resources();
    if (resources.m_buffer != VK_NULL_HANDLE && resources.m_capacity >= required_bytes) {
        return;
    }

    const auto capacity = required_bytes;

    const auto old_buffer = std::exchange(resources.m_buffer, VK_NULL_HANDLE);
    const auto old_alloc = std::exchange(resources.m_alloc, VK_NULL_HANDLE);
    resources.m_alloc_info = {};

    const auto buffer_ci = make_info<VkBufferCreateInfo>({
        .size = capacity,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    });

    const VmaAllocationCreateInfo alloc_ci{
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO_PREFER_HOST,
        .priority = 0.1f,
    };

    if (const auto result = vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &resources.m_buffer,
                                            &resources.m_alloc, &resources.m_alloc_info);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCreateBuffer failed!", result, m_name);
    }

    resources.m_capacity = capacity;
    const auto slot_name = m_slots.size() > 1 ? m_name + "[slot " + std::to_string(m_current_frame_slot) + "]" : m_name;
    vmaSetAllocationName(m_device.allocator(), resources.m_alloc, slot_name.c_str());
    m_device.set_debug_name(resources.m_buffer, slot_name);

    if (old_buffer != VK_NULL_HANDLE) {
        const auto allocator = m_device.allocator();
        pending_releases.push_back(
            [allocator, old_buffer, old_alloc] { vmaDestroyBuffer(allocator, old_buffer, old_alloc); });
    }
}

void StagingBuffer::reset() {
    for (auto &resources : m_slots) {
        destroy_per_frame_resources(resources);
    }
}

VkBuffer StagingBuffer::buffer() const {
    return current_frame_resources().m_buffer;
}

VmaAllocation StagingBuffer::allocation() const {
    return current_frame_resources().m_alloc;
}

void *StagingBuffer::mapped_data() const {
    return current_frame_resources().m_alloc_info.pMappedData;
}

void StagingBuffer::set_frame_context(const std::size_t frame_slot_count, const std::size_t current_frame_slot) {
    const auto desired_slot_count = std::max<std::size_t>(1, frame_slot_count);

    if (desired_slot_count < m_slots.size()) {
        for (std::size_t slot_index = desired_slot_count; slot_index < m_slots.size(); ++slot_index) {
            destroy_per_frame_resources(m_slots[slot_index]);
        }
    }
    if (m_slots.size() != desired_slot_count) {
        m_slots.resize(desired_slot_count);
    }

    m_frame_slot_count = desired_slot_count;
    m_current_frame_slot = std::min(current_frame_slot, m_slots.size() - 1);
}

StagingBuffer::PerFrameStagingBufferResources &StagingBuffer::current_frame_resources() {
    return m_slots.at(m_current_frame_slot);
}

const StagingBuffer::PerFrameStagingBufferResources &StagingBuffer::current_frame_resources() const {
    return m_slots.at(m_current_frame_slot);
}

void StagingBuffer::destroy_per_frame_resources(PerFrameStagingBufferResources &resources) {
    if (resources.m_buffer == VK_NULL_HANDLE && resources.m_alloc == VK_NULL_HANDLE) {
        return;
    }

    vmaDestroyBuffer(m_device.allocator(), resources.m_buffer, resources.m_alloc);
    resources.m_buffer = VK_NULL_HANDLE;
    resources.m_alloc = VK_NULL_HANDLE;
    resources.m_alloc_info = {};
    resources.m_capacity = 0;
}

} // namespace inexor::vulkan_renderer::render_graph