#pragma once

#include <vk_mem_alloc.h>

#include <functional>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::commands {
/// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::synchronization {
/// Forward declaration
class PipelineBarrierBuilder;
} // namespace inexor::vulkan_renderer::wrapper::synchronization

namespace inexor::vulkan_renderer::tools {
/// Forward declarations
class InexorException;
class VulkanException;
} // namespace inexor::vulkan_renderer::tools

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class RenderGraph;
class StagingBuffer;

// Using declarations
using tools::InexorException;
using tools::VulkanException;
using wrapper::commands::CommandBuffer;
using wrapper::core::Device;

/// The supported buffer types
/// Based on the buffer type, the rendergraph will use Vulkan Memory Allocator to create the buffers
enum class BufferType {
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
    // @TODO: Support more buffer types (storage buffer, indirect buffer...)
};

enum class BufferUpdateMode {
    DEVICE_LOCAL,
    PER_FRAME_DEVICE_LOCAL,
    PER_FRAME_HOST_VISIBLE,
};

struct PendingBufferCopy {
    VkBuffer src_buffer{VK_NULL_HANDLE};
    VkBuffer dst_buffer{VK_NULL_HANDLE};
    VkBufferCopy region{};
    VkPipelineStageFlags2 dst_stage_mask{VK_PIPELINE_STAGE_2_NONE};
    VkAccessFlags2 dst_access_mask{VK_ACCESS_2_NONE};
};

class Buffer {
    friend render_graph::RenderGraph;

private:
    struct PerFrameBufferResources {
        VkBuffer m_buffer{VK_NULL_HANDLE};
        VmaAllocation m_alloc{VK_NULL_HANDLE};
        VmaAllocationInfo m_alloc_info{};
        std::size_t m_buffer_capacity{0};
        VkDescriptorBufferInfo m_descriptor_buffer_info{};
    };

    /// The device wrapper
    const Device &m_device;
    /// The internal debug name of the buffer resource
    std::string m_name;

    /// The buffer type will be set depending on which constructor of the Buffer wrapper is called by rendergraph. The
    /// engine currently supports three different types of buffers in the Buffer wrapper class: vertex buffers, index
    /// buffers, and uniform buffers. The instances of the Buffer wrapper class are managed by rendergraph only. One
    /// solution to deal with the different buffer types would be to use a BufferBase class and to make three distinct
    /// classes VertexBuffer, IndexBuffer, and UniformBuffer. However, we aimed for simplicity and wanted to avoid
    /// polymorphism in the rendergraph for performance reasons. We also refrained from using templates for this use
    /// case. Therefore, we have chosen to use only one Buffer wrapper class which contains members for all three
    /// different buffer types. The type of the buffer will be set depending on which Buffer constructor is called by
    /// rendergraph. The actual memory management for the buffers is done by Vulkan Memory Allocator (VMA) internally.
    BufferType m_buffer_type;

    BufferUpdateMode m_update_mode{BufferUpdateMode::DEVICE_LOCAL};

    /// The buffer update function which is called by rendergraph to update the buffer's data. This update function is
    /// called, no matter what the type of the buffer is. With the currently supported buffer types (vertex-, index-,
    /// and uniform buffers) there is always a discussion about whether some update lambdas can be made std::optional.
    /// For example we could have one vertex buffer with an index buffer and the index buffer is updated together with
    /// the vertex buffer in the update function of the vertex buffer. From the design of the engine there is no
    /// limitation which buffer is updated in which update function, as long as the handle to that buffer has been
    /// created in rendergraph. In our example, the update function of the index buffer could be std::nullopt. In this
    /// case, rendergraph could separate all buffers into those which require an update and those who do not. For
    /// simplicity however, we made the update function not std::optional.
    std::function<void()> m_on_check_for_update;

    /// NOTE: It's the responsibility of the programmer to make sure the data m_src_data points to is still valid when
    /// update_buffer() is called!
    void *m_src_data{nullptr};
    std::size_t m_src_data_size{0};
    bool m_update_requested{false};
    bool m_descriptor_resource_changed{true};

    std::vector<PerFrameBufferResources> m_slots{1};
    std::size_t m_frame_slot_count{1};
    std::size_t m_current_frame_slot{0};

    /// Create the buffer using Vulkan Memory Allocator (VMA) library
    /// @param pending_buffer_copies A list of copy requests to be recorded later in a batched form
    /// @param upload_buffer The shared upload arena buffer
    /// @param upload_alloc The shared upload arena allocation
    /// @param upload_offset Current write offset inside the shared upload arena buffer
    /// @param pending_releases Deferred resource releases that must happen after GPU completion
    void create(std::vector<PendingBufferCopy> &pending_buffer_copies, StagingBuffer &staging_buffer,
                std::size_t &upload_offset, std::vector<std::function<void()>> &pending_releases);

    [[nodiscard]] bool can_update_without_command_buffer() const;

    void update_without_command_buffer();

    void create_per_frame_buffer_resources(PerFrameBufferResources &resources,
                                           std::vector<PendingBufferCopy> &pending_buffer_copies,
                                           StagingBuffer &staging_buffer, std::size_t &upload_offset,
                                           std::vector<std::function<void()>> &pending_releases,
                                           std::size_t slot_index);

    void destroy_per_frame_buffer_resources(PerFrameBufferResources &resources);

    [[nodiscard]] PerFrameBufferResources &current_frame_resources();

    [[nodiscard]] const PerFrameBufferResources &current_frame_resources() const;

    void set_frame_context(std::size_t frame_slot_count, std::size_t current_frame_slot);

    /// Destroy the buffer and the staging buffer
    void destroy_all();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param name The buffer name
    /// @param type The buffer type
    /// @param on_update The buffer update function
    Buffer(const Device &device, std::string name, BufferType type, std::function<void()> on_update,
           BufferUpdateMode update_mode = BufferUpdateMode::DEVICE_LOCAL);

    Buffer(const Buffer &) = delete;
    Buffer(Buffer &&other) noexcept;

    ~Buffer();

    Buffer &operator=(const Buffer &) = delete;
    Buffer &operator=(Buffer &&) = delete;

    [[nodiscard]] auto buffer() const {
        return current_frame_resources().m_buffer;
    }

    [[nodiscard]] const auto *buffer_address() const {
        return &current_frame_resources().m_buffer;
    }

    [[nodiscard]] const auto *descriptor_buffer_info() const {
        return &current_frame_resources().m_descriptor_buffer_info;
    }

    [[nodiscard]] auto name() const {
        return m_name;
    }

    /// Request a buffer update
    /// @param src_data A pointer to the data to copy the updated data from
    /// @warning It is the responsibility of the programmer to make sure src_data still points to valid memory when
    /// update_buffer() is called!
    /// @param src_data_size The size of the data to copy
    void request_update(void *src_data, std::size_t src_data_size);

    /// Request a buffer update
    /// @tparam BufferDataType
    /// @param data
    template <typename BufferDataType>
    void request_update(BufferDataType &data) {
        return request_update(std::addressof(data), sizeof(data));
    }

    /// Request a buffer update
    /// @tparam BufferDataType
    /// @param data
    template <typename BufferDataType>
    void request_update(std::vector<BufferDataType> &data) {
        return request_update(data.data(), sizeof(BufferDataType) * data.size());
    }

    [[nodiscard]] auto type() const {
        return m_buffer_type;
    }

    [[nodiscard]] auto update_mode() const {
        return m_update_mode;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
