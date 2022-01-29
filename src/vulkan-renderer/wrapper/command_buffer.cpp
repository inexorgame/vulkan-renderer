#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

CommandBuffer::CommandBuffer(const wrapper::Device &device, VkCommandPool command_pool, std::string name)
    : m_device(device), m_name(std::move(name)) {
    auto alloc_info = make_info<VkCommandBufferAllocateInfo>();
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (const auto result = vkAllocateCommandBuffers(device.device(), &alloc_info, &m_cmd_buf); result != VK_SUCCESS) {
        throw VulkanException("Failed to allocate command buffer!", result);
    }

    m_device.set_debug_marker_name(m_cmd_buf, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, m_name);
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept : m_device(other.m_device) {
    m_cmd_buf = std::exchange(other.m_cmd_buf, nullptr);
    m_name = std::move(other.m_name);
}

void CommandBuffer::begin(const VkCommandBufferUsageFlags flags) const {
    auto begin_info = make_info<VkCommandBufferBeginInfo>();
    begin_info.flags = flags;
    vkBeginCommandBuffer(m_cmd_buf, &begin_info);
}

void CommandBuffer::bind_descriptor_set(const VkDescriptorSet descriptor_set, const VkPipelineLayout layout,
                                        const VkPipelineBindPoint bind_point) const {
    assert(descriptor_set);
    assert(layout);

    vkCmdBindDescriptorSets(m_cmd_buf, bind_point, layout, 0, 1, &descriptor_set, 0, nullptr);
}

void CommandBuffer::bind_descriptor_sets(const std::vector<VkDescriptorSet> &descriptor_sets,
                                         const VkPipelineLayout layout, const VkPipelineBindPoint bind_point) const {
    assert(!descriptor_sets.empty());
    assert(layout);

    vkCmdBindDescriptorSets(m_cmd_buf, bind_point, layout, 0, static_cast<std::uint32_t>(descriptor_sets.size()),
                            descriptor_sets.data(), 0, nullptr);
}
void CommandBuffer::end() const {
    vkEndCommandBuffer(m_cmd_buf);
}

void CommandBuffer::begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi) const {
    vkCmdBeginRenderPass(m_cmd_buf, &render_pass_bi, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::bind_graphics_pipeline(const VkPipeline pipeline) const {
    vkCmdBindPipeline(m_cmd_buf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void CommandBuffer::bind_index_buffer(const VkBuffer buffer) const {
    vkCmdBindIndexBuffer(m_cmd_buf, buffer, 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::bind_vertex_buffers(const std::vector<VkBuffer> &buffers) const {
    assert(!buffers.empty());
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);
    vkCmdBindVertexBuffers(m_cmd_buf, 0, static_cast<std::uint32_t>(buffers.size()), buffers.data(), offsets.data());
}

void CommandBuffer::draw(const std::size_t vertex_count) const {
    vkCmdDraw(m_cmd_buf, static_cast<std::uint32_t>(vertex_count), 1, 0, 0);
}

void CommandBuffer::draw_indexed(const std::size_t index_count, const std::uint32_t first_index) const {
    vkCmdDrawIndexed(m_cmd_buf, static_cast<std::uint32_t>(index_count), 1, first_index, 0, 0);
}

void CommandBuffer::end_render_pass() const {
    vkCmdEndRenderPass(m_cmd_buf);
}

} // namespace inexor::vulkan_renderer::wrapper
