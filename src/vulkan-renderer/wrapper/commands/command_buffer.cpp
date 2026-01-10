#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"

#include <cassert>
#include <memory>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::commands {

using tools::VulkanException;

CommandBuffer::CommandBuffer(const Device &device, const VkCommandPool cmd_pool, std::string name)
    : m_device(device), m_name(std::move(name)) {
    const auto cmd_buf_ai = make_info<VkCommandBufferAllocateInfo>({
        .commandPool = cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    });

    if (const auto result = vkAllocateCommandBuffers(m_device.device(), &cmd_buf_ai, &m_command_buffer);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkAllocateCommandBuffers failed!", result, m_name);
    }

    m_device.set_debug_name(m_command_buffer, m_name);

    m_wait_fence = std::make_unique<Fence>(m_device, m_name, false);
    m_cmd_buf_execution_completed = std::make_unique<Fence>(m_device, m_name, false);
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept : m_device(other.m_device) {
    m_command_buffer = std::exchange(other.m_command_buffer, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
    m_wait_fence = std::exchange(other.m_wait_fence, nullptr);
    m_staging_bufs = std::move(other.m_staging_bufs);
}

const CommandBuffer &CommandBuffer::begin_command_buffer(const VkCommandBufferUsageFlags flags) const {
    const auto begin_info = make_info<VkCommandBufferBeginInfo>({
        .flags = flags,
    });
    vkBeginCommandBuffer(m_command_buffer, &begin_info);

    // We must clear the staging buffers which could be left over from previous use of this command buffer
    m_staging_bufs.clear();
    return *this;
}

const CommandBuffer &CommandBuffer::begin_debug_label_region(std::string name, std::array<float, 4> color) const {
    if (name.empty()) {
        // NOTE: Despite Vulkan spec allowing name to be empty, we strictly enforce this rule in our code base!
        throw InexorException("Error: Parameter 'name' is empty!");
    }
    auto label = make_info<VkDebugUtilsLabelEXT>({
        .pLabelName = name.c_str(),
        .color = {color[0], color[1], color[2], color[3]},
    });
    vkCmdBeginDebugUtilsLabelEXT(m_command_buffer, &label);
    return *this;
}

const CommandBuffer &CommandBuffer::begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi,
                                                      const VkSubpassContents subpass_contents) const {
    vkCmdBeginRenderPass(m_command_buffer, &render_pass_bi, subpass_contents);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_descriptor_set(
    const VkDescriptorSet descriptor_set,
    const std::weak_ptr<inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipeline> pipeline) const {
    if (!descriptor_set) {
        throw InexorException("Error: Parameter 'descriptor_set' is invalid!");
    }
    if (pipeline.expired()) {
        throw InexorException("Error: Parameter 'pipeline' is an invalid pointer!");
    }
    vkCmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.lock()->pipeline_layout(), 0, 1,
                            &descriptor_set, 0, nullptr);
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

const CommandBuffer &
CommandBuffer::bind_index_buffer(const std::weak_ptr<inexor::vulkan_renderer::render_graph::Buffer> buffer,
                                 const VkIndexType index_type, const VkDeviceSize offset) const {
    if (buffer.expired()) {
        throw InexorException("Error: Parameter 'buffer' is an invalid pointer!");
    }
    if (buffer.lock()->type() != inexor::vulkan_renderer::render_graph::BufferType::INDEX_BUFFER) {
        throw InexorException("Error: Rendergraph buffer resource " + buffer.lock()->name() +
                              " is not an index buffer!");
    }
    vkCmdBindIndexBuffer(m_command_buffer, buffer.lock()->buffer(), offset, index_type);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_index_buffer(const VkBuffer buf, const VkIndexType index_type,
                                                      const VkDeviceSize offset) const {
    assert(buf);
    vkCmdBindIndexBuffer(m_command_buffer, buf, offset, index_type);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_pipeline(
    std::weak_ptr<inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipeline> pipeline) const {
    return bind_pipeline(pipeline.lock()->pipeline());
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

const CommandBuffer &
CommandBuffer::change_image_layout(const VkImage image, const VkImageLayout old_layout, const VkImageLayout new_layout,
                                   const std::uint32_t mip_level_count, const std::uint32_t array_layer_count,
                                   const std::uint32_t base_mip_level, const std::uint32_t base_array_layer,
                                   const VkPipelineStageFlags src_mask, const VkPipelineStageFlags dst_mask) const {
    return change_image_layout(image, old_layout, new_layout,
                               {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .baseMipLevel = base_mip_level,
                                .levelCount = mip_level_count,
                                .baseArrayLayer = base_array_layer,
                                .layerCount = array_layer_count},
                               src_mask, dst_mask);
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
    return copy_buffer(src_buf, dst_buf, {.size = src_buf_size});
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
    if (!src_buf) {
        throw InexorException("Error: Parameter 'src_buf' is invalid!");
    }
    if (!dst_img) {
        throw InexorException("Error: Parameter 'dst_img' is invalid!");
    }
    vkCmdCopyBufferToImage(m_command_buffer, src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    return *this;
}

const CommandBuffer &CommandBuffer::copy_buffer_to_image(const VkBuffer buffer, const VkImage img,
                                                         const VkExtent3D extent) const {
    // NOTE: We delegate error checks to the other function overload
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

const CommandBuffer &CommandBuffer::copy_buffer_to_image(const void *data,
                                                         const VkDeviceSize data_size, // NOLINT
                                                         const VkImage dst_img, const VkBufferImageCopy &copy_region,
                                                         const std::string &name) const {
    return copy_buffer_to_image(create_staging_buffer(data, data_size, name), dst_img, copy_region);
}

const CommandBuffer &
CommandBuffer::copy_buffer_to_image(const VkBuffer src_buf,
                                    const std::weak_ptr<inexor::vulkan_renderer::render_graph::Image> img) const {
    // NOTE: We delegate error checks to the other function overload
    const auto &image = img.lock();
    return copy_buffer_to_image(src_buf, image->image(),
                                {
                                    .width = image->width(),
                                    .height = image->height(),
                                    .depth = 1,
                                });
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

const CommandBuffer &CommandBuffer::begin_rendering(const VkRenderingInfo &rendering_info) const {
    vkCmdBeginRendering(m_command_buffer, &rendering_info);
    return *this;
};

const CommandBuffer &CommandBuffer::end_render_pass() const {
    vkCmdEndRenderPass(m_command_buffer);
    return *this;
}

const CommandBuffer &CommandBuffer::end_debug_label_region() const {
    vkCmdEndDebugUtilsLabelEXT(m_command_buffer);
    return *this;
}

const CommandBuffer &CommandBuffer::end_rendering() const {
    vkCmdEndRendering(m_command_buffer);
    return *this;
}

const CommandBuffer &CommandBuffer::pipeline_barrier(const VkPipelineStageFlags src_stage_flags,
                                                     const VkPipelineStageFlags dst_stage_flags,
                                                     const std::span<const VkImageMemoryBarrier> img_mem_barriers,
                                                     const std::span<const VkMemoryBarrier> mem_barriers,
                                                     const std::span<const VkBufferMemoryBarrier> buf_mem_barriers,
                                                     const VkDependencyFlags dep_flags) const {
    vkCmdPipelineBarrier(m_command_buffer, src_stage_flags, dst_stage_flags, dep_flags,
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
                                                                   VkAccessFlags dst_access_flags, VkBuffer buffer,
                                                                   VkDeviceSize size, VkDeviceSize offset) const {
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

const CommandBuffer &CommandBuffer::pipeline_image_memory_barrier(
    const VkPipelineStageFlags src_stage_flags, const VkPipelineStageFlags dst_stage_flags,
    const VkAccessFlags src_access_flags, const VkAccessFlags dst_access_flags, const VkImageLayout old_img_layout,
    const VkImageLayout new_img_layout, const VkImage img) const {
    if (!img) {
        throw InexorException("Error: Parameter 'img' is invalid!");
    }
    return pipeline_image_memory_barrier(src_stage_flags, dst_stage_flags,
                                         make_info<VkImageMemoryBarrier>({
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
    if (!img) {
        throw InexorException("Error: Parameter 'img' is invalid!");
    }
    return pipeline_image_memory_barrier(VK_PIPELINE_STAGE_TRANSFER_BIT,           // src_stage_flags
                                         VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,    // dst_stage_flags
                                         VK_ACCESS_TRANSFER_WRITE_BIT,             // src_access_flags
                                         VK_ACCESS_SHADER_READ_BIT,                // dst_access_flags
                                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,     // old_img_layout
                                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // new_img_layout
                                         img);
}

const CommandBuffer &CommandBuffer::pipeline_image_memory_barrier_before_copy_buffer_to_image(const VkImage img) const {
    if (!img) {
        throw InexorException("Error: Parameter 'img' is invalid!");
    }
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
    if (name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
    auto label = make_info<VkDebugUtilsLabelEXT>({
        .pLabelName = name.c_str(),
        .color = {color[0], color[1], color[2], color[3]},
    });
    vkCmdInsertDebugUtilsLabelEXT(m_command_buffer, &label);
    return *this;
}

const CommandBuffer &CommandBuffer::push_constants(const VkPipelineLayout layout, const VkShaderStageFlags stage,
                                                   const std::uint32_t size, const void *data,
                                                   const VkDeviceSize offset) const {
    assert(layout);
    assert(size > 0);
    assert(data);
    vkCmdPushConstants(m_command_buffer, layout, stage, static_cast<std::uint32_t>(offset), size, data);
    return *this;
}

const CommandBuffer &CommandBuffer::reset_fence() const {
    m_wait_fence->reset();
    return *this;
}

const CommandBuffer &CommandBuffer::set_scissor(const VkRect2D scissor) const {
    vkCmdSetScissor(m_command_buffer, 0, 1, &scissor);
    return *this;
}

const CommandBuffer &CommandBuffer::submit(const std::span<const VkSubmitInfo> submit_infos) const {
    assert(!submit_infos.empty());
    end_command_buffer();

    if (const auto result = vkQueueSubmit(m_device.graphics_queue(), static_cast<std::uint32_t>(submit_infos.size()),
                                          submit_infos.data(), m_wait_fence->fence())) {
        throw VulkanException("Error: vkQueueSubmit failed!", result);
    }
    return *this;
}

const CommandBuffer &CommandBuffer::submit(const VkSubmitInfo submit_info) const {
    return submit({&submit_info, 1});
}

const CommandBuffer &CommandBuffer::submit() const {
    return submit(make_info<VkSubmitInfo>({
        .commandBufferCount = 1,
        .pCommandBuffers = &m_command_buffer,
    }));
}

const CommandBuffer &CommandBuffer::set_viewport(const VkViewport viewport) const {
    vkCmdSetViewport(m_command_buffer, 0, 1, &viewport);
    return *this;
}

const CommandBuffer &CommandBuffer::set_suboperation_debug_name(std::string name) const {
    m_device.set_debug_name(m_command_buffer, m_name + name);
    return *this;
}

const CommandBuffer &CommandBuffer::submit_and_wait(const std::span<const VkSubmitInfo> submit_infos) const {
    submit(submit_infos);
    m_wait_fence->wait();
    return *this;
}

const CommandBuffer &CommandBuffer::submit_and_wait(VkSubmitInfo submit_info) const {
    submit_info.pCommandBuffers = &m_command_buffer;
    return submit_and_wait({&submit_info, 1});
}

const CommandBuffer &CommandBuffer::submit_and_wait() const {
    return submit_and_wait(make_info<VkSubmitInfo>({
        .commandBufferCount = 1,
        .pCommandBuffers = &m_command_buffer,
    }));
}

void CommandBuffer::submit_and_wait(const VkQueueFlagBits queue_type,
                                    const std::span<const VkSemaphore> wait_semaphores,
                                    const std::span<const VkSemaphore> signal_semaphores) const {
    end_command_buffer();

    // TODO: What to do here with graphics queue?

    // NOTE: We must specify as many pipeline stage flags as there are wait semaphores!
    std::vector<VkPipelineStageFlags> wait_stages(wait_semaphores.size(),
                                                  VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

    const auto submit_info = make_info<VkSubmitInfo>({
        .waitSemaphoreCount = static_cast<std::uint32_t>(wait_semaphores.size()),
        .pWaitSemaphores = wait_semaphores.data(),
        .pWaitDstStageMask = wait_stages.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = &m_command_buffer,
        .signalSemaphoreCount = static_cast<std::uint32_t>(signal_semaphores.size()),
        .pSignalSemaphores = signal_semaphores.data(),
    });

    // TODO: Support VK_QUEUE_SPARSE_BINDING_BIT if required
    auto get_queue = [&]() {
        switch (queue_type) {
        case VK_QUEUE_TRANSFER_BIT: {
            return m_device.m_transfer_queue;
        }
        case VK_QUEUE_COMPUTE_BIT: {
            return m_device.m_compute_queue;
        }
        default: {
            // VK_QUEUE_GRAPHICS_BIT and rest
            return m_device.m_graphics_queue;
        }
        }
    };

    if (const auto result = vkQueueSubmit(get_queue(), 1, &submit_info, m_cmd_buf_execution_completed->fence())) {
        throw VulkanException("Error: vkQueueSubmit failed!", result, m_name);
    }
    m_cmd_buf_execution_completed->wait();
}

void CommandBuffer::set_debug_name(const std::string &name) {
    m_device.set_debug_name(m_command_buffer, name);
}

} // namespace inexor::vulkan_renderer::wrapper::commands
