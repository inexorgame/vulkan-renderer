#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/per_frame_descriptor_sets.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::commands {

// Using declarations
using tools::make_info;
using tools::VulkanException;

CommandBuffer::CommandBuffer(const core::Device &device, const VkCommandPool cmd_pool, std::string name,
                             const VkCommandBufferLevel level)
    : m_device(device), m_name(std::move(name)) {

    const auto cmd_buf_ai = make_info<VkCommandBufferAllocateInfo>({
        .commandPool = cmd_pool,
        .level = level,
        .commandBufferCount = 1,
    });

    if (const auto result = vkAllocateCommandBuffers(m_device.device(), &cmd_buf_ai, &m_cmd_buf);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkAllocateCommandBuffers failed!", result, m_name);
    }
    m_device.set_debug_name(m_cmd_buf, m_name);
    m_wait_fence = std::make_unique<Fence>(m_device, m_name);
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept : m_device(other.m_device) {
    m_cmd_buf = std::exchange(other.m_cmd_buf, VK_NULL_HANDLE);
    m_name = std::move(other.m_name);
    m_wait_fence = std::move(other.m_wait_fence);
    m_has_been_submitted = other.m_has_been_submitted;
    m_wait_submit_infos_scratch = std::move(other.m_wait_submit_infos_scratch);
    m_signal_submit_infos_scratch = std::move(other.m_signal_submit_infos_scratch);
}

const CommandBuffer &CommandBuffer::begin_command_buffer(const VkCommandBufferUsageFlags flags) const {
    const auto begin_info = make_info<VkCommandBufferBeginInfo>({
        .flags = flags,
    });
    vkBeginCommandBuffer(m_cmd_buf, &begin_info);
    return *this;
}

const CommandBuffer &
CommandBuffer::begin_secondary_command_buffer(const VkCommandBufferInheritanceInfo &inheritance_info,
                                              const VkCommandBufferUsageFlags flags) const {
    const auto begin_info = make_info<VkCommandBufferBeginInfo>({
        .flags = flags,
        .pInheritanceInfo = &inheritance_info,
    });
    vkBeginCommandBuffer(m_cmd_buf, &begin_info);
    return *this;
}

const CommandBuffer &CommandBuffer::end_recording() const {
    return end_command_buffer();
}

void CommandBuffer::reset_recording() const {
    reset();
}

const CommandBuffer &
CommandBuffer::execute_secondary_command_buffers(const std::span<const VkCommandBuffer> secondary_cmd_buffers) const {
    if (secondary_cmd_buffers.empty()) {
        throw std::invalid_argument("Error: Parameter 'secondary_cmd_buffers' is empty!");
    }
    vkCmdExecuteCommands(m_cmd_buf, static_cast<std::uint32_t>(secondary_cmd_buffers.size()),
                         secondary_cmd_buffers.data());
    return *this;
}

const CommandBuffer &CommandBuffer::begin_debug_label_region(const std::string &name,
                                                             std::array<float, 4> color) const {
    if (name.empty()) {
        // NOTE: Despite Vulkan spec allowing name to be empty, we strictly enforce this rule in our code base!
        throw InexorException("Error: Parameter 'name' is empty!");
    }
    auto label = make_info<VkDebugUtilsLabelEXT>({
        .pLabelName = name.c_str(),
        .color = {color[0], color[1], color[2], color[3]},
    });
    vkCmdBeginDebugUtilsLabelEXT(m_cmd_buf, &label);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_descriptor_set(const VkDescriptorSet descriptor_set,
                                                        const std::weak_ptr<GraphicsPipeline> pipeline) const {
    if (!descriptor_set) {
        throw InexorException("Error: Parameter 'descriptor_set' is invalid!");
    }
    const auto pipeline_ref = pipeline.lock();
    if (!pipeline_ref) {
        throw InexorException("Error: Parameter 'pipeline' is an invalid pointer!");
    }
    vkCmdBindDescriptorSets(m_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_ref->pipeline_layout(), 0, 1,
                            &descriptor_set, 0, nullptr);
    return *this;
}

const CommandBuffer &
CommandBuffer::bind_descriptor_set(const std::weak_ptr<descriptors::PerFrameDescriptorSets> descriptor_sets,
                                   const std::weak_ptr<GraphicsPipeline> pipeline) const {
    const auto descriptor_sets_ref = descriptor_sets.lock();
    if (!descriptor_sets_ref) {
        throw InexorException("Error: Parameter 'descriptor_set' is an invalid pointer!");
    }
    return bind_descriptor_set(descriptor_sets_ref->current_descriptor_set(), pipeline);
}

const CommandBuffer &
CommandBuffer::bind_descriptor_set(const std::weak_ptr<descriptors::PerFrameDescriptorSets> descriptor_sets) const {
    const auto desc_sets = descriptor_sets.lock();
    if (!desc_sets) {
        throw InexorException("Error: Parameter 'descriptor_set' is an invalid pointer!");
    }
    const auto descriptor_set = desc_sets->current_descriptor_set();
    if (!descriptor_set) {
        throw InexorException("Error: Current descriptor set is invalid!");
    }

    const auto pipeline_layout = desc_sets->pipeline_layout();
    if (!pipeline_layout) {
        throw InexorException("Error: Descriptor set pipeline layout is invalid!");
    }

    vkCmdBindDescriptorSets(m_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0,
                            nullptr);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_descriptor_sets(const std::span<const VkDescriptorSet> desc_sets,
                                                         const VkPipelineLayout layout,
                                                         const VkPipelineBindPoint bind_point,
                                                         const std::uint32_t first_set,
                                                         const std::span<const std::uint32_t> dyn_offsets) const {
    if (!layout) {
        throw std::invalid_argument("Error: Parameter 'layout' is invalid!");
    }
    if (desc_sets.empty()) {
        throw std::invalid_argument("Error: Parameter 'desc_sets' is empty!");
    }
    vkCmdBindDescriptorSets(m_cmd_buf, bind_point, layout, first_set, static_cast<std::uint32_t>(desc_sets.size()),
                            desc_sets.data(), static_cast<std::uint32_t>(dyn_offsets.size()), dyn_offsets.data());
    return *this;
}

const CommandBuffer &
CommandBuffer::bind_index_buffer(const std::weak_ptr<inexor::vulkan_renderer::render_graph::Buffer> buffer,
                                 const VkIndexType index_type, const VkDeviceSize offset) const {
    const auto buffer_ref = buffer.lock();
    if (!buffer_ref) {
        throw InexorException("Error: Parameter 'buffer' is an invalid pointer!");
    }
    if (buffer_ref->type() != inexor::vulkan_renderer::render_graph::BufferType::INDEX_BUFFER) {
        throw InexorException("Error: Rendergraph buffer resource " + buffer_ref->name() + " is not an index buffer!");
    }
    vkCmdBindIndexBuffer(m_cmd_buf, buffer_ref->buffer(), offset, index_type);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_index_buffer(const VkBuffer buf, const VkIndexType index_type,
                                                      const VkDeviceSize offset) const {
    if (!buf) {
        throw std::invalid_argument("Error: Parameter 'buf' is invalid!");
    }
    vkCmdBindIndexBuffer(m_cmd_buf, buf, offset, index_type);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_pipeline(
    std::weak_ptr<inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipeline> pipeline) const {
    const auto pipeline_ref = pipeline.lock();
    if (!pipeline_ref) {
        throw InexorException("Error: Parameter 'pipeline' is an invalid pointer!");
    }
    return bind_pipeline(pipeline_ref->pipeline());
}

const CommandBuffer &CommandBuffer::bind_pipeline(const VkPipeline pipeline,
                                                  const VkPipelineBindPoint bind_point) const {
    if (!pipeline) {
        throw std::invalid_argument("Error: Parameter 'pipeline' is invalid!");
    }
    vkCmdBindPipeline(m_cmd_buf, bind_point, pipeline);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_vertex_buffers(const std::span<const VkBuffer> bufs,
                                                        const std::uint32_t first_binding,
                                                        const std::span<const VkDeviceSize> offsets) const {
    if (bufs.empty()) {
        throw std::invalid_argument("Error: Parameter 'bufs' is empty!");
    }
    if (!offsets.empty()) {
        vkCmdBindVertexBuffers(m_cmd_buf, first_binding, static_cast<std::uint32_t>(bufs.size()), bufs.data(),
                               offsets.data());
        return *this;
    }
    // NOTE: When no offsets are specified, all buffers are bound at offset 0. We use a fixed-size stack array here
    // instead of a temporary std::vector to avoid a heap allocation on every call.
    constexpr std::size_t MAX_STACK_BINDINGS = 16;
    if (bufs.size() > MAX_STACK_BINDINGS) {
        throw std::out_of_range("Too many vertex buffer bindings for the fixed-size stack buffer!");
    }
    const std::array<VkDeviceSize, MAX_STACK_BINDINGS> zero_offsets{};
    vkCmdBindVertexBuffers(m_cmd_buf, first_binding, static_cast<std::uint32_t>(bufs.size()), bufs.data(),
                           zero_offsets.data());
    return *this;
}

const CommandBuffer &CommandBuffer::change_image_layout(const VkImage image, const VkImageLayout old_layout,
                                                        const VkImageLayout new_layout,
                                                        const VkImageSubresourceRange subres_range,
                                                        const VkPipelineStageFlags src_mask,
                                                        const VkPipelineStageFlags dst_mask) const {
    if (image == VK_NULL_HANDLE) {
        throw std::invalid_argument("Error: Parameter 'image' is an invalid pointer!");
    }
    if (new_layout == old_layout) {
        throw std::invalid_argument("Error: old_layout and new_layout must differ!");
    }

    auto barrier = VkImageMemoryBarrier2{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = static_cast<VkPipelineStageFlags2>(src_mask),
        .dstStageMask = static_cast<VkPipelineStageFlags2>(dst_mask),
        .oldLayout = old_layout,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = subres_range,
    };

    switch (old_layout) {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        barrier.srcAccessMask = 0;
        break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        barrier.srcAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        break;
    default:
        break;
    }

    switch (new_layout) {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_2_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        barrier.dstAccessMask = barrier.dstAccessMask | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        if (barrier.srcAccessMask == 0) {
            barrier.srcAccessMask = VK_ACCESS_2_HOST_WRITE_BIT | VK_ACCESS_2_TRANSFER_WRITE_BIT;
        }
        barrier.dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT;
        break;
    default:
        break;
    }

    return pipeline_image_memory_barrier(barrier);
}

const CommandBuffer &
CommandBuffer::change_image_layout(const VkImage image, const VkFormat format, const VkImageLayout old_layout,
                                   const VkImageLayout new_layout, const std::uint32_t mip_level_count,
                                   const std::uint32_t array_layer_count, const std::uint32_t base_mip_level,
                                   const std::uint32_t base_array_layer, const VkPipelineStageFlags src_mask,
                                   const VkPipelineStageFlags dst_mask) const {

    auto deduce_aspect_mask = [&]() -> VkImageAspectFlags {
        switch (format) {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;

        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;

        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;

        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
        }
    }();

    return change_image_layout(image, old_layout, new_layout,
                               {
                                   .aspectMask = deduce_aspect_mask,
                                   .baseMipLevel = base_mip_level,
                                   .levelCount = mip_level_count,
                                   .baseArrayLayer = base_array_layer,
                                   .layerCount = array_layer_count,
                               },
                               src_mask, dst_mask);
}

const CommandBuffer &CommandBuffer::copy_buffer(const VkBuffer src_buf, const VkBuffer dst_buf,
                                                const std::span<const VkBufferCopy> copy_regions) const {
    if (!src_buf) {
        throw std::invalid_argument("Error: Parameter 'src_buf' is invalid!");
    }
    if (!dst_buf) {
        throw std::invalid_argument("Error: Parameter 'dst_buf' is invalid!");
    }
    if (copy_regions.empty()) {
        throw std::invalid_argument("Error: Parameter 'copy_regions' is empty!");
    }
    vkCmdCopyBuffer(m_cmd_buf, src_buf, dst_buf, static_cast<std::uint32_t>(copy_regions.size()), copy_regions.data());
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
    if (!src_buf) {
        throw std::invalid_argument("Error: Parameter 'src_buf' is invalid!");
    }
    if (!dst_img) {
        throw std::invalid_argument("Error: Parameter 'dst_img' is invalid!");
    }
    if (copy_regions.empty()) {
        throw std::invalid_argument("Error: Parameter 'copy_regions' is empty!");
    }
    vkCmdCopyBufferToImage(m_cmd_buf, src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
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
    vkCmdCopyBufferToImage(m_cmd_buf, src_buf, dst_img, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
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

const CommandBuffer &CommandBuffer::copy_buffer_to_image(const VkBuffer src_buf, const std::weak_ptr<Image> img) const {
    // NOTE: We delegate error checks to the other function overload
    const auto image = img.lock();
    if (!image) {
        throw InexorException("Error: Parameter 'img' is an invalid pointer!");
    }
    return copy_buffer_to_image(src_buf, image->image(),
                                {
                                    .width = image->width(),
                                    .height = image->height(),
                                    .depth = 1,
                                });
}

const CommandBuffer &CommandBuffer::draw(const std::uint32_t vert_count, const std::uint32_t inst_count,
                                         const std::uint32_t first_vert, const std::uint32_t first_inst) const {
    vkCmdDraw(m_cmd_buf, vert_count, inst_count, first_vert, first_inst);
    return *this;
}

const CommandBuffer &CommandBuffer::draw_indexed(const std::uint32_t index_count, const std::uint32_t inst_count,
                                                 const std::uint32_t first_index, const std::int32_t vert_offset,
                                                 const std::uint32_t first_inst) const {
    vkCmdDrawIndexed(m_cmd_buf, index_count, inst_count, first_index, vert_offset, first_inst);
    return *this;
}

const CommandBuffer &CommandBuffer::end_command_buffer() const {
    vkEndCommandBuffer(m_cmd_buf);
    return *this;
}

const CommandBuffer &CommandBuffer::begin_rendering(const VkRenderingInfo &rendering_info) const {
    vkCmdBeginRendering(m_cmd_buf, &rendering_info);
    return *this;
};

const CommandBuffer &CommandBuffer::end_debug_label_region() const {
    vkCmdEndDebugUtilsLabelEXT(m_cmd_buf);
    return *this;
}

const CommandBuffer &CommandBuffer::end_rendering() const {
    vkCmdEndRendering(m_cmd_buf);
    return *this;
}

const CommandBuffer &CommandBuffer::pipeline_barrier(const VkDependencyInfo &dependency_info) const {
    vkCmdPipelineBarrier2(m_cmd_buf, &dependency_info);
    return *this;
}

const CommandBuffer &
CommandBuffer::pipeline_buffer_memory_barrier(const VkBufferMemoryBarrier2 &buffer_mem_barrier) const {
    const auto dependency_info = make_info<VkDependencyInfo>({
        .bufferMemoryBarrierCount = 1,
        .pBufferMemoryBarriers = &buffer_mem_barrier,
    });
    return pipeline_barrier(dependency_info);
}

const CommandBuffer &CommandBuffer::pipeline_image_memory_barrier(const VkImageMemoryBarrier2 &img_barrier) const {
    const auto dependency_info = make_info<VkDependencyInfo>({
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &img_barrier,
    });
    return pipeline_barrier(dependency_info);
}

const CommandBuffer &CommandBuffer::pipeline_memory_barrier(const VkMemoryBarrier2 &mem_barrier) const {
    const auto dependency_info = make_info<VkDependencyInfo>({
        .memoryBarrierCount = 1,
        .pMemoryBarriers = &mem_barrier,
    });
    return pipeline_barrier(dependency_info);
}

const CommandBuffer &CommandBuffer::barrier_transfer_write_to_shader_read() const {
    return pipeline_memory_barrier({
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
        .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
    });
}

const CommandBuffer &CommandBuffer::barrier_color_attachment_write_to_shader_read() const {
    return pipeline_memory_barrier({
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
    });
}

const CommandBuffer &CommandBuffer::barrier_depth_stencil_write_to_shader_read() const {
    return pipeline_memory_barrier({
        .sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
        .srcAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT |
                        VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_2_SHADER_READ_BIT,
    });
}

const CommandBuffer &CommandBuffer::blit_image(VkImage src_image, VkImageLayout src_layout, VkImage dst_image,
                                               VkImageLayout dst_layout, const VkImageBlit &blit,
                                               VkFilter filter) const {
    vkCmdBlitImage(m_cmd_buf, src_image, src_layout, dst_image, dst_layout, 1, &blit, filter);
    return *this;
}

const CommandBuffer &CommandBuffer::insert_debug_label(const std::string &name, std::array<float, 4> color) const {
    if (name.empty()) {
        throw InexorException("Error: Parameter 'name' is an empty string!");
    }
    auto label = make_info<VkDebugUtilsLabelEXT>({
        .pLabelName = name.c_str(),
        .color = {color[0], color[1], color[2], color[3]},
    });
    vkCmdInsertDebugUtilsLabelEXT(m_cmd_buf, &label);
    return *this;
}

const CommandBuffer &CommandBuffer::push_constants(const VkPipelineLayout layout, const VkShaderStageFlags stage,
                                                   const std::uint32_t size, const void *data,
                                                   const VkDeviceSize offset) const {
    if (!layout) {
        throw std::invalid_argument("Error: Parameter 'layout' is invalid!");
    }
    if (size == 0) {
        throw std::invalid_argument("Error: Parameter 'size' must be greater than zero!");
    }
    if (data == nullptr) {
        throw std::invalid_argument("Error: Parameter 'data' is invalid!");
    }
    vkCmdPushConstants(m_cmd_buf, layout, stage, static_cast<std::uint32_t>(offset), size, data);
    return *this;
}

const CommandBuffer &CommandBuffer::set_scissor(const VkRect2D scissor) const {
    vkCmdSetScissor(m_cmd_buf, 0, 1, &scissor);
    return *this;
}

const CommandBuffer &CommandBuffer::set_viewport(const VkViewport viewport) const {
    vkCmdSetViewport(m_cmd_buf, 0, 1, &viewport);
    return *this;
}

void CommandBuffer::submit(const VkQueueFlagBits queue_type,
                           const std::span<const VkSemaphoreSubmitInfo> wait_semaphore_infos,
                           const std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos) const {
    const auto command_buffer_info = VkCommandBufferSubmitInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = m_cmd_buf,
        .deviceMask = 0,
    };

    const auto submit_info = VkSubmitInfo2{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = static_cast<std::uint32_t>(wait_semaphore_infos.size()),
        .pWaitSemaphoreInfos = wait_semaphore_infos.empty() ? nullptr : wait_semaphore_infos.data(),
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &command_buffer_info,
        .signalSemaphoreInfoCount = static_cast<std::uint32_t>(signal_semaphore_infos.size()),
        .pSignalSemaphoreInfos = signal_semaphore_infos.empty() ? nullptr : signal_semaphore_infos.data(),
    };

    auto get_queue = [&]() {
        switch (queue_type) {
        case VK_QUEUE_TRANSFER_BIT: {
            return m_device.m_transfer_queue;
        }
        case VK_QUEUE_COMPUTE_BIT: {
            return m_device.m_compute_queue;
        }
        case VK_QUEUE_SPARSE_BINDING_BIT: {
            return m_device.m_sparse_binding_queue;
        }
        default: {
            return m_device.m_graphics_queue;
        }
        }
    };

    if (const auto result = vkQueueSubmit2(get_queue(), 1, &submit_info, m_wait_fence->fence())) {
        throw VulkanException("Error: vkQueueSubmit2 failed!", result, m_name);
    }
    m_has_been_submitted = true;
}

void CommandBuffer::submit(const VkQueueFlagBits queue_type,
                           const std::span<const VkSemaphoreSubmitInfo> wait_semaphore_infos,
                           const std::span<const VkSemaphore> signal_semaphores) const {
    const auto stage_mask = [&]() -> VkPipelineStageFlags2 {
        switch (queue_type) {
        case VK_QUEUE_TRANSFER_BIT:
            return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        case VK_QUEUE_COMPUTE_BIT:
            return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        case VK_QUEUE_SPARSE_BINDING_BIT:
            return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        default:
            return VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        }
    }();

    m_signal_submit_infos_scratch.resize(signal_semaphores.size());
    for (std::size_t index = 0; index < signal_semaphores.size(); ++index) {
        m_signal_submit_infos_scratch[index] = VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = signal_semaphores[index],
            .stageMask = stage_mask,
        };
    }

    submit(queue_type, wait_semaphore_infos, m_signal_submit_infos_scratch);
}

void CommandBuffer::submit(const VkQueueFlagBits queue_type, const std::span<const VkSemaphore> wait_semaphores,
                           const std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos) const {
    const auto default_wait_stage_mask = [&]() -> VkPipelineStageFlags2 {
        switch (queue_type) {
        case VK_QUEUE_TRANSFER_BIT:
            return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        case VK_QUEUE_COMPUTE_BIT:
            return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        case VK_QUEUE_SPARSE_BINDING_BIT:
            return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        default:
            return VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        }
    }();

    m_wait_submit_infos_scratch.resize(wait_semaphores.size());
    for (std::size_t index = 0; index < wait_semaphores.size(); ++index) {
        m_wait_submit_infos_scratch[index] = VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = wait_semaphores[index],
            .stageMask = default_wait_stage_mask,
        };
    }

    submit(queue_type, m_wait_submit_infos_scratch, signal_semaphore_infos);
}

void CommandBuffer::submit(const VkQueueFlagBits queue_type, const std::span<const VkSemaphore> wait_semaphores,
                           const std::span<const VkSemaphore> signal_semaphores) const {
    const auto default_wait_stage_mask = [&]() -> VkPipelineStageFlags2 {
        switch (queue_type) {
        case VK_QUEUE_TRANSFER_BIT:
            return VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        case VK_QUEUE_COMPUTE_BIT:
            return VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
        case VK_QUEUE_SPARSE_BINDING_BIT:
            return VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        default:
            return VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
        }
    }();

    m_wait_submit_infos_scratch.resize(wait_semaphores.size());
    for (std::size_t index = 0; index < wait_semaphores.size(); ++index) {
        m_wait_submit_infos_scratch[index] = VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = wait_semaphores[index],
            .stageMask = default_wait_stage_mask,
        };
    }

    submit(queue_type, m_wait_submit_infos_scratch, signal_semaphores);
}

void CommandBuffer::submit(const VkQueueFlagBits queue_type,
                           const std::span<const core::QueueSemaphoreWait> wait_semaphores,
                           const std::span<const VkSemaphore> signal_semaphores) const {
    m_wait_submit_infos_scratch.resize(wait_semaphores.size());
    for (std::size_t index = 0; index < wait_semaphores.size(); ++index) {
        const auto &wait_semaphore = wait_semaphores[index];
        m_wait_submit_infos_scratch[index] = VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = wait_semaphore.semaphore,
            .stageMask = wait_semaphore.stage_mask,
        };
    }

    submit(queue_type, m_wait_submit_infos_scratch, signal_semaphores);
}

void CommandBuffer::submit(const VkQueueFlagBits queue_type,
                           const std::span<const core::QueueSemaphoreWait> wait_semaphores,
                           const std::span<const VkSemaphoreSubmitInfo> signal_semaphore_infos) const {
    m_wait_submit_infos_scratch.resize(wait_semaphores.size());
    for (std::size_t index = 0; index < wait_semaphores.size(); ++index) {
        const auto &wait_semaphore = wait_semaphores[index];
        m_wait_submit_infos_scratch[index] = VkSemaphoreSubmitInfo{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = wait_semaphore.semaphore,
            .stageMask = wait_semaphore.stage_mask,
        };
    }

    submit(queue_type, m_wait_submit_infos_scratch, signal_semaphore_infos);
}

void CommandBuffer::set_debug_name(const std::string &name) const {
    m_name = name;
    m_device.set_debug_name(m_cmd_buf, name);
}

void CommandBuffer::reset() const {
    vkResetCommandBuffer(m_cmd_buf, 0);
}

void CommandBuffer::reset_fence() const {
    m_wait_fence->reset();
}

VkResult CommandBuffer::status() const {
    return m_wait_fence->status();
}

void CommandBuffer::wait_fence() const {
    m_wait_fence->wait();
}

} // namespace inexor::vulkan_renderer::wrapper::commands
