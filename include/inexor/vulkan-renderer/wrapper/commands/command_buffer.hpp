#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/synchronization/fence.hpp"

#include <array>
#include <cassert>
#include <memory>
#include <span>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::pipelines {
// Forward declaration
class GraphicsPipeline;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class Buffer;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::commands {

/// RAII wrapper class for VkCommandBuffer
/// @todo Make trivially copyable (this class doesn't really "own" the command buffer, more just an OOP wrapper).
class CommandBuffer {
    VkCommandBuffer m_cmd_buf{VK_NULL_HANDLE};
    const Device &m_device;
    std::string m_name;
    std::unique_ptr<synchronization::Fence> m_wait_fence;

    // The Device wrapper must be able to call begin_command_buffer and end_command_buffer
    friend class Device;

    /// The staging buffers which are maybe used in the command buffer
    /// This vector of staging buffers will be cleared every time ``begin_command_buffer`` is called
    /// @note We are not recycling staging buffers. Once they are used and the command buffer handle has reached the end
    /// of its lifetime, the staging bufers will be cleared. We trust Vulkan Memory Allocator (VMA) in managing the
    /// memory for staging buffers.
    mutable std::vector<render_graph::Buffer> m_staging_bufs;

    friend class CommandPool;

    /// Call vkBeginCommandBuffer
    /// @param flags The command buffer usage flags, ``VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`` by default
    const CommandBuffer & // NOLINT
    begin_command_buffer(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) const;

    // TODO: Implement create_staging_buffer()!

    /// Call vkEndCommandBuffer
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_command_buffer() const; // NOLINT

public:
    /// Default constructor
    /// @param device A const reference to the device wrapper class
    /// @param cmd_pool The command pool from which the command buffer will be allocated
    /// @param name The internal debug marker name of the command buffer (must not be empty)
    CommandBuffer(const wrapper::Device &device, VkCommandPool cmd_pool, std::string name);

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) noexcept;

    /// @note Command buffers are allocated from a command pool, meaning the memory required for this will be
    /// freed if the corresponding command pool is destroyed. Command buffers are not freed in the Destructor.
    ~CommandBuffer() = default;

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) = delete;

    /// Call vkCmdBeginDebugUtilsLabelEXT
    /// @param name The name of the debug label
    /// @param color The color of the debug label
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &begin_debug_label_region(std::string name, std::array<float, 4> color) const;

    /// Call vkCmdBeginRendering
    /// @note We don't need to call it ``vkCmdBeginRenderingKHR`` anymore since it's part of Vulkan 1.3's core
    /// @note ``begin_render_pass`` has been deprecated because of dynamic rendering (``VK_KHR_dynamic_rendering``)
    /// @param rendering_info The info for dynamic rendering
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &begin_rendering(const VkRenderingInfo *rendering_info) const;

    /// Call vkCmdBindDescriptorSets
    /// @param desc_sets The descriptor set to bind
    /// @param layout The pipeline layout
    /// @param bind_point the pipeline bind point (``VK_PIPELINE_BIND_POINT_GRAPHICS`` by default)
    /// @param first_set The first descriptor set (``0`` by default)
    /// @param dyn_offsets The dynamic offset values (empty by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_descriptor_set(VkDescriptorSet desc_set,
                                             VkPipelineLayout layout,
                                             VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                             std::uint32_t first_set = 0,
                                             std::span<const std::uint32_t> dyn_offsets = {}) const;

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
    /// @param buf The index buffer to bind
    /// @param index_type The index type to use (``VK_INDEX_TYPE_UINT32`` by default)
    /// @param offset The offset (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_index_buffer(std::shared_ptr<render_graph::Buffer> buf,
                                           VkIndexType index_type = VK_INDEX_TYPE_UINT32, // NOLINT
                                           VkDeviceSize offset = 0) const;

    /// Call vkCmdBindPipeline
    /// @param pipeline The graphics pipeline to bind
    /// @param bind_point The pipeline bind point (``VK_PIPELINE_BIND_POINT_GRAPHICS`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_pipeline(std::weak_ptr<pipelines::GraphicsPipeline> pipeline, // NOLINT
                                       VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

    /// Call vkCmdBindVertexBuffers
    /// @note When binding only a single vertex buffer, the parameters ``firstBinding`` and ``bindingCount`` in
    /// ``pOffsets`` in ``vkCmdBindVertexBuffers`` do not need to be exposed.
    /// @param buf The vertex buffer to bind
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_vertex_buffer(std::shared_ptr<render_graph::Buffer> buf) const;

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
    change_image_layout(VkImage image,
                        VkImageLayout old_layout,
                        VkImageLayout new_layout,
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
    change_image_layout(VkImage image,
                        VkImageLayout old_layout,
                        VkImageLayout new_layout,
                        std::uint32_t mip_level_count = 1,
                        std::uint32_t array_layer_count = 1,
                        std::uint32_t base_mip_level = 0,
                        std::uint32_t base_array_layer = 0,
                        VkPipelineStageFlags src_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                        VkPipelineStageFlags dst_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT) const;

    /// Call vkCmdCopyBuffer
    /// @param src_buf The source buffer
    /// @param dst_buf The destination buffer
    /// @param copy_region A single buffer copy region
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer(VkBuffer src_buf,
                                     VkBuffer dst_buf, // NOLINT
                                     const VkBufferCopy &copy_region) const;

    /// Call vkCmdCopyBuffer
    /// @param src_buf The source buffer
    /// @param dst_buf The destination buffer
    /// @param copy_regions A std::span of buffer copy regions
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer(VkBuffer src_buf,
                                     VkBuffer dst_buf, // NOLINT
                                     std::span<const VkBufferCopy> copy_regions) const;

    /// Call vkCmdCopyBuffer
    /// @param src_buf The source buffer
    /// @param dst_buf The destination buffer
    /// @param src_buf_size The size of the source buffer
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer(VkBuffer src_buf,
                                     VkBuffer dst_buf, // NOLINT
                                     VkDeviceSize src_buf_size) const;

    /// Call vkCmdCopyBufferToImage
    /// @param src_buf The source buffer
    /// @param dst_img The destination image
    /// @note The destination image is always expected to be in layout ``VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL``
    /// @param copy_region The buffer image copy region
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer_to_image(VkBuffer src_buf,
                                              VkImage dst_img, // NOLINT
                                              const VkBufferImageCopy &copy_region) const;

    /// Call vkCmdCopyBufferToImage
    /// @param src_buffer The source buffer
    /// @param dst_img The image to copy the buffer into
    /// @param extent The extent of the image
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer_to_image(VkBuffer src_buffer, VkImage dst_img, VkExtent3D extent) const;

    /// Call vkCmdDraw
    /// @param vert_count The number of vertices to draw
    /// @param inst_count The number of instances (``1`` by default)
    /// @param first_vert The index of the first vertex (``0`` by default)
    /// @param first_inst The instance ID of the first instance to draw (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &draw(std::uint32_t vert_count,
                              std::uint32_t inst_count = 1, // NOLINT
                              std::uint32_t first_vert = 0,
                              std::uint32_t first_inst = 0) const;

    /// Call vkCmdDrawIndexed
    /// @param index_count The number of vertices to draw
    /// @param inst_count The number of instances to draw (``1`` by defaul)
    /// @param first_index The base index withing the index buffer (``0`` by default)
    /// @param vert_offset The value added to the vertex index before indexing into the vertex buffer (``0`` by default)
    /// @param first_inst The instance ID of the first instance to draw (``0`` by default)
    /// @param index_count The number of indices to draw
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &draw_indexed(std::uint32_t index_count,
                                      std::uint32_t inst_count = 1, // NOLINT
                                      std::uint32_t first_index = 0,
                                      std::int32_t vert_offset = 0,
                                      std::uint32_t first_inst = 0) const;

    /// Call vkCmdEndDebugUtilsLabelEXT
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_debug_label_region() const;

    /// Call vkCmdEndRendering
    /// @note We don't need to call it ``vkCmdEndRenderingKHR`` anymore since it's part of Vulkan 1.3's core
    /// @note ``end_render_pass`` has been deprecated because of dynamic rendering (``VK_KHR_dynamic_rendering``)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_rendering() const;

    [[nodiscard]] VkResult fence_status() const {
        return m_wait_fence->status();
    }

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param img_mem_barriers The image memory barriers
    /// @note We start with image memory barriers as no-default parameter, since it's the most common use case
    /// @param mem_barriers The memory barriers (empty by default)
    /// @param buf_mem_barriers The buffer memory barriers (empty by default)
    /// @param dep_flags The dependency flags (``0`` by default)
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_barrier(VkPipelineStageFlags src_stage_flags, // NOLINT
                                          VkPipelineStageFlags dst_stage_flags,
                                          std::span<const VkImageMemoryBarrier> img_mem_barriers,
                                          std::span<const VkMemoryBarrier> mem_barriers = {},
                                          std::span<const VkBufferMemoryBarrier> buf_mem_barriers = {},
                                          VkDependencyFlags dep_flags = 0) const;

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param buffer_mem_barrier The buffer memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_buffer_memory_barrier(VkPipelineStageFlags src_stage_flags,
                                                        VkPipelineStageFlags dst_stage_flags,
                                                        const VkBufferMemoryBarrier &buffer_mem_barrier) const;

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param src_access_flags The source access flags
    /// @param dst_access_flags The destination access flags
    /// @param buffer
    /// @param size
    /// @param offset
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_buffer_memory_barrier(VkPipelineStageFlags src_stage_flags,
                                                        VkPipelineStageFlags dst_stage_flags,
                                                        VkAccessFlags src_access_flags,
                                                        VkAccessFlags dst_access_flags,
                                                        VkBuffer buffer,
                                                        VkDeviceSize size = VK_WHOLE_SIZE,
                                                        VkDeviceSize offset = 0) const;

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param buffer_mem_barriers The buffer memory barriers
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &
    pipeline_buffer_memory_barriers(VkPipelineStageFlags src_stage_flags,
                                    VkPipelineStageFlags dst_stage_flags,
                                    std::span<const VkBufferMemoryBarrier> buffer_mem_barriers) const;

    /// Place a buffer memory pipeline barrier before a vkCmdCopyBuffer command
    /// @param buffer The affected buffer
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_buffer_memory_barrier_before_copy_buffer(VkBuffer buffer) const;

    /// Place a buffer memory pipeline barrier after a vkCmdCopyBuffer command
    /// @param buffer The affected buffer
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_buffer_memory_barrier_after_copy_buffer(VkBuffer buffer) const;

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param barrier The image memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_image_memory_barrier(VkPipelineStageFlags src_stage_flags, // NOLINT
                                                       VkPipelineStageFlags dst_stage_flags,
                                                       const VkImageMemoryBarrier &barrier) const;

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags
    /// @param dst_stage_flags
    /// @param src_access_flags
    /// @param dst_access_flags
    /// @param old_img_layout
    /// @param new_img_layout
    /// @param img
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_image_memory_barrier(VkPipelineStageFlags src_stage_flags,
                                                       VkPipelineStageFlags dst_stage_flags,
                                                       VkAccessFlags src_access_flags,
                                                       VkAccessFlags dst_access_flags,
                                                       VkImageLayout old_img_layout,
                                                       VkImageLayout new_img_layout,
                                                       VkImage img) const;
    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param barriers The image memory barriers
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_image_memory_barriers(VkPipelineStageFlags src_stage_flags, // NOLINT
                                                        VkPipelineStageFlags dst_stage_flags,
                                                        std::span<const VkImageMemoryBarrier> barriers) const;

    ///
    /// @param img
    /// @return
    const CommandBuffer &pipeline_image_memory_barrier_after_copy_buffer_to_image(VkImage img) const;

    ///
    /// @param img
    /// @return
    const CommandBuffer &pipeline_image_memory_barrier_before_copy_buffer_to_image(VkImage img) const;

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param barrier The memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_memory_barrier(VkPipelineStageFlags src_stage_flags, // NOLINT
                                                 VkPipelineStageFlags dst_stage_flags,
                                                 const VkMemoryBarrier &barrier) const;

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param barriers The memory barriers
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_memory_barriers(VkPipelineStageFlags src_stage_flags, // NOLINT
                                                  VkPipelineStageFlags dst_stage_flags,
                                                  const std::span<const VkMemoryBarrier> barriers) const;

    /// Call vkCmdPipelineBarrier to place a full memory barrier
    /// @warning You should avoid full barriers since they are not the most performant solution in most cases
    const CommandBuffer &full_barrier() const;

    /// Call vkCmdInsertDebugUtilsLabelEXT
    /// @param name The name of the debug label to insert
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &insert_debug_label(std::string name, std::array<float, 4> color) const;

    /// Call vkCmdPushConstants
    /// @param layout The pipeline layout
    /// @param stage The shader stage that will be accepting the push constants
    /// @param size The size of the push constant data in bytes
    /// @param data A pointer to the push constant data
    /// @param offset The offset value (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &push_constants(VkPipelineLayout layout,
                                        VkShaderStageFlags stage, // NOLINT
                                        std::uint32_t size,
                                        const void *data,
                                        VkDeviceSize offset = 0) const;

    /// Call vkCmdPushConstants
    /// @tparam T the data type of the push constant
    /// @param layout The pipeline layout
    /// @param data A const reference to the data
    /// @param stage The shader stage that will be accepting the push constants
    /// @param offset The offset value (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    template <typename T>
    const CommandBuffer &push_constant(const VkPipelineLayout layout,
                                       const T &data, // NOLINT
                                       const VkShaderStageFlags stage,
                                       const VkDeviceSize offset = 0) const {
        return push_constants(layout, stage, sizeof(data), &data, offset);
    }

    // Graphics commands
    // TODO(): Switch to taking in OOP wrappers when we have them (e.g. bind_vertex_buffers takes in a VertexBuffer)

    [[nodiscard]] VkCommandBuffer get() const {
        return m_cmd_buf;
    }

    [[nodiscard]] const synchronization::Fence &get_wait_fence() const {
        return *m_wait_fence;
    }

    [[nodiscard]] const VkCommandBuffer *ptr() const {
        return &m_cmd_buf;
    }

    /// Call the reset method of the Fence member
    const CommandBuffer &reset_fence() const;

    /// Call vkQueueSubmit
    /// @param submit_infos The submit infos
    const CommandBuffer &submit(std::span<const VkSubmitInfo> submit_infos) const; // NOLINT

    /// Call vkQueueSubmit
    /// @param submit_info The submit info
    const CommandBuffer &submit(VkSubmitInfo submit_infos) const; // NOLINT

    /// Call vkQueueSubmit
    const CommandBuffer &submit() const; // NOLINT

    /// Call vkQueueSubmit and use a fence to wait for command buffer submission and execution to complete
    /// @param submit_infos The submit infos
    const CommandBuffer &submit_and_wait(std::span<const VkSubmitInfo> submit_infos) const; // NOLINT

    /// Call vkQueueSubmit and use a fence to wait for command buffer submission and execution to complete
    /// @param submit_info The submit info
    const CommandBuffer &submit_and_wait(VkSubmitInfo submit_info) const; // NOLINT

    /// Call vkQueueSubmit and use a fence to wait for command buffer submission and execution to complete
    const CommandBuffer &submit_and_wait() const; // NOLINT
};

} // namespace inexor::vulkan_renderer::wrapper::commands
