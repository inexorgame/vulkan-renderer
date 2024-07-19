#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline.hpp"

#include <cassert>
#include <memory>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::commands {

CommandBuffer::CommandBuffer(const wrapper::Device &device, const VkCommandPool cmd_pool, std::string name)
    : m_device(device), m_name(std::move(name)) {
    const auto cmd_buf_ai = make_info<VkCommandBufferAllocateInfo>({
        .commandPool = cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    });

    // Note that command buffers are allocated from a command pool, meaning the memory required for this will be
    // freed if the corresponding command pool is destroyed. Command buffers are not freed in the destructor.
    if (const auto result = vkAllocateCommandBuffers(m_device.device(), &cmd_buf_ai, &m_cmd_buf);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkAllocateCommandBuffers failed!", result);
    }

    m_device.set_debug_name(m_cmd_buf, m_name);
    m_cmd_buf_execution_completed = std::make_unique<synchronization::Fence>(m_device, m_name, false);
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept : m_device(other.m_device) {
    m_cmd_buf = std::exchange(other.m_cmd_buf, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
    m_cmd_buf_execution_completed = std::exchange(other.m_cmd_buf_execution_completed, nullptr);
}

const CommandBuffer &CommandBuffer::begin_command_buffer(const VkCommandBufferUsageFlags flags) const {
    const auto begin_info = make_info<VkCommandBufferBeginInfo>({
        .flags = flags,
    });
    vkBeginCommandBuffer(m_cmd_buf, &begin_info);
    return *this;
}

const CommandBuffer &CommandBuffer::begin_debug_label_region(std::string name, std::array<float, 4> color) const {
    auto label = make_info<VkDebugUtilsLabelEXT>({
        .pLabelName = name.c_str(),
        .color = {color[0], color[1], color[2], color[3]},
    });
    vkCmdBeginDebugUtilsLabelEXT(m_cmd_buf, &label);
    return *this;
}

const CommandBuffer &CommandBuffer::begin_rendering(const VkRenderingInfo &rendering_info) const {
    vkCmdBeginRendering(m_cmd_buf, &rendering_info);
    return *this;
};

const CommandBuffer &CommandBuffer::bind_descriptor_set(const VkDescriptorSet descriptor_set,
                                                        const std::weak_ptr<GraphicsPipeline> pipeline) const {
    assert(descriptor_set);
    vkCmdBindDescriptorSets(m_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline.lock()->m_pipeline_layout->m_pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_index_buffer(const std::weak_ptr<render_graph::Buffer> buffer,
                                                      const VkIndexType index_type,
                                                      const VkDeviceSize offset) const {
    if (buffer.lock()->m_buffer_type != render_graph::BufferType::INDEX_BUFFER) {
        throw std::invalid_argument("Error: Rendergraph buffer resource " + buffer.lock()->m_name +
                                    " is not an index buffer!");
    }
    vkCmdBindIndexBuffer(m_cmd_buf, buffer.lock()->m_buffer, offset, index_type);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_pipeline(const std::weak_ptr<GraphicsPipeline> pipeline,
                                                  const VkPipelineBindPoint bind_point) const {
    vkCmdBindPipeline(m_cmd_buf, bind_point, pipeline.lock()->m_pipeline);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_vertex_buffer(const std::weak_ptr<render_graph::Buffer> buffer) const {
    if (buffer.lock()->m_buffer_type != render_graph::BufferType::VERTEX_BUFFER) {
        throw std::invalid_argument("Error: Rendergraph buffer resource " + buffer.lock()->m_name +
                                    " is not a vertex buffer!");
    }
    vkCmdBindVertexBuffers(m_cmd_buf, 0, 1, &buffer.lock()->m_buffer, std::vector<VkDeviceSize>(1, 0).data());
    return *this;
}

const CommandBuffer &CommandBuffer::change_image_layout(const VkImage image,
                                                        const VkImageLayout old_layout,
                                                        const VkImageLayout new_layout,
                                                        const VkImageSubresourceRange subres_range,
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

const CommandBuffer &CommandBuffer::change_image_layout(const VkImage img,
                                                        const VkImageLayout old_layout,
                                                        const VkImageLayout new_layout,
                                                        const std::uint32_t mip_level_count,
                                                        const std::uint32_t array_layer_count,
                                                        const std::uint32_t base_mip_level,
                                                        const std::uint32_t base_array_layer,
                                                        const VkPipelineStageFlags src_mask,
                                                        const VkPipelineStageFlags dst_mask) const {
    assert(img);
    return change_image_layout(img, old_layout, new_layout,
                               {
                                   .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                   .baseMipLevel = base_mip_level,
                                   .levelCount = mip_level_count,
                                   .baseArrayLayer = base_array_layer,
                                   .layerCount = array_layer_count,
                               },
                               src_mask, dst_mask);
}

const CommandBuffer &CommandBuffer::copy_buffer(const VkBuffer src_buf,
                                                const VkBuffer dst_buf,
                                                const std::span<const VkBufferCopy> copy_regions) const {
    assert(src_buf);
    assert(dst_buf);
    assert(!copy_regions.empty());
    vkCmdCopyBuffer(m_cmd_buf, src_buf, dst_buf, static_cast<std::uint32_t>(copy_regions.size()), copy_regions.data());
    return *this;
}

const CommandBuffer &
CommandBuffer::copy_buffer(const VkBuffer src_buf, const VkBuffer dst_buf, const VkBufferCopy &copy_region) const {
    return copy_buffer(src_buf, dst_buf, {&copy_region, 1});
}

const CommandBuffer &
CommandBuffer::copy_buffer(const VkBuffer src_buf, const VkBuffer dst_buf, const VkDeviceSize src_buf_size) const {
    return copy_buffer(src_buf, dst_buf, {.size = src_buf_size});
}

const CommandBuffer &CommandBuffer::copy_buffer_to_image(const VkBuffer src_buf,
                                                         const VkImage dst_img,
                                                         const VkBufferImageCopy &copy_region) const {
    assert(src_buf);
    assert(dst_img);
    vkCmdCopyBufferToImage(m_cmd_buf, src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    return *this;
}

const CommandBuffer &
CommandBuffer::copy_buffer_to_image(const VkBuffer buffer, const VkImage img, const VkExtent3D extent) const {
    return copy_buffer_to_image(buffer, img,
                                {
                                    .imageSubresource =
                                        {
                                            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                            .layerCount = 1,
                                        },
                                    .imageExtent =
                                        {
                                            .width = extent.width,
                                            .height = extent.height,
                                            .depth = 1,
                                        },
                                });
}

const CommandBuffer &CommandBuffer::draw(const std::uint32_t vert_count,
                                         const std::uint32_t inst_count,
                                         const std::uint32_t first_vert,
                                         const std::uint32_t first_inst) const {
    vkCmdDraw(m_cmd_buf, vert_count, inst_count, first_vert, first_inst);
    return *this;
}

const CommandBuffer &CommandBuffer::draw_indexed(const std::uint32_t index_count,
                                                 const std::uint32_t inst_count,
                                                 const std::uint32_t first_index,
                                                 const std::int32_t vert_offset,
                                                 const std::uint32_t first_inst) const {
    vkCmdDrawIndexed(m_cmd_buf, index_count, inst_count, first_index, vert_offset, first_inst);
    return *this;
}

const CommandBuffer &CommandBuffer::end_command_buffer() const {
    vkEndCommandBuffer(m_cmd_buf);
    return *this;
}

const CommandBuffer &CommandBuffer::end_debug_label_region() const {
    vkCmdEndDebugUtilsLabelEXT(m_cmd_buf);
    return *this;
}

const CommandBuffer &CommandBuffer::end_rendering() const {
    vkCmdEndRendering(m_cmd_buf);
    return *this;
}

// TODO: Reduce number of methods which deal with barriers to only those which are really needed!

const CommandBuffer &CommandBuffer::pipeline_barrier(const VkPipelineStageFlags src_stage_flags,
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

const CommandBuffer &
CommandBuffer::pipeline_buffer_memory_barrier(const VkPipelineStageFlags src_stage_flags,
                                              const VkPipelineStageFlags dst_stage_flags,
                                              const VkBufferMemoryBarrier &buffer_mem_barrier) const {
    return pipeline_barrier(src_stage_flags, dst_stage_flags, {}, {}, {&buffer_mem_barrier, 1});
}

const CommandBuffer &CommandBuffer::pipeline_buffer_memory_barrier(VkPipelineStageFlags src_stage_flags,
                                                                   VkPipelineStageFlags dst_stage_flags,
                                                                   VkAccessFlags src_access_flags,
                                                                   VkAccessFlags dst_access_flags,
                                                                   VkBuffer buffer,
                                                                   VkDeviceSize size,
                                                                   VkDeviceSize offset) const {
    return pipeline_buffer_memory_barrier(src_stage_flags, dst_stage_flags,
                                          wrapper::make_info<VkBufferMemoryBarrier>({
                                              .srcAccessMask = src_access_flags,
                                              .dstAccessMask = dst_access_flags,
                                              .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                              .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                              .buffer = buffer,
                                              .offset = offset,
                                              .size = size,
                                          }));
}

const CommandBuffer &
CommandBuffer::pipeline_buffer_memory_barriers(const VkPipelineStageFlags src_stage_flags,
                                               const VkPipelineStageFlags dst_stage_flags,
                                               const std::span<const VkBufferMemoryBarrier> buffer_mem_barriers) const {
    return pipeline_barrier(src_stage_flags, dst_stage_flags, {}, {}, buffer_mem_barriers);
}

const CommandBuffer &CommandBuffer::pipeline_buffer_memory_barrier_before_copy_buffer(const VkBuffer buffer) const {
    return pipeline_buffer_memory_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                          VK_ACCESS_MEMORY_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT, buffer);
}

const CommandBuffer &CommandBuffer::pipeline_buffer_memory_barrier_after_copy_buffer(const VkBuffer buffer) const {
    return pipeline_buffer_memory_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                          VK_ACCESS_TRANSFER_WRITE_BIT,
                                          VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT, buffer);
}

const CommandBuffer &CommandBuffer::pipeline_image_memory_barrier(const VkPipelineStageFlags src_stage_flags,
                                                                  const VkPipelineStageFlags dst_stage_flags,
                                                                  const VkImageMemoryBarrier &img_barrier) const {
    return pipeline_barrier(src_stage_flags, dst_stage_flags, {&img_barrier, 1});
}

const CommandBuffer &CommandBuffer::pipeline_image_memory_barrier(const VkPipelineStageFlags src_stage_flags,
                                                                  const VkPipelineStageFlags dst_stage_flags,
                                                                  const VkAccessFlags src_access_flags,
                                                                  const VkAccessFlags dst_access_flags,
                                                                  const VkImageLayout old_img_layout,
                                                                  const VkImageLayout new_img_layout,
                                                                  const VkImage img) const {
    assert(img);
    return pipeline_image_memory_barrier(src_stage_flags, dst_stage_flags,
                                         wrapper::make_info<VkImageMemoryBarrier>({
                                             .srcAccessMask = src_access_flags,
                                             .dstAccessMask = dst_access_flags,
                                             .oldLayout = old_img_layout,
                                             .newLayout = new_img_layout,
                                             .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                             .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
                                             .image = img,
                                             .subresourceRange =
                                                 {
                                                     .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                                     .baseMipLevel = 0,
                                                     .levelCount = 1,
                                                     .baseArrayLayer = 0,
                                                     .layerCount = 1,
                                                 },
                                         }));
}

const CommandBuffer &CommandBuffer::pipeline_image_memory_barrier_after_copy_buffer_to_image(const VkImage img) const {
    assert(img);
    return pipeline_image_memory_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT,           // src_stage_flags
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,    // dst_stage_flags
                                         VK_ACCESS_TRANSFER_WRITE_BIT,             // src_access_flags
                                         VK_ACCESS_SHADER_READ_BIT,                // dst_access_flags
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     // old_img_layout
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // new_img_layout
                                         img);
}

const CommandBuffer &CommandBuffer::pipeline_image_memory_barrier_before_copy_buffer_to_image(const VkImage img) const {
    assert(img);
    return pipeline_image_memory_barrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,    // src_stage_flags
                                         VK_PIPELINE_STAGE_TRANSFER_BIT,       // dst_stage_flags
                                         0,                                    // src_access_flags
                                         VK_ACCESS_TRANSFER_WRITE_BIT,         // dst_access_flags
                                         VK_IMAGE_LAYOUT_UNDEFINED,            // old_img_layout
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, // new_img_layout
                                         img);
}

const CommandBuffer &
CommandBuffer::pipeline_image_memory_barriers(const VkPipelineStageFlags src_stage_flags,
                                              const VkPipelineStageFlags dst_stage_flags,
                                              const std::span<const VkImageMemoryBarrier> img_barrier) const {
    return pipeline_barrier(src_stage_flags, dst_stage_flags, img_barrier);
}

const CommandBuffer &CommandBuffer::pipeline_memory_barrier(const VkPipelineStageFlags src_stage_flags,
                                                            const VkPipelineStageFlags dst_stage_flags,
                                                            const VkMemoryBarrier &mem_barrier) const {
    return pipeline_barrier(src_stage_flags, dst_stage_flags, {}, {&mem_barrier, 1});
}

const CommandBuffer &
CommandBuffer::pipeline_memory_barriers(const VkPipelineStageFlags src_stage_flags,
                                        const VkPipelineStageFlags dst_stage_flags,
                                        const std::span<const VkMemoryBarrier> mem_barriers) const {
    return pipeline_barrier(src_stage_flags, dst_stage_flags, {}, mem_barriers);
}

const CommandBuffer &CommandBuffer::full_barrier() const {
    return pipeline_memory_barrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                                   make_info<VkMemoryBarrier>({
                                       .srcAccessMask = VK_ACCESS_MEMORY_WRITE_BIT,
                                       .dstAccessMask = VK_ACCESS_MEMORY_READ_BIT,
                                   }));
}

const CommandBuffer &CommandBuffer::insert_debug_label(std::string name, std::array<float, 4> color) const {
    auto label = make_info<VkDebugUtilsLabelEXT>({
        .pLabelName = name.c_str(),
        .color = {color[0], color[1], color[2], color[3]},
    });
    vkCmdInsertDebugUtilsLabelEXT(m_cmd_buf, &label);
    return *this;
}

const CommandBuffer &CommandBuffer::push_constants(const VkPipelineLayout layout,
                                                   const VkShaderStageFlags stage,
                                                   const std::uint32_t size,
                                                   const void *data,
                                                   const VkDeviceSize offset) const {
    assert(layout);
    assert(size > 0);
    assert(data);
    vkCmdPushConstants(m_cmd_buf, layout, stage, static_cast<std::uint32_t>(offset), size, data);
    return *this;
}

const CommandBuffer &CommandBuffer::set_suboperation_debug_name(std::string name) const {
    m_device.set_debug_name(m_cmd_buf, m_name + name);
    return *this;
}

void CommandBuffer::submit_and_wait(const std::span<const VkSemaphore> wait_semaphores,
                                    const std::span<const VkSemaphore> signal_semaphores) const {
    end_command_buffer();

    const VkPipelineStageFlags wait_dst_stage_mask[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    const auto submit_info = make_info<VkSubmitInfo>({
        .waitSemaphoreCount = static_cast<std::uint32_t>(wait_semaphores.size()),
        .pWaitSemaphores = wait_semaphores.data(),
        .pWaitDstStageMask = wait_dst_stage_mask,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_cmd_buf,
        .signalSemaphoreCount = static_cast<std::uint32_t>(signal_semaphores.size()),
        .pSignalSemaphores = signal_semaphores.data(),
    });

    if (const auto result =
            vkQueueSubmit(m_device.graphics_queue(), 1, &submit_info, m_cmd_buf_execution_completed->m_fence)) {
        throw VulkanException("[CommandBuffer::submit] Error: vkQueueSubmit failed!", result);
    }
    m_cmd_buf_execution_completed->wait();
}

} // namespace inexor::vulkan_renderer::wrapper::commands
