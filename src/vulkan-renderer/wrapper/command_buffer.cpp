#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <memory>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

CommandBuffer::CommandBuffer(const Device &device, const VkCommandPool cmd_pool, const QueueType queue_type,
                             std::string name)
    : m_device(device), m_queue_type(queue_type), m_name(std::move(name)) {
    auto cmd_buf_ai = make_info<VkCommandBufferAllocateInfo>();
    cmd_buf_ai.commandBufferCount = 1;
    cmd_buf_ai.commandPool = cmd_pool;
    cmd_buf_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (const auto result = vkAllocateCommandBuffers(m_device.device(), &cmd_buf_ai, &m_command_buffer);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkAllocateCommandBuffers failed!", result);
    }

    m_wait_fence = std::make_unique<Fence>(m_device, m_name, false);
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept
    : m_device(other.m_device), m_queue_type(other.m_queue_type) {
    m_command_buffer = std::exchange(other.m_command_buffer, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
    m_wait_fence = std::exchange(other.m_wait_fence, nullptr);
    m_staging_bufs = std::move(other.m_staging_bufs);
}

const CommandBuffer &CommandBuffer::begin_command_buffer(const VkCommandBufferUsageFlags flags) const {
    auto begin_info = make_info<VkCommandBufferBeginInfo>();
    begin_info.flags = flags;
    vkBeginCommandBuffer(m_command_buffer, &begin_info);

    // We must clear the staging buffers which could be left over from previous use of this command buffer
    m_staging_bufs.clear();
    return *this;
}

const CommandBuffer &CommandBuffer::begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi,
                                                      const VkSubpassContents subpass_contents) const {
    vkCmdBeginRenderPass(m_command_buffer, &render_pass_bi, subpass_contents);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_descriptor_sets(const std::span<const VkDescriptorSet> desc_sets,
                                                         const VkPipelineLayout layout,
                                                         const VkPipelineBindPoint bind_point,
                                                         const std::uint32_t first_set,
                                                         const std::span<const std::uint32_t> dyn_offsets) const {
    assert(layout);
    assert(!desc_sets.empty());
    vkCmdBindDescriptorSets(m_command_buffer, bind_point, layout, first_set,
                            static_cast<std::uint32_t>(desc_sets.size()), desc_sets.data(),
                            static_cast<std::uint32_t>(dyn_offsets.size()), dyn_offsets.data());
    return *this;
}

const CommandBuffer &CommandBuffer::bind_index_buffer(const VkBuffer buf, const VkIndexType index_type,
                                                      const VkDeviceSize offset) const {
    assert(buf);
    vkCmdBindIndexBuffer(m_command_buffer, buf, offset, index_type);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_pipeline(const VkPipeline pipeline,
                                                  const VkPipelineBindPoint bind_point) const {
    assert(pipeline);
    vkCmdBindPipeline(m_command_buffer, bind_point, pipeline);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_vertex_buffers(const std::span<const VkBuffer> bufs,
                                                        const std::uint32_t first_binding,
                                                        const std::span<const VkDeviceSize> offsets) const {
    assert(!bufs.empty());
    vkCmdBindVertexBuffers(m_command_buffer, first_binding, static_cast<std::uint32_t>(bufs.size()), bufs.data(),
                           offsets.empty() ? std::vector<VkDeviceSize>(bufs.size(), 0).data() : offsets.data());
    return *this;
}

const CommandBuffer &CommandBuffer::change_image_layout(const VkImage image, const VkImageLayout old_layout,
                                                        const VkImageLayout new_layout,
                                                        const VkImageSubresourceRange subres_range,
                                                        const VkPipelineStageFlags src_mask,
                                                        const VkPipelineStageFlags dst_mask) const {
    assert(new_layout != old_layout);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.image = image;
    barrier.subresourceRange = subres_range;

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

const CommandBuffer &
CommandBuffer::change_image_layout(const VkImage image, const VkImageLayout old_layout, const VkImageLayout new_layout,
                                   const std::uint32_t mip_level_count, const std::uint32_t array_layer_count,
                                   const std::uint32_t base_mip_level, const std::uint32_t base_array_layer,
                                   const VkPipelineStageFlags src_mask, const VkPipelineStageFlags dst_mask) const {

    const VkImageSubresourceRange subres_range{.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                               .baseMipLevel = base_mip_level,
                                               .levelCount = mip_level_count,
                                               .baseArrayLayer = base_array_layer,
                                               .layerCount = array_layer_count};

    return change_image_layout(image, old_layout, new_layout, subres_range, src_mask, dst_mask);
}

const CommandBuffer &CommandBuffer::copy_buffer(const VkBuffer src_buf, const VkBuffer dst_buf,
                                                const std::span<const VkBufferCopy> copy_regions) const {
    assert(src_buf);
    assert(dst_buf);
    assert(!copy_regions.empty());
    vkCmdCopyBuffer(m_command_buffer, src_buf, dst_buf, static_cast<std::uint32_t>(copy_regions.size()),
                    copy_regions.data());
    return *this;
}

const CommandBuffer &CommandBuffer::copy_buffer(const VkBuffer src_buf, const VkBuffer dst_buf,
                                                const VkBufferCopy &copy_region) const {
    return copy_buffer(src_buf, dst_buf, {&copy_region, 1});
}

const CommandBuffer &CommandBuffer::copy_buffer(const VkBuffer src_buf, const VkBuffer dst_buf,
                                                const VkDeviceSize src_buf_size) const {
    VkBufferCopy buf_copy{};
    buf_copy.size = src_buf_size;
    return copy_buffer(src_buf, dst_buf, buf_copy);
}

const CommandBuffer &CommandBuffer::copy_buffer_to_image(const VkBuffer src_buf, const VkImage dst_img,
                                                         const std::span<const VkBufferImageCopy> copy_regions) const {
    assert(src_buf);
    assert(dst_img);
    vkCmdCopyBufferToImage(m_command_buffer, src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<std::uint32_t>(copy_regions.size()), copy_regions.data());
    return *this;
}

const CommandBuffer &CommandBuffer::copy_buffer_to_image(const VkBuffer src_buf, const VkImage dst_img,
                                                         const VkBufferImageCopy &copy_region) const {
    assert(src_buf);
    assert(dst_img);
    vkCmdCopyBufferToImage(m_command_buffer, src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    return *this;
}

const CommandBuffer &CommandBuffer::copy_buffer_to_image(const void *data,
                                                         const VkDeviceSize data_size, // NOLINT
                                                         const VkImage dst_img, const VkBufferImageCopy &copy_region,
                                                         const std::string &name) const {
    return copy_buffer_to_image(create_staging_buffer(data, data_size, name), dst_img, copy_region);
}

const CommandBuffer &CommandBuffer::draw(const std::uint32_t vert_count, const std::uint32_t inst_count,
                                         const std::uint32_t first_vert, const std::uint32_t first_inst) const {
    vkCmdDraw(m_command_buffer, vert_count, inst_count, first_vert, first_inst);
    return *this;
}

const CommandBuffer &CommandBuffer::draw_indexed(const std::uint32_t index_count, const std::uint32_t inst_count,
                                                 const std::uint32_t first_index, const std::int32_t vert_offset,
                                                 const std::uint32_t first_inst) const {
    vkCmdDrawIndexed(m_command_buffer, index_count, inst_count, first_index, vert_offset, first_inst);
    return *this;
}

const CommandBuffer &CommandBuffer::end_command_buffer() const {
    vkEndCommandBuffer(m_command_buffer);
    return *this;
}

const CommandBuffer &CommandBuffer::end_render_pass() const {
    vkCmdEndRenderPass(m_command_buffer);
    return *this;
}

const CommandBuffer &CommandBuffer::pipeline_barrier(const VkPipelineStageFlags src_stage_flags,
                                                     const VkPipelineStageFlags dst_stage_flags,
                                                     const std::span<const VkImageMemoryBarrier> img_mem_barriers,
                                                     const std::span<const VkMemoryBarrier> mem_barriers,
                                                     const std::span<const VkBufferMemoryBarrier> buf_mem_barriers,
                                                     const VkDependencyFlags dep_flags) const {
    // One barrier must be set at least
    assert(!(img_mem_barriers.empty() && mem_barriers.empty()) && buf_mem_barriers.empty());

    vkCmdPipelineBarrier(m_command_buffer, src_stage_flags, dst_stage_flags, dep_flags,
                         static_cast<std::uint32_t>(mem_barriers.size()), mem_barriers.data(),
                         static_cast<std::uint32_t>(buf_mem_barriers.size()), buf_mem_barriers.data(),
                         static_cast<std::uint32_t>(img_mem_barriers.size()), img_mem_barriers.data());
    return *this;
}

const CommandBuffer &CommandBuffer::pipeline_image_memory_barrier(const VkPipelineStageFlags src_stage_flags,
                                                                  const VkPipelineStageFlags dst_stage_flags,
                                                                  const VkImageMemoryBarrier &img_barrier) const {
    return pipeline_barrier(src_stage_flags, dst_stage_flags, {&img_barrier, 1});
}

const CommandBuffer &CommandBuffer::pipeline_memory_barrier(VkPipelineStageFlags src_stage_flags,
                                                            VkPipelineStageFlags dst_stage_flags,
                                                            const VkMemoryBarrier &mem_barrier) const {
    return pipeline_barrier(src_stage_flags, dst_stage_flags, {}, {&mem_barrier, 1});
}

const CommandBuffer &CommandBuffer::pipeline_full_memory_barrier() const {
    auto mem_barrier = make_info<VkMemoryBarrier>();
    mem_barrier.srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT;
    mem_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    return pipeline_memory_barrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, mem_barrier);
}

const CommandBuffer &CommandBuffer::push_constants(const VkPipelineLayout layout, const VkShaderStageFlags stage,
                                                   const std::uint32_t size, const void *data,
                                                   const VkDeviceSize offset) const {
    assert(layout);
    assert(size > 0);
    assert(data);
    vkCmdPushConstants(m_command_buffer, layout, stage, offset, size, data);
    return *this;
}

const CommandBuffer &CommandBuffer::reset_fence() const {
    m_wait_fence->reset();
    return *this;
}

const CommandBuffer &CommandBuffer::submit() const {
    auto submit_info = make_info<VkSubmitInfo>();
    submit_info.pCommandBuffers = &m_command_buffer;
    submit_info.commandBufferCount = 1;

    if (const auto result =
            vkQueueSubmit((m_queue_type == QueueType::GRAPHICS) ? m_device.graphics_queue() : m_device.transfer_queue(),
                          1, &submit_info, m_wait_fence->get())) {
        throw VulkanException("Error: vkQueueSubmit failed!", result);
    }
    return *this;
}

const CommandBuffer &CommandBuffer::submit_and_wait() const {
    submit();
    m_wait_fence->block();
    return *this;
}

} // namespace inexor::vulkan_renderer::wrapper
