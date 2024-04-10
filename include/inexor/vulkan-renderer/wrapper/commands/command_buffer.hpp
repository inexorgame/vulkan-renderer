#pragma once

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/fence.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"

#include <array>
#include <cassert>
#include <memory>
#include <span>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::pipelines {
// Forward declaration
class GraphicsPipeline;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_graph {
// Forward declaration
class Buffer;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;

/// RAII wrapper class for VkCommandBuffer
/// @todo Make trivially copyable (this class doesn't really "own" the command buffer, more just an OOP wrapper).
class CommandBuffer {
    VkCommandBuffer m_cmd_buf{VK_NULL_HANDLE};
    const Device &m_device;
    std::string m_name;
    std::unique_ptr<Fence> m_wait_fence;

    // The Device wrapper must be able to call begin_command_buffer and end_command_buffer
    friend class Device;
    friend class CommandPool;

    /// The staging buffers which are maybe used in the command buffer
    /// This vector of staging buffers will be cleared every time ``begin_command_buffer`` is called
    /// @note We are not recycling staging buffers. Once they are used and the command buffer handle has reached the end
    /// of its lifetime, the staging bufers will be cleared. We trust Vulkan Memory Allocator (VMA) in managing the
    /// memory for staging buffers.
    mutable std::vector<render_graph::Buffer> m_staging_bufs;

    /// Call vkBeginCommandBuffer
    /// @param flags The command buffer usage flags (``VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT`` by default)
    const CommandBuffer &
    begin_command_buffer(const VkCommandBufferUsageFlags flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) const {
        const auto begin_info = make_info<VkCommandBufferBeginInfo>({
            .flags = flags,
        });
        vkBeginCommandBuffer(m_cmd_buf, &begin_info);
        // We must clear the staging buffers which could be left over from previous use of this command buffer
        m_staging_bufs.clear();
        return *this;
    }

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
        m_staging_bufs.emplace_back(m_device, data_size, data, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VMA_MEMORY_USAGE_CPU_ONLY, name);

        return m_staging_bufs.back().m_buffer;
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
    const CommandBuffer &end_command_buffer() const {
        vkEndCommandBuffer(m_cmd_buf);
        return *this;
    }

public:
    /// Default constructor
    /// @param device A const reference to the device wrapper class
    /// @param cmd_pool The command pool from which the command buffer will be allocated
    /// @param name The internal debug marker name of the command buffer (must not be empty)
    CommandBuffer(const Device &device, VkCommandPool cmd_pool, std::string name);

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) noexcept;

    /// @note Command buffers are allocated from a command pool, meaning the memory required for this will be
    /// freed if the corresponding command pool is destroyed. Command buffers are not freed in the Destructor.
    ~CommandBuffer() = default;

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) = delete;

    /// Call vkCmdBeginDebugUtilsLabelEXT
    /// @param name The name of the debug label
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &begin_debug_label_region(std::string name, float color[4]) const {
        auto label = make_info<VkDebugUtilsLabelEXT>({
            .pLabelName = name.c_str(),
        });
        // TODO: Fix me :(
        label.color[0] = color[0];
        label.color[1] = color[1];
        label.color[2] = color[2];
        label.color[3] = color[3];
        vkCmdBeginDebugUtilsLabelEXT(m_cmd_buf, &label);
        return *this;
    }

    /// Call vkCmdBeginRendering
    /// @note We don't need to call it ``vkCmdBeginRenderingKHR`` anymore since it's part of Vulkan 1.3's core
    /// @note ``begin_render_pass`` has been deprecated because of dynamic rendering (``VK_KHR_dynamic_rendering``)
    /// @param rendering_info The info for dynamic rendering
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &begin_rendering(const VkRenderingInfo *rendering_info) const {
        assert(rendering_info);
        vkCmdBeginRendering(m_cmd_buf, rendering_info);
        return *this;
    }

    /// Call vkCmdBindDescriptorSets
    /// @param desc_sets The descriptor sets to bind
    /// @param layout The pipeline layout
    /// @param bind_point the pipeline bind point (``VK_PIPELINE_BIND_POINT_GRAPHICS`` by default)
    /// @param first_set The first descriptor set (``0`` by default)
    /// @param dyn_offsets The dynamic offset values (empty by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_descriptor_sets(const std::span<const VkDescriptorSet> desc_sets,
                                              const VkPipelineLayout layout, const VkPipelineBindPoint bind_point,
                                              const std::uint32_t first_set,
                                              const std::span<const std::uint32_t> dyn_offsets) const {
        assert(layout);
        assert(!desc_sets.empty());
        vkCmdBindDescriptorSets(m_cmd_buf, bind_point, layout, first_set, static_cast<std::uint32_t>(desc_sets.size()),
                                desc_sets.data(), static_cast<std::uint32_t>(dyn_offsets.size()), dyn_offsets.data());
        return *this;
    }

    /// Call vkCmdBindDescriptorSets
    /// @param desc_sets The descriptor set to bind
    /// @param layout The pipeline layout
    /// @param bind_point the pipeline bind point (``VK_PIPELINE_BIND_POINT_GRAPHICS`` by default)
    /// @param first_set The first descriptor set (``0`` by default)
    /// @param dyn_offsets The dynamic offset values (empty by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_descriptor_set(const VkDescriptorSet descriptor_set, const VkPipelineLayout layout,
                                             const VkPipelineBindPoint bind_point, const std::uint32_t first_set,
                                             const std::span<const std::uint32_t> dyn_offsets) const {
        return bind_descriptor_sets({&descriptor_set, 1}, layout, bind_point, first_set, dyn_offsets);
    }

    /// Call vkCmdBindIndexBuffer
    /// @param index_buffer The index buffer to bind
    /// @param index_type The index type to use (``VK_INDEX_TYPE_UINT32`` by default)
    /// @param offset The offset (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_index_buffer(const VkBuffer buf, const VkIndexType index_type,
                                           const VkDeviceSize offset) const {
        vkCmdBindIndexBuffer(m_cmd_buf, buf, offset, index_type);
        return *this;
    }

    /// Call vkCmdBindPipeline
    /// @param pipeline The graphics pipeline to bind
    /// @param bind_point The pipeline bind point (``VK_PIPELINE_BIND_POINT_GRAPHICS`` by default)
    /// @return A const reference to the dereferenced this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_pipeline(const pipelines::GraphicsPipeline &pipeline,
                                       const VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const {
        // CommandBuffer is a friend class of GraphicsPipeline and is allowed to access m_pipeline
        vkCmdBindPipeline(m_cmd_buf, bind_point, pipeline.m_pipeline);
        return *this;
    }

    /// Call vkCmdBindVertexBuffers to bind one vertex buffer
    /// @param vertex_buffer The vertex buffer to bind
    /// @return A const reference to the dereferenced this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_vertex_buffer(std::weak_ptr<render_graph::Buffer> buffer) const {
        vkCmdBindVertexBuffers(m_cmd_buf, 0, 1, &buffer.lock()->m_buffer, 0);
        return *this;
    }

    /// Call vkCmdBindVertexBuffers to call multiple vertex buffers
    /// @param vertex_buffers The vertex buffer to bind
    /// @return A const reference to the dereferenced this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_vertex_buffers(const std::span<const VkBuffer> buffers) const {
        assert(!buffers.empty());
        vkCmdBindVertexBuffers(m_cmd_buf, 0, static_cast<std::uint32_t>(buffers.size()), buffers.data(), 0);
        return *this;
    }

    /// Call vkCmdPipelineBarrier
    /// @param image The image
    /// @param old_layout The old layout of the image
    /// @param new_layout The new layout of the image
    /// @note The new layout must be different from the old layout!
    /// @param subres_range The image subresource range
    /// @param src_mask The source pipeline stage flags (``VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`` by default)
    /// @param dst_mask The destination pipeline stage flags (``VK_PIPELINE_STAGE_ALL_COMMANDS_BIT`` by default)
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &change_image_layout(const VkImage image, const VkImageLayout old_layout,
                                             const VkImageLayout new_layout, const VkImageSubresourceRange subres_range,
                                             const VkPipelineStageFlags src_mask,
                                             const VkPipelineStageFlags dst_mask) const {
        assert(new_layout != old_layout);

        auto barrier = make_info<VkImageMemoryBarrier>({
            .oldLayout = old_layout,
            .newLayout = new_layout,
            .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
            .image = image,
            .subresourceRange = subres_range,
        });

        switch (old_layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            barrier.srcAccessMask = 0;
            break;
        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            break;
        }

        switch (new_layout) {
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;
        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            if (barrier.srcAccessMask == 0) {
                barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
            }
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;
        default:
            break;
        }

        return pipeline_image_memory_barrier(src_mask, dst_mask, barrier);
    }

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
    CommandBuffer &change_image_layout(const VkImage image, const VkImageLayout old_layout,
                                       const VkImageLayout new_layout, const std::uint32_t mip_level_count,
                                       const std::uint32_t array_layer_count, const std::uint32_t base_mip_level,
                                       const std::uint32_t base_array_layer, const VkPipelineStageFlags src_mask,
                                       const VkPipelineStageFlags dst_mask) const {
        return change_image_layout(image, old_layout, new_layout,
                                   {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                    .baseMipLevel = base_mip_level,
                                    .levelCount = mip_level_count,
                                    .baseArrayLayer = base_array_layer,
                                    .layerCount = array_layer_count},
                                   src_mask, dst_mask);
    }

    /// Call vkCmdCopyBuffer
    /// @param src_buf The source buffer
    /// @param dst_buf The destination buffer
    /// @param copy_region A single buffer copy region
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer(const VkBuffer src_buf, const VkBuffer dst_buf,
                                     const VkBufferCopy &copy_region) const {
        return copy_buffer(src_buf, dst_buf, {&copy_region, 1});
    }

    /// Call vkCmdCopyBuffer
    /// @param src_buf The source buffer
    /// @param dst_buf The destination buffer
    /// @param copy_regions A std::span of buffer copy regions
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer(const VkBuffer src_buf, const VkBuffer dst_buf,
                                     const std::span<const VkBufferCopy> copy_regions) const {
        assert(src_buf);
        assert(dst_buf);
        assert(!copy_regions.empty());
        vkCmdCopyBuffer(m_cmd_buf, src_buf, dst_buf, static_cast<std::uint32_t>(copy_regions.size()),
                        copy_regions.data());
        return *this;
    }

    /// Call vkCmdCopyBuffer
    /// @param src_buf The source buffer
    /// @param dst_buf The destination buffer
    /// @param src_buf_size The size of the source buffer
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer(const VkBuffer src_buf, const VkBuffer dst_buf,
                                     const VkDeviceSize src_buf_size) const {
        return copy_buffer(src_buf, dst_buf, {.size = src_buf_size});
    }

    /// Call vkCmdCopyBufferToImage
    /// @param src_buf The source buffer
    /// @param dst_img The destination image
    /// @note The destination image is always expected to be in layout ``VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL``
    /// @param copy_regions A std::span of buffer image copy regions
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer_to_image(const VkBuffer src_buf, const VkImage dst_img,
                                              const std::span<const VkBufferImageCopy> copy_regions) const {
        assert(src_buf);
        assert(dst_img);
        vkCmdCopyBufferToImage(m_cmd_buf, src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                               static_cast<std::uint32_t>(copy_regions.size()), copy_regions.data());
        return *this;
    }

    /// Call vkCmdCopyBufferToImage
    /// @param src_buf The source buffer
    /// @param dst_img The destination image
    /// @note The destination image is always expected to be in layout ``VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL``
    /// @param copy_region The buffer image copy region
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer_to_image(const VkBuffer src_buf, const VkImage dst_img,
                                              const VkBufferImageCopy &copy_region) const {
        assert(src_buf);
        assert(dst_img);
        vkCmdCopyBufferToImage(m_cmd_buf, src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
        return *this;
    }

    /// Call vkCmdCopyBufferToImage
    /// @param data A raw pointer to the data to copy
    /// @param data_size The size of the data to copy
    /// @param dst_img The destination image (must not be ``VK_NULL_HANDLE``)
    /// @note The destination image is always expected to be in layout ``VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL`` for the
    /// copy operation
    /// @param name The internal name of the staging buffer (must not be empty)
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &copy_buffer_to_image(const void *data,
                                              const VkDeviceSize data_size, // NOLINT
                                              const VkImage dst_img, const VkBufferImageCopy &copy_region,
                                              const std::string &name) const {
        return copy_buffer_to_image(create_staging_buffer(data, data_size, name), dst_img, copy_region);
    }

    /// Call vkCmdCopyBufferToImage
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
    const CommandBuffer &draw(const std::uint32_t vert_count, const std::uint32_t inst_count,
                              const std::uint32_t first_vert, const std::uint32_t first_inst) const {
        vkCmdDraw(m_cmd_buf, vert_count, inst_count, first_vert, first_inst);
        return *this;
    }

    /// Call vkCmdDrawIndexed
    /// @param index_count The number of vertices to draw
    /// @param inst_count The number of instances to draw (``1`` by defaul)
    /// @param first_index The base index withing the index buffer (``0`` by default)
    /// @param vert_offset The value added to the vertex index before indexing into the vertex buffer (``0`` by default)
    /// @param first_inst The instance ID of the first instance to draw (``0`` by default)
    /// @param index_count The number of indices to draw
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &draw_indexed(const std::uint32_t index_count, const std::uint32_t inst_count,
                                      const std::uint32_t first_index, const std::int32_t vert_offset,
                                      const std::uint32_t first_inst) const {
        vkCmdDrawIndexed(m_cmd_buf, index_count, inst_count, first_index, vert_offset, first_inst);
        return *this;
    }

    /// Call vkCmdEndDebugUtilsLabelEXT
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_debug_label_region() const {
        vkCmdEndDebugUtilsLabelEXT(m_cmd_buf);
        return *this;
    }

    /// Call vkCmdEndRendering
    /// @note We don't need to call it ``vkCmdEndRenderingKHR`` anymore since it's part of Vulkan 1.3's core
    /// @note ``end_render_pass`` has been deprecated because of dynamic rendering (``VK_KHR_dynamic_rendering``)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &end_rendering() const {
        vkCmdEndRendering(m_cmd_buf);
        return *this;
    }

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
    const CommandBuffer &pipeline_barrier(const VkPipelineStageFlags src_stage_flags,
                                          const VkPipelineStageFlags dst_stage_flags,
                                          const std::span<const VkImageMemoryBarrier> img_mem_barriers,
                                          const std::span<const VkMemoryBarrier> mem_barriers,
                                          const std::span<const VkBufferMemoryBarrier> buf_mem_barriers,
                                          const VkDependencyFlags dep_flags) const {
        vkCmdPipelineBarrier(m_cmd_buf, src_stage_flags, dst_stage_flags, dep_flags,
                             static_cast<std::uint32_t>(mem_barriers.size()), mem_barriers.data(),
                             static_cast<std::uint32_t>(buf_mem_barriers.size()), buf_mem_barriers.data(),
                             static_cast<std::uint32_t>(img_mem_barriers.size()), img_mem_barriers.data());
        return *this;
    }

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param barrier The image memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_image_memory_barrier(const VkPipelineStageFlags src_stage_flags,
                                                       const VkPipelineStageFlags dst_stage_flags,
                                                       const VkImageMemoryBarrier &img_barrier) const {
        return pipeline_barrier(src_stage_flags, dst_stage_flags, {&img_barrier, 1});
    }

    /// Call vkCmdPipelineBarrier
    /// @param src_stage_flags The the source stage flags
    /// @param dst_stage_flags The destination stage flags
    /// @param barrier The memory barrier
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &pipeline_memory_barrier(VkPipelineStageFlags src_stage_flags,
                                                 VkPipelineStageFlags dst_stage_flags,
                                                 const VkMemoryBarrier &mem_barrier) const {
        return pipeline_barrier(src_stage_flags, dst_stage_flags, {}, {&mem_barrier, 1});
    }

    /// Call vkCmdPipelineBarrier to place a full memory barrier
    /// @warning You should avoid full barriers since they are not the most performant solution in most cases
    const CommandBuffer &full_barrier() const {
        return pipeline_memory_barrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                       make_info<VkMemoryBarrier>({
                                           .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
                                           .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                                       }));
    }

    /// Call vkCmdInsertDebugUtilsLabelEXT
    /// @param name The name of the debug label to insert
    /// @return A const reference to the dereferenced ``this`` pointer (allowing for method calls to be chained)
    const CommandBuffer &insert_debug_label(std::string name, float color[4]) const {
        auto label = make_info<VkDebugUtilsLabelEXT>({
            .pLabelName = name.c_str(),
        });
        // TODO: Fix me :(
        label.color[0] = color[0];
        label.color[1] = color[1];
        label.color[2] = color[2];
        label.color[3] = color[3];

        vkCmdBeginDebugUtilsLabelEXT(m_cmd_buf, &label);
        return *this;
    }

    /// Call vkCmdPushConstants
    /// @param layout The pipeline layout
    /// @param stage The shader stage that will be accepting the push constants
    /// @param size The size of the push constant data in bytes
    /// @param data A pointer to the push constant data
    /// @param offset The offset value (``0`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    auto &push_constants(const VkPipelineLayout layout, const VkShaderStageFlags stage, const std::uint32_t size,
                         const void *data, const VkDeviceSize offset) const {
        assert(layout);
        assert(size > 0);
        assert(data);
        vkCmdPushConstants(m_cmd_buf, layout, stage, static_cast<std::uint32_t>(offset), size, data);
        return *this;
    }

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
        return m_cmd_buf;
    }

    [[nodiscard]] const Fence &get_wait_fence() const {
        return *m_wait_fence;
    }

    // TODO: Remove get method and resolve via friend class?
    [[nodiscard]] const VkCommandBuffer *ptr() const {
        return &m_cmd_buf;
    }

    /// Call the reset method of the Fence member
    const CommandBuffer &reset_fence() const {
        m_wait_fence->reset();
        return *this;
    }

    /// Call vkQueueSubmit
    /// @param submit_infos The submit infos
    const CommandBuffer &submit(const std::span<const VkSubmitInfo> submit_infos) const {
        assert(!submit_infos.empty());
        end_command_buffer();

        if (const auto result =
                vkQueueSubmit(m_device.graphics_queue(), static_cast<std::uint32_t>(submit_infos.size()),
                              submit_infos.data(), m_wait_fence->get())) {
            throw VulkanException("Error: vkQueueSubmit failed!", result);
        }
        return *this;
    }

    /// Call vkQueueSubmit
    /// @param submit_info The submit info
    const CommandBuffer &submit(const VkSubmitInfo submit_info) const {
        return submit({&submit_info, 1});
    }

    /// Call vkQueueSubmit
    const CommandBuffer &submit() const {
        return submit(make_info<VkSubmitInfo>({
            .commandBufferCount = 1,
            .pCommandBuffers = &m_cmd_buf,
        }));
    }

    /// Call vkQueueSubmit and use a fence to wait for command buffer submission and execution to complete
    /// @param submit_infos The submit infos
    const CommandBuffer &submit_and_wait(const std::span<const VkSubmitInfo> submit_infos) const {
        submit(submit_infos);
        m_wait_fence->block();
        return *this;
    }

    /// Call vkQueueSubmit and use a fence to wait for command buffer submission and execution to complete
    /// @param submit_info The submit info
    const CommandBuffer &submit_and_wait(const VkSubmitInfo submit_info) const {
        return submit_and_wait({&submit_info, 1});
    }

    /// Call vkQueueSubmit and use a fence to wait for command buffer submission and execution to complete
    const CommandBuffer &submit_and_wait() const {
        return submit_and_wait(make_info<VkSubmitInfo>({
            .commandBufferCount = 1,
            .pCommandBuffers = &m_cmd_buf,
        }));
    }
};

} // namespace inexor::vulkan_renderer::wrapper
