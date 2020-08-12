#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"

#include "inexor/vulkan-renderer/wrapper/info.hpp"
#include "inexor/vulkan-renderer/wrapper/resource_descriptor.hpp"

#include <array>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer::wrapper {

CommandBuffer::CommandBuffer(wrapper::Device &device, VkCommandPool command_pool, const std::string &name)
    : m_device(device), m_name(name) {
    auto alloc_info = make_info<VkCommandBufferAllocateInfo>();
    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool = command_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    if (vkAllocateCommandBuffers(device.device(), &alloc_info, &m_command_buffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffer!");
    }

#ifndef NDEBUG
    // Assign an internal name using Vulkan debug markers.
    m_device.set_object_name(m_command_buffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, m_name);
#endif
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept
    : m_command_buffer(std::exchange(other.m_command_buffer, nullptr)), m_device(other.m_device), m_name(other.m_name) {
}

void CommandBuffer::begin(VkCommandBufferUsageFlags flags) const {
    auto begin_info = make_info<VkCommandBufferBeginInfo>();
    begin_info.flags = flags;
    vkBeginCommandBuffer(m_command_buffer, &begin_info);
}

void CommandBuffer::bind_descriptor(const ResourceDescriptor &descriptor, VkPipelineLayout layout) const {
    vkCmdBindDescriptorSets(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1,
                            descriptor.descriptor_sets().data(), 0, nullptr);
}

void CommandBuffer::end() const {
    vkEndCommandBuffer(m_command_buffer);
}

void CommandBuffer::begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi) const {
    vkCmdBeginRenderPass(m_command_buffer, &render_pass_bi, VK_SUBPASS_CONTENTS_INLINE);
}

void CommandBuffer::bind_graphics_pipeline(VkPipeline pipeline) const {
    vkCmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void CommandBuffer::bind_vertex_buffers(const std::vector<VkBuffer> &buffers) const {
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);
    vkCmdBindVertexBuffers(m_command_buffer, 0, static_cast<std::uint32_t>(buffers.size()), buffers.data(),
                           offsets.data());
}

void CommandBuffer::draw(std::size_t vertex_count) const {
    vkCmdDraw(m_command_buffer, static_cast<std::uint32_t>(vertex_count), 1, 0, 0);
}

void CommandBuffer::end_render_pass() const {
    vkCmdEndRenderPass(m_command_buffer);
}

} // namespace inexor::vulkan_renderer::wrapper
