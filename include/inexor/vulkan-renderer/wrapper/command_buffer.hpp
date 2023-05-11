#pragma once

#include <volk.h>

#include "inexor/vulkan-renderer/wrapper/fence.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_memory_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/semaphore.hpp"

#include <cstdint>
#include <memory>
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
    std::unique_ptr<Fence> m_wait_fence;

    // The Device wrapper must be able to call begin_command_buffer and end_command_buffer
    friend class Device;

    /// The staging buffers which are maybe used in the command buffer
    /// This vector of staging buffers will be cleared every time ``begin_command_buffer`` is called
    /// @note We are not recycling staging buffers. Once they are used and the command buffer handle has reached the end
    /// of its lifetime, the staging bufers will be cleared. We trust Vulkan Memory Allocator (VMA) in managing the
    /// memory for staging buffers.
    mutable std::vector<GPUMemoryBuffer> m_staging_bufs;

    friend class CommandPool;

    /// Call vkBeginCommandBuffer
    /// @param flags The command buffer usage flags, ``VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`` by default
    const CommandBuffer & // NOLINT
    begin_command_buffer(VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) const;

    /// Create a new staging buffer which will be stored internally for a copy operation
    /// @param data A raw pointer to the data to copy (must not be ``nullptr``)
    /// @param data_size The size of the data to copy (must be greater than ``0``)
    /// @param name The internal name of the staging buffer (must not be empty)
    /// @return A VkBuffer which contains the staging buffer data
    [[nodiscard]] VkBuffer create_staging_buffer(const void *data, const VkDeviceSize data_size,
                                                 const std::string &name) const {
        assert(data);
        assert(data_size > 0);
        assert(!name.empty());

        // Create a staging buffer for the copy operation and keep it until the CommandBuffer exceeds its lifetime
        m_staging_bufs.emplace_back(m_device, name, data_size, data, data_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VMA_MEMORY_USAGE_CPU_ONLY);

        return m_staging_bufs.back().buffer();
    }

    /// Create a new staging buffer which will be stored internally for a copy operation
    /// @tparam The data type of the staging buffer
    /// @param data A std::span of the source data
    /// @param name The internal name of the staging buffer (must not be empty)
    /// @return The staging buffer's VkBuffer
    template <typename DataType>
    [[nodiscard]] VkBuffer create_staging_buffer(const std::span<const DataType> data, const std::string &name) const {
        return create_staging_buffer(data.data(), static_cast<VkDeviceSize>(sizeof(data) * data.size()), name);
    }

    /// Call vkEndCommandBuffer
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_command_buffer() const; // NOLINT

public:
    /// Default constructor
    /// @param device A const reference to the device wrapper class
    /// @param cmd_pool The command pool from which the command buffer will be allocated
    /// @param name The internal debug marker name of the command buffer (must not be empty)
    CommandBuffer(const Device &device, VkCommandPool cmd_pool, std::string name);

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) noexcept;

    ~CommandBuffer() = default;

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) = delete;

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

    /// Call vkCmdCopyBuffer
    /// @param data A std::span of the source data
    /// @note A staging buffer for the copy operation will be created automatically from ``data``
    /// @param dst_img The destination image (must not be ``VK_NULL_HANDLE``)
    /// @note The destination image is always expected to be in layout ``VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`` for the
    /// copy operation
    /// @param name The internal name of the staging buffer (must not be empty)
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    template <typename DataType>
    const CommandBuffer &copy_buffer_to_image(const std::span<const DataType> data, // NOLINT
                                              VkImage dst_img, const VkBufferImageCopy &copy_region,
                                              const std::string &name) const {
        return copy_buffer_to_image(create_staging_buffer<DataType>(data, name), dst_img,
                                    static_cast<VkDeviceSize>(sizeof(data) * data.size()), copy_region, name);
    }

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

    /// Call vkCmdEndRenderPass
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_render_pass() const; // NOLINT

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
    const CommandBuffer &full_barrier() const;

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

    // Graphics commands
    // TODO(): Switch to taking in OOP wrappers when we have them (e.g. bind_vertex_buffers takes in a VertexBuffer)

    [[nodiscard]] VkCommandBuffer get() const {
        return m_command_buffer;
    }

    [[nodiscard]] const Fence &get_wait_fence() const {
        return *m_wait_fence;
    }

    [[nodiscard]] const VkCommandBuffer *ptr() const {
        return &m_command_buffer;
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

} // namespace inexor::vulkan_renderer::wrapper
