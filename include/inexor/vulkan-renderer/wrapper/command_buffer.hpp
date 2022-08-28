#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for VkCommandBuffer.
/// @todo Make trivially copyable (this class doesn't really "own" the command buffer, more just an OOP wrapper).
class CommandBuffer {
    VkCommandBuffer m_command_buffer{VK_NULL_HANDLE};
    const wrapper::Device &m_device;
    std::string m_name;

public:
    /// @brief Default constructor.
    /// @param device The const reference to the device RAII wrapper class.
    /// @param command_pool The command pool from which the command buffer will be allocated.
    /// @param name The internal debug marker name of the command buffer. This must not be an empty string.
    CommandBuffer(const wrapper::Device &device, VkCommandPool command_pool, std::string name);

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) noexcept;

    ~CommandBuffer() = default;

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) = delete;

    /// Call vkBeginCommandBuffer
    /// @param flags The command buffer usage flags, 0 by default
    const CommandBuffer &begin_command_buffer(VkCommandBufferUsageFlags flags = 0) const; // NOLINT

    /// Call vkCmdBeginRenderPass
    /// @param render_pass_bi The renderpass begin info
    /// @param subpass_contents The subpass contents (``VK_SUBPASS_CONTENTS_INLINE`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi, // NOLINT
                                           VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) const;

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
    const CommandBuffer &bind_index_buffer(VkBuffer buf, VkIndexType index_type = VK_INDEX_TYPE_UINT32, // NOLINT
                                           VkDeviceSize offset = 0) const;

    /// Call vkCmdBindPipeline
    /// @param pipeline The graphics pipeline to bind
    /// @param bind_point The pipeline bind point (``VK_PIPELINE_BIND_POINT_GRAPHICS`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_pipeline(VkPipeline pipeline, // NOLINT
                                       VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

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
    change_image_layout(VkImage image, VkImageLayout old_layout, VkImageLayout new_layout,
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
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param barrier The image memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_image_memory_barrier(VkPipelineStageFlags src_stage_flags, // NOLINT
                                                       VkPipelineStageFlags dst_stage_flags,
                                                       const VkImageMemoryBarrier &barrier) const;

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param barrier The memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_memory_barrier(VkPipelineStageFlags src_stage_flags, // NOLINT
                                                 VkPipelineStageFlags dst_stage_flags,
                                                 const VkMemoryBarrier &barrier) const;

    /// Call vkCmdPipelineBarrier to place a full memory barrier
    /// @warning You should avoid full barriers since they are not the most performant solution in most cases
    const CommandBuffer &pipeline_full_memory_barrier() const;

    /// @brief Update push constant data.
    /// @param layout The pipeline layout
    /// @param stage The shader stage that will be accepting the push constants
    /// @param size The size of the push constant data in bytes
    /// @param data A pointer to the push constant data
    void push_constants(VkPipelineLayout layout, VkShaderStageFlags stage, std::uint32_t size, void *data) const;

    /// @brief Call vkEndCommandBuffer.
    void end() const;

    // Graphics commands
    // TODO(): Switch to taking in OOP wrappers when we have them (e.g. bind_vertex_buffers takes in a VertexBuffer)

    /// @brief Call vkCmdDraw.
    /// @param vertex_count The number of vertices to draw.
    void draw(std::size_t vertex_count) const;

    /// @brief Call vkCmdDrawIndexed.
    /// @param index_count The number of indices to draw.
    void draw_indexed(std::size_t index_count) const;

    /// @brief Call vkCmdEndRenderPass.
    void end_render_pass() const;

    [[nodiscard]] VkCommandBuffer get() const {
        return m_command_buffer;
    }

    [[nodiscard]] const VkCommandBuffer *ptr() const {
        return &m_command_buffer;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
