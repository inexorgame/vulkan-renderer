#include "inexor/vulkan-renderer/render-graph/buffer.hpp"

#include "inexor/vulkan-renderer/render-graph/staging_buffer.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <utility>

namespace inexor::vulkan_renderer::render_graph {

Buffer::Buffer(const Device &device, std::string buffer_name, BufferType buffer_type, std::function<void()> on_update,
                             const BufferUpdateMode update_mode)
        : m_device(device), m_name(std::move(buffer_name)), m_on_check_for_update(std::move(on_update)),
            m_buffer_type(buffer_type), m_update_mode(update_mode) {
    if (m_name.empty()) {
        throw InexorException("Error: Parameter 'buffer_name' is an empty string!");
    }
}

Buffer::Buffer(Buffer &&other) noexcept : m_device(other.m_device) {
    m_name = std::move(other.m_name);
    m_buffer_type = other.m_buffer_type;
    m_update_mode = other.m_update_mode;
    m_on_check_for_update = std::move(other.m_on_check_for_update);
    m_src_data = std::exchange(other.m_src_data, nullptr);
    m_src_data_size = other.m_src_data_size;
    m_update_requested = other.m_update_requested;
    m_descriptor_resource_changed = other.m_descriptor_resource_changed;
    m_slots = std::move(other.m_slots);
    m_frame_slot_count = other.m_frame_slot_count;
    m_current_frame_slot = other.m_current_frame_slot;
}

Buffer::~Buffer() {
    destroy_all();
}

bool Buffer::can_update_without_command_buffer() const {
    if (m_update_mode != BufferUpdateMode::PER_FRAME_HOST_VISIBLE) {
        return false;
    }

    for (const auto &slot : m_slots) {
        if (slot.m_buffer == VK_NULL_HANDLE || slot.m_alloc_info.pMappedData == nullptr ||
            slot.m_buffer_capacity < m_src_data_size) {
            return false;
        }
    }

    return true;
}

void Buffer::update_without_command_buffer() {
    auto &slot = current_frame_resources();
    if (const auto result =
            vmaCopyMemoryToAllocation(m_device.allocator(), m_src_data, slot.m_alloc, 0, m_src_data_size);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vmaCopyMemoryToAllocation failed for buffer!", result, m_name);
    }

    m_descriptor_resource_changed = false;
    m_update_requested = false;
    m_src_data = nullptr;
    m_src_data_size = 0;
}

void Buffer::create(std::vector<PendingBufferCopy> &pending_buffer_copies, StagingBuffer &staging_buffer,
                    std::size_t &upload_offset, std::vector<std::function<void()>> &pending_releases) {
    if (can_update_without_command_buffer()) {
        update_without_command_buffer();
        return;
    }

    m_descriptor_resource_changed = false;

    auto &slot = current_frame_resources();
    const auto slot_index = m_current_frame_slot;
    create_per_frame_buffer_resources(slot, pending_buffer_copies, staging_buffer, upload_offset, pending_releases,
                                      slot_index);

    m_update_requested = false;
    m_src_data = nullptr;
    m_src_data_size = 0;
}

void Buffer::create_per_frame_buffer_resources(PerFrameBufferResources &slot,
                                               std::vector<PendingBufferCopy> &pending_buffer_copies,
                                               StagingBuffer &staging_buffer, std::size_t &upload_offset,
                                               std::vector<std::function<void()>> &pending_releases,
                                               const std::size_t slot_index) {

    auto grow_capacity = [](const std::size_t current_capacity, const std::size_t required_capacity) {
        if (current_capacity >= required_capacity) {
            return current_capacity;
        }
        auto capacity = current_capacity == 0 ? required_capacity : current_capacity;
        while (capacity < required_capacity) {
            capacity *= 2;
        }
        return capacity;
    };

    const std::unordered_map<BufferType, VkBufferUsageFlags> buffer_usage{
        {BufferType::UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
        {BufferType::VERTEX_BUFFER, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
        {BufferType::INDEX_BUFFER, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
    };

    const auto slot_name = m_slots.size() > 1 ? m_name + "[slot " + std::to_string(slot_index) + "]" : m_name;
    const auto required_buffer_capacity = grow_capacity(slot.m_buffer_capacity, m_src_data_size);

    const bool host_visible_mapped = m_update_mode == BufferUpdateMode::PER_FRAME_HOST_VISIBLE;

    const VmaAllocationCreateInfo alloc_ci{
        .flags = host_visible_mapped
                     ? static_cast<VmaAllocationCreateFlags>(VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                                                             VMA_ALLOCATION_CREATE_MAPPED_BIT)
                     : 0,
        .usage = host_visible_mapped ? VMA_MEMORY_USAGE_AUTO : VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
        .priority =
            (m_buffer_type == BufferType::INDEX_BUFFER || m_buffer_type == BufferType::VERTEX_BUFFER) ? 0.9f : 0.7f,
    };

    const bool needs_fresh_device_local_buffer = m_update_mode == BufferUpdateMode::DEVICE_LOCAL;

    if (slot.m_buffer == VK_NULL_HANDLE || slot.m_buffer_capacity < m_src_data_size || needs_fresh_device_local_buffer) {
        const auto old_buffer = std::exchange(slot.m_buffer, VK_NULL_HANDLE);
        const auto old_alloc = std::exchange(slot.m_alloc, VK_NULL_HANDLE);
        slot.m_alloc_info = {};
        slot.m_descriptor_buffer_info = {};

        const auto buffer_ci = tools::make_info<VkBufferCreateInfo>({
            .size = required_buffer_capacity,
            .usage = buffer_usage.at(m_buffer_type),
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        });

        if (const auto result = vmaCreateBuffer(m_device.allocator(), &buffer_ci, &alloc_ci, &slot.m_buffer,
                                                &slot.m_alloc, &slot.m_alloc_info);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaCreateBuffer failed!", result, m_name);
        }

        vmaSetAllocationName(m_device.allocator(), slot.m_alloc, slot_name.c_str());
        m_device.set_debug_name(slot.m_buffer, slot_name);
        slot.m_buffer_capacity = required_buffer_capacity;
        m_descriptor_resource_changed = true;

        if (old_buffer != VK_NULL_HANDLE) {
            const auto allocator = m_device.allocator();
            pending_releases.push_back(
                [allocator, old_buffer, old_alloc] { vmaDestroyBuffer(allocator, old_buffer, old_alloc); });
        }
    }

    VkMemoryPropertyFlags mem_prop_flags{};
    vmaGetAllocationMemoryProperties(m_device.allocator(), slot.m_alloc, &mem_prop_flags);

    if (mem_prop_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        if (const auto result =
                vmaCopyMemoryToAllocation(m_device.allocator(), m_src_data, slot.m_alloc, 0, m_src_data_size);
            result != VK_SUCCESS) {
            throw VulkanException("Error: vmaCopyMemoryToAllocation failed for buffer!", result, m_name);
        }
    } else {
        upload_offset = (upload_offset + 15) & ~std::size_t(15);
        const auto src_offset = static_cast<VkDeviceSize>(upload_offset);
        std::memcpy(static_cast<std::byte *>(staging_buffer.mapped_data()) + upload_offset, m_src_data,
                    m_src_data_size);
        upload_offset += m_src_data_size;

        const auto [dst_stage_mask, dst_access_mask] = [&]() -> std::pair<VkPipelineStageFlags2, VkAccessFlags2> {
            switch (m_buffer_type) {
            case BufferType::INDEX_BUFFER:
                return {VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT};
            case BufferType::VERTEX_BUFFER:
                return {VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT};
            case BufferType::UNIFORM_BUFFER:
            default:
                return {VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
                        VK_ACCESS_2_UNIFORM_READ_BIT};
            }
        }();

        pending_buffer_copies.push_back({
            .src_buffer = staging_buffer.buffer(),
            .dst_buffer = slot.m_buffer,
            .region =
                {
                    .srcOffset = src_offset,
                    .dstOffset = 0,
                    .size = m_src_data_size,
                },
            .dst_stage_mask = dst_stage_mask,
            .dst_access_mask = dst_access_mask,
        });
    }

    slot.m_descriptor_buffer_info = {
        .buffer = slot.m_buffer,
        .offset = 0,
        .range = m_src_data_size,
    };
}

void Buffer::destroy_all() {
    for (auto &slot : m_slots) {
        destroy_per_frame_buffer_resources(slot);
    }
}

void Buffer::destroy_per_frame_buffer_resources(PerFrameBufferResources &slot) {
    if (slot.m_buffer == VK_NULL_HANDLE && slot.m_alloc == VK_NULL_HANDLE) {
        return;
    }

    vmaDestroyBuffer(m_device.allocator(), slot.m_buffer, slot.m_alloc);
    slot.m_buffer = VK_NULL_HANDLE;
    slot.m_alloc = VK_NULL_HANDLE;
    slot.m_alloc_info = {};
    slot.m_buffer_capacity = 0;
    slot.m_descriptor_buffer_info = {};
}

Buffer::PerFrameBufferResources &Buffer::current_frame_resources() {
    return m_slots.at(m_current_frame_slot);
}

const Buffer::PerFrameBufferResources &Buffer::current_frame_resources() const {
    return m_slots.at(m_current_frame_slot);
}

void Buffer::set_frame_context(const std::size_t frame_slot_count, const std::size_t current_frame_slot) {
    const auto desired_slot_count =
        (m_update_mode == BufferUpdateMode::DEVICE_LOCAL) ? 1u : std::max<std::size_t>(1, frame_slot_count);

    if (desired_slot_count < m_slots.size()) {
        for (std::size_t slot_index = desired_slot_count; slot_index < m_slots.size(); ++slot_index) {
            destroy_per_frame_buffer_resources(m_slots[slot_index]);
        }
    }
    if (m_slots.size() != desired_slot_count) {
        m_slots.resize(desired_slot_count);
    }
    m_frame_slot_count = desired_slot_count;
    m_current_frame_slot = std::min(current_frame_slot, m_slots.size() - 1);
}

void Buffer::request_update(void *src_data, const std::size_t src_data_size) {
    if (src_data == nullptr) {
        throw std::runtime_error("Error: Parameter 'src_data' is nullptr!");
    }
    if (src_data_size == 0) {
        throw std::runtime_error("Error: Parameter 'src_data_size' is 0!");
    }
    m_src_data = src_data;
    m_src_data_size = src_data_size;
    m_update_requested = true;
}

} // namespace inexor::vulkan_renderer::render_graph
