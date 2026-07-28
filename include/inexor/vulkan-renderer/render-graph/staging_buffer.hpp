#pragma once

#include <vk_mem_alloc.h>

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::tools {
/// Forward declarations
class InexorException;
class VulkanException;
} // namespace inexor::vulkan_renderer::tools

namespace inexor::vulkan_renderer::render_graph {

using tools::InexorException;
using tools::VulkanException;
using wrapper::Device;

class StagingBuffer {
private:
    struct PerFrameStagingBufferResources {
        VkBuffer m_buffer{VK_NULL_HANDLE};
        VmaAllocation m_alloc{VK_NULL_HANDLE};
        VmaAllocationInfo m_alloc_info{};
        std::size_t m_capacity{0};
    };

    const Device &m_device;
    std::string m_name;
    std::vector<PerFrameStagingBufferResources> m_slots{1};
    std::size_t m_frame_slot_count{1};
    std::size_t m_current_frame_slot{0};

    [[nodiscard]] PerFrameStagingBufferResources &current_frame_resources();
    [[nodiscard]] const PerFrameStagingBufferResources &current_frame_resources() const;

    void destroy_per_frame_resources(PerFrameStagingBufferResources &resources);

public:
    StagingBuffer(const Device &device, std::string name);
    StagingBuffer(const StagingBuffer &) = delete;
    StagingBuffer(StagingBuffer &&) noexcept = delete;
    ~StagingBuffer();

    StagingBuffer &operator=(const StagingBuffer &) = delete;
    StagingBuffer &operator=(StagingBuffer &&) = delete;

    void set_frame_context(std::size_t frame_slot_count, std::size_t current_frame_slot);

    void ensure_capacity(std::size_t required_bytes, std::vector<std::function<void()>> &pending_releases);

    void reset();

    [[nodiscard]] VkBuffer buffer() const;
    [[nodiscard]] VmaAllocation allocation() const;
    [[nodiscard]] void *mapped_data() const;
};

} // namespace inexor::vulkan_renderer::render_graph