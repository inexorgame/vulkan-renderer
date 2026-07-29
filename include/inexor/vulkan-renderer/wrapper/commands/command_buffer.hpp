#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/images/image.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/fence.hpp"

#include <cassert>
#include <memory>
#include <span>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declarations
class Device;
struct QueueSemaphoreWait;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::synchronization {
// Forward declaration
class Fence;
} // namespace inexor::vulkan_renderer::wrapper::synchronization

namespace inexor::vulkan_renderer::wrapper::images {
// Forward declaration
class Image;
} // namespace inexor::vulkan_renderer::wrapper::images

namespace inexor::vulkan_renderer::wrapper::pipelines {
// Forward declaration
class GraphicsPipeline;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::wrapper::descriptors {
// Forward declaration
class PerFrameDescriptorSets;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class Buffer;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::commands {

// Using declarations
using render_graph::Buffer;
using render_graph::BufferType;
using tools::InexorException;
using wrapper::core::QueueSemaphoreWait;
using wrapper::descriptors::PerFrameDescriptorSets;
using wrapper::images::Image;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::synchronization::Fence;

/// RAII wrapper class for VkCommandBuffer.
/// @TODO Restrict access to commands which only RenderGraph should have access to (use private and friend class).
/// @TODO Switch to taking in OOP wrappers when we have them (e.g. bind_vertex_buffers takes in a VertexBuffer)
/// @TODO Make trivially copyable (this class doesn't really "own" the command buffer, more just an OOP wrapper).
class CommandBuffer {
    // The Device wrapper must be able to call begin_command_buffer and end_command_buffer
    friend class core::Device;
    friend class CommandPool;

private:
    VkCommandBuffer m_cmd_buf{VK_NULL_HANDLE};
    const core::Device &m_device;
    mutable std::string m_name;
    std::unique_ptr<Fence> m_wait_fence;
    mutable bool m_has_been_submitted{false};
    mutable std::vector<VkSemaphoreSubmitInfo> m_wait_submit_infos_scratch;
    mutable std::vector<VkSemaphoreSubmitInfo> m_signal_submit_infos_scratch;

    /// Call vkBeginCommandBuffer
    /// @param flags The command buffer usage flags, ``VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`` by default
    const CommandBuffer & // NOLINT
    begin_command_buffer(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) const;

    /// Call vkEndCommandBuffer
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_command_buffer() const; // NOLINT

    /// Set the internal debug name of the command buffer.
    /// @param name The name of the command buffer.
    void set_debug_name(const std::string &name) const;

    /// Call vkQueueSubmit2
    /// @param queue_type The queue type to submit the command buffer to
    /// @param wait_semaphore_infos The semaphores to wait for together with stage masks
    /// @param signal_semaphore_infos The semaphores to signal together with stage masks
    /// @TODO Implement submitting multiple command buffers in one batched call to vkQueueSubmit2!
    void submit(const VkQueueFlagBits queue_type, std::span<const VkSemaphoreSubmitInfo> wait_semaphore_infos,
                std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos) const;

    /// Call vkQueueSubmit2
    /// @param queue_type The queue type to submit the command buffer to
    /// @param wait_semaphore_infos The semaphores to wait for together with stage masks
    /// @param signal_semaphores The semaphores to signal
    /// @TODO Implement submitting multiple command buffers in one batched call to vkQueueSubmit2!
    void submit(const VkQueueFlagBits queue_type, std::span<const VkSemaphoreSubmitInfo> wait_semaphore_infos,
                std::span<const VkSemaphore> signal_semaphores = {}) const;

    /// Overload with plain wait semaphores and explicit signal semaphore infos.
    void submit(const VkQueueFlagBits queue_type, std::span<const VkSemaphore> wait_semaphores,
                std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos) const;

    /// Compatibility overload that applies one default stage mask for all waits based on queue type.
    void submit(const VkQueueFlagBits queue_type, std::span<const VkSemaphore> wait_semaphores = {},
                std::span<const VkSemaphore> signal_semaphores = {}) const;

    /// Compatibility overload that accepts explicit stage masks per wait semaphore.
    void submit(const VkQueueFlagBits queue_type, std::span<const QueueSemaphoreWait> wait_semaphores,
                std::span<const VkSemaphore> signal_semaphores = {}) const;

    /// Compatibility overload that accepts explicit stage masks per wait semaphore and per signal semaphore.
    void submit(const VkQueueFlagBits queue_type, std::span<const QueueSemaphoreWait> wait_semaphores,
                std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos) const;

    ///
    void reset_fence() const;

    ///
    VkResult status() const;

    [[nodiscard]] VkFence submission_fence() const {
        return m_wait_fence->fence();
    }

    [[nodiscard]] bool was_submitted() const {
        return m_has_been_submitted;
    }

    /// Block until this command buffer's submission fence is signaled.
    void wait_fence() const;

    void reset() const;

public:
    /// Default constructor
    /// @param device A const reference to the device wrapper class
    /// @param cmd_pool The command pool from which the command buffer will be allocated
    /// @param name The internal debug marker name of the command buffer (must not be empty)
    CommandBuffer(const core::Device &device, VkCommandPool cmd_pool, std::string name,
                  VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) noexcept;

    ~CommandBuffer() = default;

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) = delete;

    [[nodiscard]] VkCommandBuffer command_buffer() const {
        return m_cmd_buf;
    }

    [[nodiscard]] const std::string &name() const {
        return m_name;
    }

    /// Begin recording this command buffer as a secondary command buffer.
    /// @param inheritance_info Inheritance info including dynamic-rendering inheritance chain.
    /// @param flags Recording usage flags.
    const CommandBuffer &
    begin_secondary_command_buffer(const VkCommandBufferInheritanceInfo &inheritance_info,
                                   VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT |
                                                                     VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) const;

    /// End recording this command buffer.
    const CommandBuffer &end_recording() const;

    /// Reset this command buffer before recording again.
    void reset_recording() const;

    /// Execute secondary command buffers from this (primary) command buffer.
    const CommandBuffer &
    execute_secondary_command_buffers(std::span<const VkCommandBuffer> secondary_cmd_buffers) const;

    /// Call vkCmdBeginDebugUtilsLabelEXT
    /// @param name The name of the debug label
    /// @param color The color of the debug label
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &begin_debug_label_region(const std::string &name, std::array<float, 4> color) const;

    /// Call vkCmdBindDescriptorSets to bind one single descriptor set
    /// @note Binding multiple descriptor sets would require implementing bind_descriptor_sets, which is not required
    /// for now.
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    /// @param descriptor_set The descriptor set to bind
    /// @param pipeline The graphics pipeline whose pipeline layout will be used
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_descriptor_set(VkDescriptorSet desc_set, std::weak_ptr<GraphicsPipeline> pipeline) const;

    const CommandBuffer &bind_descriptor_set(std::weak_ptr<descriptors::PerFrameDescriptorSets> descriptor_sets) const;

    const CommandBuffer &bind_descriptor_set(std::weak_ptr<descriptors::PerFrameDescriptorSets> descriptor_sets,
                                             std::weak_ptr<GraphicsPipeline> pipeline) const;

    /// Call vkCmdBindDescriptorSets
    /// @param desc_sets The descriptor sets to bind
    /// @param layout The pipeline layout
    /// @param bind_point the pipeline bind point (``VK_PIPELINE_BIND_POINT_GRAPHICS`` by default)
    /// @param first_set The first descriptor set (``0`` by default)
    /// @param dyn_offsets The dynamic offset values (empty by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_descriptor_sets(std::span<const VkDescriptorSet> desc_sets, // NOLINT
                                              VkPipelineLayout layout,
                                              VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                              std::uint32_t first_set = 0,
                                              std::span<const std::uint32_t> dyn_offsets = {}) const;

    /// Call vkCmdBindIndexBuffer
    /// @param buffer The index buffer to bind
    /// @param index_type The index type to use (``VK_INDEX_TYPE_UINT32`` by default)
    /// @param offset The offset (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_index_buffer(std::weak_ptr<Buffer> buffer,
                                           VkIndexType index_type = VK_INDEX_TYPE_UINT32, // NOLINT
                                           VkDeviceSize offset = 0) const;

    /// Call vkCmdBindIndexBuffer
    /// @param buf The index buffer to bind
    /// @param index_type The index type to use (``VK_INDEX_TYPE_UINT32`` by default)
    /// @param offset The offset (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_index_buffer(VkBuffer buf, VkIndexType index_type = VK_INDEX_TYPE_UINT32, // NOLINT
                                           VkDeviceSize offset = 0) const;

    /// Call vkCmdBindPipeline
    /// @param graphics_pipeline
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_pipeline(std::weak_ptr<GraphicsPipeline> graphics_pipeline) const;

    /// Call vkCmdBindPipeline
    /// @param pipeline The graphics pipeline to bind
    /// @param bind_point The pipeline bind point (``VK_PIPELINE_BIND_POINT_GRAPHICS`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_pipeline(VkPipeline pipeline, // NOLINT
                                       VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

    const CommandBuffer &bind_vertex_buffer(const std::weak_ptr<Buffer> buffer) const {
        const auto buffer_ref = buffer.lock();
        if (!buffer_ref) {
            throw InexorException("Error: Parameter 'buffer' is an invalid pointer!");
        }
        if (buffer_ref->type() != BufferType::VERTEX_BUFFER) {
            throw InexorException("Error: Rendergraph buffer resource " + buffer_ref->name() +
                                  " is not a vertex buffer!");
        }
        const auto vk_buffer = buffer_ref->buffer();
        const VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(m_cmd_buf, 0, 1, &vk_buffer, &offset);
        return *this;
    }

    /// Call vkCmdBindVertexBuffers
    /// @param bufs The vertex buffers to bind
    /// @param first_binding The first binding (``0`` by default)
    /// @param offsets The device offsets (empty by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_vertex_buffers(std::span<const VkBuffer> bufs, // NOLINT
                                             std::uint32_t first_binding = 0,
                                             std::span<const VkDeviceSize> offsets = {}) const;

    /// Call vkCmdPipelineBarrier
    /// @param image The image
    /// @param old_layout The old layout of the image
    /// @param new_layout The new layout of the image
    /// @note The new layout must be different from the old layout!
    /// @param subres_range The image subresource range
    /// @param src_mask The source pipeline stage flags (``VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`` by default)
    /// @param dst_mask The destination pipeline stage flags (``VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`` by default)
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer & // NOLINT
    change_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
                        VkImageSubresourceRange subres_range,
                        VkPipelineStageFlags src_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VkPipelineStageFlags dst_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) const;

    /// Call vkCmdPipelineBarrier
    /// @param image The image
    /// @param old_layout The old layout of the image
    /// @param new_layout The new layout of the image
    /// @param mip_level_count The number of mip levels (The parameter in ``VkImageSubresourceRange``)
    /// @param array_layer_count The number of array layers (The parameter in ``VkImageSubresourceRange``)
    /// @param base_mip_level The base mip level index (The parameter in ``VkImageSubresourceRange``)
    /// @param base_array_layer The base array layer index (The parameter in ``VkImageSubresourceRange``)
    /// @param src_mask The source pipeline stage flags (``VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`` by default)
    /// @param dst_mask The destination pipeline stage flags (``VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`` by default)
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer & // NOLINT
    change_image_layout(VkImage image, VkFormat format, VkImageLayout old_layout, VkImageLayout new_layout,
                        std::uint32_t mip_level_count = 1, std::uint32_t array_layer_count = 1,
                        std::uint32_t base_mip_level = 0, std::uint32_t base_array_layer = 0,
                        VkPipelineStageFlags src_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VkPipelineStageFlags dst_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) const;

    /// Call vkCmdCopyBuffer
    /// @param src_buf The source buffer
    /// @param dst_buf The destination buffer
    /// @param copy_region A single buffer copy region
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer(VkBuffer src_buf, VkBuffer dst_buf, // NOLINT
                                     const VkBufferCopy &copy_region) const;

    /// Call vkCmdCopyBuffer
    /// @param src_buf The source buffer
    /// @param dst_buf The destination buffer
    /// @param copy_regions A std::span of buffer copy regions
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer(VkBuffer src_buf, VkBuffer dst_buf, // NOLINT
                                     std::span<const VkBufferCopy> copy_regions) const;

    /// Call vkCmdCopyBuffer
    /// @param src_buf The source buffer
    /// @param dst_buf The destination buffer
    /// @param src_buf_size The size of the source buffer
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer(VkBuffer src_buf, VkBuffer dst_buf, // NOLINT
                                     VkDeviceSize src_buf_size) const;

    /// Call vkCmdCopyBufferToImage
    /// @param src_buf The source buffer
    /// @param dst_img The destination image
    /// @note The destination image is always expected to be in layout ``VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL``
    /// @param copy_regions A std::span of buffer image copy regions
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer_to_image(VkBuffer src_buf, VkImage dst_img, // NOLINT
                                              std::span<const VkBufferImageCopy> copy_regions) const;

    /// Call vkCmdCopyBufferToImage
    /// copy region
    /// @param src_buf The source buffer
    /// @param dst_img The destination image
    /// @note The destination image is always expected to be in layout ``VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL``
    /// @param copy_region The buffer image copy region
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer_to_image(VkBuffer src_buf, VkImage dst_img, // NOLINT
                                              const VkBufferImageCopy &copy_region) const;

    ///
    /// @param buffer
    /// @param img
    /// @param extent
    /// @return
    const CommandBuffer &copy_buffer_to_image(VkBuffer buffer, VkImage img, VkExtent3D extent) const;

    /// Call vkCmdCopyBuffer
    /// @param data A raw pointer to the data to copy
    /// @param data_size The size of the data to copy
    /// @param dst_img The destination image (must not be ``VK_NULL_HANDLE``)
    /// @note The destination image is always expected to be in layout ``VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`` for the
    /// copy operation
    /// @param name The internal name of the staging buffer (must not be empty)
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer_to_image(const void *data, const VkDeviceSize data_size, // NOLINT
                                              VkImage dst_img, const VkBufferImageCopy &copy_region,
                                              const std::string &name) const;

    /// Call vkCmdCopyBufferToImage
    /// @param src_buffer The source buffer
    /// @param img The image to copy the buffer into
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer_to_image(VkBuffer src_buffer, std::weak_ptr<Image> img) const;

    /// Call vkCmdDraw
    /// @param vert_count The number of vertices to draw
    /// @param inst_count The number of instances (``1`` by default)
    /// @param first_vert The index of the first vertex (``0`` by default)
    /// @param first_inst The instance ID of the first instance to draw (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &draw(std::uint32_t vert_count, std::uint32_t inst_count = 1, // NOLINT
                              std::uint32_t first_vert = 0, std::uint32_t first_inst = 0) const;

    /// Call vkCmdDrawIndexed
    /// @param index_count The number of vertices to draw
    /// @param inst_count The number of instances to draw (``1`` by defaul)
    /// @param first_index The base index withing the index buffer (``0`` by default)
    /// @param vert_offset The value added to the vertex index before indexing into the vertex buffer (``0`` by default)
    /// @param first_inst The instance ID of the first instance to draw (``0`` by default)
    /// @param index_count The number of indices to draw
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &draw_indexed(std::uint32_t index_count, std::uint32_t inst_count = 1, // NOLINT
                                      std::uint32_t first_index = 0, std::int32_t vert_offset = 0,
                                      std::uint32_t first_inst = 0) const;

    /// Call vkCmdBeginRendering
    /// @note We don't need to call it ``vkCmdBeginRenderingKHR`` anymore since it's part of Vulkan 1.3's core
    /// @note ``begin_render_pass`` has been deprecated because of dynamic rendering (``VK_KHR_dynamic_rendering``)
    /// @param rendering_info The info for dynamic rendering
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &begin_rendering(const VkRenderingInfo &rendering_info) const;

    /// Call vkCmdEndDebugUtilsLabelEXT
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_debug_label_region() const;

    // TODO: Make begin_rendering and end_rendering private and allow only rendergraph to access it!

    /// Call vkCmdEndRendering
    /// @note We don't need to call it ``vkCmdEndRenderingKHR`` anymore since it's part of Vulkan 1.3's core
    /// @note ``end_render_pass`` has been deprecated because of dynamic rendering (``VK_KHR_dynamic_rendering``)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_rendering() const;

    /// Call vkCmdPipelineBarrier2
    /// @param dependency_info Fully specified dependency info
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_barrier(const VkDependencyInfo &dependency_info) const;

    /// Call vkCmdPipelineBarrier2 with one buffer memory barrier
    /// @param buffer_mem_barrier The buffer memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_buffer_memory_barrier(const VkBufferMemoryBarrier2 &buffer_mem_barrier) const;

    /// Call vkCmdPipelineBarrier2 with one image memory barrier
    /// @param barrier The image memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_image_memory_barrier(const VkImageMemoryBarrier2 &barrier) const;

    /// Call vkCmdPipelineBarrier2 with one memory barrier
    /// @param barrier The memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_memory_barrier(const VkMemoryBarrier2 &barrier) const;

    /// Call vkCmdPipelineBarrier2 for a transfer-write -> shader-read dependency.
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &barrier_transfer_write_to_shader_read() const;

    /// Call vkCmdPipelineBarrier2 for a color-attachment-write -> shader-read dependency.
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &barrier_color_attachment_write_to_shader_read() const;

    /// Call vkCmdPipelineBarrier2 for a depth-stencil-attachment-write -> shader-read dependency.
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &barrier_depth_stencil_write_to_shader_read() const;

    /// Call vkCmdInsertDebugUtilsLabelEXT
    /// @param name The name of the debug label to insert
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &insert_debug_label(const std::string &name, std::array<float, 4> color) const;

    /// Call vkCmdPushConstants
    /// @param layout The pipeline layout
    /// @param stage The shader stage that will be accepting the push constants
    /// @param size The size of the push constant data in bytes
    /// @param data A pointer to the push constant data
    /// @param offset The offset value (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &push_constants(VkPipelineLayout layout, VkShaderStageFlags stage, // NOLINT
                                        std::uint32_t size, const void *data, VkDeviceSize offset = 0) const;

    /// Call vkCmdPushConstants
    /// @tparam T the data type of the push constant
    /// @param layout The pipeline layout
    /// @param data A const reference to the data
    /// @param stage The shader stage that will be accepting the push constants
    /// @param offset The offset value (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    template <typename T>
    const CommandBuffer &push_constant(const VkPipelineLayout layout, const T &data, // NOLINT
                                       const VkShaderStageFlags stage, const VkDeviceSize offset = 0) const {
        return push_constants(layout, stage, sizeof(data), &data, offset);
    }

    /// Call vkCmdPushConstants
    /// @tparam T the data type of the push constant
    /// @param pipeline The graphics pipeline
    /// @param data A const reference to the data
    /// @param stage The shader stage that will be accepting the push constants
    /// @param offset The offset value (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    template <typename T>
    const CommandBuffer &push_constant(const std::weak_ptr<wrapper::pipelines::GraphicsPipeline> pipeline,
                                       const T &data, // NOLINT
                                       const VkShaderStageFlags stage, const VkDeviceSize offset = 0) const {
        const auto pipeline_ref = pipeline.lock();
        if (!pipeline_ref) {
            throw InexorException("Error: Parameter 'pipeline' is an invalid pointer!");
        }
        return push_constants(pipeline_ref->pipeline_layout(), stage, sizeof(data), &data, offset);
    }

    /// @brief
    /// @param scissor
    /// @return
    const CommandBuffer &set_scissor(VkRect2D scissor) const;

    /// Set the viewport
    /// @param viewport The viewport
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &set_viewport(VkViewport viewport) const;
};

} // namespace inexor::vulkan_renderer::wrapper::commands
