#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/fence.hpp"
#include "inexor/vulkan-renderer/wrapper/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipeline_layout.hpp"

#include <cassert>
#include <limits>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

CommandBuffer::CommandBuffer(const Device &device, std::string name) : m_device(device), m_name(name) {}

void CommandBuffer::create_command_buffer(const VkCommandPool command_pool) {
    auto alloc_info = make_info<VkCommandBufferAllocateInfo>();
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    // TODO: Move to device wrapper!
    if (const auto result = vkAllocateCommandBuffers(m_device.device(), &alloc_info, &m_cmd_buf);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to allocate command buffer!", result);
    }

    m_device.set_debug_marker_name(m_cmd_buf, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, m_name);
}

CommandBuffer::CommandBuffer(const wrapper::Device &device, const VkCommandPool command_pool, std::string name)
    : m_device(device), m_name(std::move(name)) {
    create_command_buffer(command_pool);
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept : m_device(other.m_device) {
    m_cmd_buf = std::exchange(other.m_cmd_buf, nullptr);
    m_name = std::move(other.m_name);
}

const CommandBuffer &CommandBuffer::begin_command_buffer(const VkCommandBufferUsageFlags flags) const {
    auto begin_info = make_info<VkCommandBufferBeginInfo>();
    begin_info.flags = flags;

    if (const auto result = vkBeginCommandBuffer(m_cmd_buf, &begin_info); result != VK_SUCCESS) {
        throw VulkanException("Error: vkBeginCommandBuffer failed!", result);
    }
    return *this;
}

const CommandBuffer &CommandBuffer::end_command_buffer() const {
    if (const auto result = vkEndCommandBuffer(m_cmd_buf); result != VK_SUCCESS) {
        throw VulkanException("Error: VkEndCommandBuffer failed!", result);
    }
    return *this;
}

// TODO: Expose more parameters
const CommandBuffer &CommandBuffer::bind_descriptor_set(const VkDescriptorSet descriptor_set,
                                                        const VkPipelineLayout layout,
                                                        const VkPipelineBindPoint bind_point) const {
    assert(descriptor_set);
    assert(layout);

    vkCmdBindDescriptorSets(m_cmd_buf, bind_point, layout, 0, 1, &descriptor_set, 0, nullptr);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_descriptor_set(const VkDescriptorSet descriptor_set,
                                                        const PipelineLayout &layout,
                                                        const VkPipelineBindPoint bind_point) const {
    return bind_descriptor_set(descriptor_set, layout.pipeline_layout(), bind_point);
}

// TODO: Expose more parameters
const CommandBuffer &CommandBuffer::bind_descriptor_sets(const std::vector<VkDescriptorSet> &descriptor_sets,
                                                         const VkPipelineLayout layout,
                                                         const VkPipelineBindPoint bind_point) const {
    assert(!descriptor_sets.empty());
    assert(layout);

    vkCmdBindDescriptorSets(m_cmd_buf, bind_point, layout, 0, static_cast<std::uint32_t>(descriptor_sets.size()),
                            descriptor_sets.data(), 0, nullptr);
    return *this;
}

const CommandBuffer &CommandBuffer::change_image_layout(const VkImage image, const VkImageLayout old_layout,
                                                        const VkImageLayout new_layout,
                                                        const VkImageSubresourceRange subresource_range,
                                                        const VkPipelineStageFlags src_mask,
                                                        const VkPipelineStageFlags dst_mask) const {
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.oldLayout = old_layout;
    barrier.newLayout = new_layout;
    barrier.image = image;
    barrier.subresourceRange = subresource_range;

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

    vkCmdPipelineBarrier(m_cmd_buf, src_mask, dst_mask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    return *this;
}

const CommandBuffer &
CommandBuffer::change_image_layout(const VkImage image, const VkImageLayout old_layout, const VkImageLayout new_layout,
                                   const std::uint32_t mip_level_count, const std::uint32_t array_layer_count,
                                   const std::uint32_t base_mip_level, const std::uint32_t base_array_layer,
                                   const VkPipelineStageFlags src_mask, const VkPipelineStageFlags dst_mask) const {

    VkImageSubresourceRange subres_range{};
    subres_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subres_range.baseArrayLayer = base_array_layer;
    subres_range.baseMipLevel = base_mip_level;
    subres_range.layerCount = array_layer_count;
    subres_range.levelCount = mip_level_count;

    return change_image_layout(image, old_layout, new_layout, subres_range, src_mask, dst_mask);
}

// TODO: Support multiple copies!
const CommandBuffer &CommandBuffer::copy_buffer(const VkBuffer source_buffer, const VkBuffer target_buffer,
                                                const VkBufferCopy &copy_region) const {
    assert(source_buffer);
    assert(target_buffer);

    vkCmdCopyBuffer(m_cmd_buf, source_buffer, target_buffer, 1, &copy_region);
    return *this;
}

const CommandBuffer &CommandBuffer::copy_buffer(const VkBuffer source_buffer, const VkBuffer target_buffer,
                                                const VkDeviceSize source_buffer_size) const {
    assert(source_buffer);
    assert(target_buffer);
    assert(source_buffer_size > 0);

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = source_buffer_size;

    vkCmdCopyBuffer(m_cmd_buf, source_buffer, target_buffer, 1, &copy_region);
    return *this;
}

// TODO: Support multiple copies!
const CommandBuffer &CommandBuffer::copy_buffer_to_image(const VkBuffer src_buffer, const VkImage target_image,
                                                         const VkBufferImageCopy &regions) const {
    assert(src_buffer);
    assert(target_image);

    vkCmdCopyBufferToImage(m_cmd_buf, src_buffer, target_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &regions);
    return *this;
}

const CommandBuffer &CommandBuffer::copy_buffer_to_image(const VkBuffer src_buffer, const VkImage target_image,
                                                         const std::vector<VkBufferImageCopy> &regions) const {
    assert(src_buffer);
    assert(target_image);
    assert(!regions.empty());

    vkCmdCopyBufferToImage(m_cmd_buf, src_buffer, target_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<std::uint32_t>(regions.size()), regions.data());
    return *this;
}

// TODO: Support multiple copy regions!
const CommandBuffer &CommandBuffer::copy_image(const VkImage source_image, const VkImage target_image,
                                               const VkImageCopy &copy_region) const {
    assert(source_image);
    assert(target_image);

    vkCmdCopyImage(m_cmd_buf, source_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target_image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
    return *this;
}

// TODO: Support multiple scissors!
const CommandBuffer &CommandBuffer::set_scissor(const VkRect2D &scissor) const {
    vkCmdSetScissor(m_cmd_buf, 0, 1, &scissor);
    return *this;
}

const CommandBuffer &CommandBuffer::set_scissor(const std::uint32_t width, const std::uint32_t height) const {
    VkRect2D scissor{};
    scissor.extent.width = width;
    scissor.extent.height = height;
    return set_scissor(scissor);
}

// TODO: Support multiple viewports!
const CommandBuffer &CommandBuffer::set_viewport(const VkViewport &viewport) const {
    vkCmdSetViewport(m_cmd_buf, 0, 1, &viewport);
    return *this;
}

const CommandBuffer &CommandBuffer::set_viewport(const std::uint32_t width, const std::uint32_t height,
                                                 const float min_depth, const float max_depth) const {
    VkViewport viewport{};
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = min_depth;
    viewport.maxDepth = max_depth;
    return set_viewport(viewport);
}

// TODO: Support multiple pipeline barriers!
const CommandBuffer &CommandBuffer::pipeline_barrier(const VkPipelineStageFlags source_stage_flags,
                                                     const VkPipelineStageFlags destination_stage_flags,
                                                     const VkImageMemoryBarrier &barrier) const {
    vkCmdPipelineBarrier(m_cmd_buf, source_stage_flags, destination_stage_flags, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);
    return *this;
}

const CommandBuffer &CommandBuffer::pipeline_barrier(const VkImageMemoryBarrier &barrier) const {
    return pipeline_barrier(VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, barrier);
}

const CommandBuffer &CommandBuffer::begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi,
                                                      const VkSubpassContents subpass_contents) const {
    vkCmdBeginRenderPass(m_cmd_buf, &render_pass_bi, subpass_contents);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_graphics_pipeline(const VkPipeline pipeline) const {
    assert(pipeline);
    vkCmdBindPipeline(m_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_index_buffer(const VkBuffer buffer, const VkIndexType index_type,
                                                      const std::uint32_t offset) const {
    assert(buffer);
    vkCmdBindIndexBuffer(m_cmd_buf, buffer, offset, index_type);
    return *this;
}

const CommandBuffer &CommandBuffer::bind_vertex_buffer(const VkBuffer buffer) const {
    assert(buffer);
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(m_cmd_buf, 0, 1, &buffer, offsets);
    return *this;
}

// TODO: Expose more parameters and use C++20 std::span!
const CommandBuffer &CommandBuffer::bind_vertex_buffers(const std::vector<VkBuffer> &buffers) const {
    assert(!buffers.empty());

    std::vector<VkDeviceSize> offsets(buffers.size(), 0);
    vkCmdBindVertexBuffers(m_cmd_buf, 0, static_cast<std::uint32_t>(buffers.size()), buffers.data(), offsets.data());
    return *this;
}

const CommandBuffer &CommandBuffer::draw(const std::size_t vertex_count, const std::uint32_t first_vertex,
                                         const std::uint32_t instance_count, const std::uint32_t first_instance) const {
    vkCmdDraw(m_cmd_buf, static_cast<std::uint32_t>(vertex_count), instance_count, first_vertex, first_instance);
    return *this;
}

const CommandBuffer &CommandBuffer::draw_indexed(const std::size_t index_count, const std::uint32_t first_index,
                                                 const std::uint32_t vertex_offset, const std::uint32_t instance_count,
                                                 const std::uint32_t first_instance) const {
    vkCmdDrawIndexed(m_cmd_buf, static_cast<std::uint32_t>(index_count), instance_count, first_index, vertex_offset,
                     first_instance);
    return *this;
}

const CommandBuffer &CommandBuffer::end_render_pass() const {
    vkCmdEndRenderPass(m_cmd_buf);
    return *this;
}

const CommandBuffer &CommandBuffer::flush_command_buffer_and_wait() const {
    // We do not check if command buffers are in recording state (validation layers should check for this)
    end_command_buffer();

    // Execute the command buffer and wait for it to finish
    Fence fence(m_device, "command buffer flush",
                [&](const VkFence wait_fence) { m_device.queue_submit(m_cmd_buf, wait_fence); });

    return *this;
}

// TODO: Use C++20 and use std::span!
const CommandBuffer &CommandBuffer::free_command_buffer(const VkCommandPool cmd_pool) const {
    vkFreeCommandBuffers(m_device.device(), cmd_pool, 1, &m_cmd_buf);
    return *this;
}

} // namespace inexor::vulkan_renderer::wrapper
