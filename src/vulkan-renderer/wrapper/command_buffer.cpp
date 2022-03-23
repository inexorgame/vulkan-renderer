#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

CommandBuffer::CommandBuffer(const Device &device) : m_device(device) {}

void CommandBuffer::create_command_buffer(const VkCommandPool command_pool) {
    auto alloc_info = make_info<VkCommandBufferAllocateInfo>();
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    // TODO: Move to device wrapper!
    if (const auto result = vkAllocateCommandBuffers(m_device.device(), &alloc_info, &m_command_buffer);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to allocate command buffer!", result);
    }

    m_device.set_debug_marker_name(m_command_buffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, m_name);
}

CommandBuffer::CommandBuffer(const wrapper::Device &device, const VkCommandPool command_pool, std::string name)
    : m_device(device), m_name(std::move(name)) {
    create_command_buffer(command_pool);
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept : m_device(other.m_device) {
    m_command_buffer = std::exchange(other.m_command_buffer, nullptr);
    m_name = std::move(other.m_name);
}

void CommandBuffer::begin(const VkCommandBufferUsageFlags flags) const {
    auto begin_info = make_info<VkCommandBufferBeginInfo>();
    begin_info.flags = flags;

    if (const auto result = vkBeginCommandBuffer(m_command_buffer, &begin_info); result != VK_SUCCESS) {
        throw VulkanException("Error: vkBeginCommandBuffer failed!", result);
    }
}

// TODO: Expose more parameters
void CommandBuffer::bind_descriptor_set(const VkDescriptorSet descriptor_set, const VkPipelineLayout layout,
                                        const VkPipelineBindPoint bind_point) const {
    assert(descriptor_set);
    assert(layout);

    vkCmdBindDescriptorSets(m_command_buffer, bind_point, layout, 0, 1, &descriptor_set, 0, nullptr);
}

// TODO: Expose more parameters
void CommandBuffer::bind_descriptor_sets(const std::vector<VkDescriptorSet> &descriptor_sets,
                                         const VkPipelineLayout layout, const VkPipelineBindPoint bind_point) const {
    assert(!descriptor_sets.empty());
    assert(layout);

    vkCmdBindDescriptorSets(m_command_buffer, bind_point, layout, 0, static_cast<std::uint32_t>(descriptor_sets.size()),
                            descriptor_sets.data(), 0, nullptr);
}

void CommandBuffer::end() const {
    if (const auto result = vkEndCommandBuffer(m_command_buffer); result != VK_SUCCESS) {
        throw VulkanException("Error: VkEndCommandBuffer failed!", result);
    }
}

// TODO: Support multiple copies!
void CommandBuffer::copy_buffer(const VkBuffer source_buffer, const VkBuffer target_buffer,
                                const VkBufferCopy &copy_region) const {
    assert(source_buffer);
    assert(target_buffer);

    vkCmdCopyBuffer(m_command_buffer, source_buffer, target_buffer, 1, &copy_region);
}

void CommandBuffer::copy_buffer(const VkBuffer source_buffer, const VkBuffer target_buffer,
                                const VkDeviceSize source_buffer_size) const {
    assert(source_buffer);
    assert(target_buffer);
    assert(source_buffer_size > 0);

    VkBufferCopy copy_region{};
    copy_region.srcOffset = 0;
    copy_region.dstOffset = 0;
    copy_region.size = source_buffer_size;

    vkCmdCopyBuffer(m_command_buffer, source_buffer, target_buffer, 1, &copy_region);
}

void CommandBuffer::copy_buffer_to_image(const VkBuffer src_buffer, const VkImage target_image,
                                         const VkBufferImageCopy &regions) const {
    assert(src_buffer);
    assert(target_image);

    vkCmdCopyBufferToImage(m_command_buffer, src_buffer, target_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1,
                           &regions);
}

void CommandBuffer::copy_buffer_to_image(const VkBuffer src_buffer, const VkImage target_image,
                                         const std::vector<VkBufferImageCopy> &regions) const {
    assert(src_buffer);
    assert(target_image);
    assert(!regions.empty());

    vkCmdCopyBufferToImage(m_command_buffer, src_buffer, target_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           static_cast<std::uint32_t>(regions.size()), regions.data());
}

// TODO: Support multiple copy regions!
void CommandBuffer::copy_image(const VkImage source_image, const VkImage target_image,
                               const VkImageCopy &copy_region) const {
    vkCmdCopyImage(m_command_buffer, source_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, target_image,
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);
}

// TODO: Support multiple scissors!
void CommandBuffer::set_scissor(const VkRect2D &scissor) const {
    vkCmdSetScissor(m_command_buffer, 0, 1, &scissor);
}

void CommandBuffer::set_scissor(const std::uint32_t width, const std::uint32_t height) const {
    VkRect2D scissor{};
    scissor.extent.width = width;
    scissor.extent.height = height;
    set_scissor(scissor);
}

// TODO: Support multiple viewports!
void CommandBuffer::set_viewport(const VkViewport &viewport) const {
    vkCmdSetViewport(m_command_buffer, 0, 1, &viewport);
}

void CommandBuffer::set_viewport(const std::uint32_t width, const std::uint32_t height, const float min_depth,
                                 const float max_depth) const {
    VkViewport viewport{};
    viewport.width = static_cast<float>(width);
    viewport.height = static_cast<float>(height);
    viewport.minDepth = min_depth;
    viewport.maxDepth = max_depth;
    set_viewport(viewport);
}

// TODO: Support multiple pipeline barriers!
void CommandBuffer::pipeline_barrier(const VkPipelineStageFlags source_stage_flags,
                                     const VkPipelineStageFlags destination_stage_flags,
                                     const VkImageMemoryBarrier &barrier) const {
    vkCmdPipelineBarrier(m_command_buffer, source_stage_flags, destination_stage_flags, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);
}

// TODO: Expose more parameters
void CommandBuffer::begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi) const {
    vkCmdBeginRenderPass(m_command_buffer, &render_pass_bi, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::bind_graphics_pipeline(const VkPipeline pipeline, const VkPipelineBindPoint bind_point) const {
    assert(pipeline);
    vkCmdBindPipeline(m_command_buffer, bind_point, pipeline);
}

void CommandBuffer::bind_index_buffer(const VkBuffer buffer, const VkIndexType index_type,
                                      const std::uint32_t offset) const {
    assert(buffer);
    vkCmdBindIndexBuffer(m_command_buffer, buffer, offset, index_type);
}

void CommandBuffer::bind_vertex_buffer(const VkBuffer buffer) const {
    assert(buffer);
    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(m_command_buffer, 0, 1, &buffer, offsets);
}

void CommandBuffer::bind_vertex_buffers(const std::vector<VkBuffer> &buffers) const {
    assert(!buffers.empty());
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);
    vkCmdBindVertexBuffers(m_command_buffer, 0, static_cast<std::uint32_t>(buffers.size()), buffers.data(),
                           offsets.data());
}

void CommandBuffer::draw(const std::size_t vertex_count, const std::uint32_t first_vertex,
                         const std::uint32_t instance_count, const std::uint32_t first_instance) const {
    vkCmdDraw(m_command_buffer, static_cast<std::uint32_t>(vertex_count), instance_count, first_vertex, first_instance);
}

void CommandBuffer::draw_indexed(const std::size_t index_count, const std::uint32_t first_index,
                                 const std::uint32_t vertex_offset, const std::uint32_t instance_count,
                                 const std::uint32_t first_instance) const {
    vkCmdDrawIndexed(m_command_buffer, static_cast<std::uint32_t>(index_count), instance_count, first_index,
                     vertex_offset, first_instance);
}

void CommandBuffer::end_render_pass() const {
    vkCmdEndRenderPass(m_command_buffer);
}

} // namespace inexor::vulkan_renderer::wrapper
