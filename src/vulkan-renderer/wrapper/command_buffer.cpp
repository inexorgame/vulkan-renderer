#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
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

    if (const auto result = vkAllocateCommandBuffers(device.device(), &alloc_info, &m_command_buffer);
        result != VK_SUCCESS) {
        throw VulkanException("Failed to allocate command buffer!", result);
    }

    // Assign an internal name using Vulkan debug markers.
    m_device.set_debug_marker_name(m_command_buffer, VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, m_name);
}

CommandBuffer::CommandBuffer(CommandBuffer &&other) noexcept : m_device(other.m_device) {
    m_command_buffer = std::exchange(other.m_command_buffer, nullptr);
    m_name = std::move(other.m_name);
}

const CommandBuffer &CommandBuffer::begin_command_buffer(const VkCommandBufferUsageFlags flags) const {
    auto begin_info = make_info<VkCommandBufferBeginInfo>();
    begin_info.flags = flags;
    vkBeginCommandBuffer(m_command_buffer, &begin_info);
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

void CommandBuffer::push_constants(VkPipelineLayout layout, VkShaderStageFlags stage, std::uint32_t size,
                                   void *data) const {
    vkCmdPushConstants(m_command_buffer, layout, stage, 0, size, data);
}

void CommandBuffer::end() const {
    vkEndCommandBuffer(m_command_buffer);
}

void CommandBuffer::bind_graphics_pipeline(VkPipeline pipeline) const {
    vkCmdBindPipeline(m_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void CommandBuffer::bind_index_buffer(VkBuffer buffer) const {
    vkCmdBindIndexBuffer(m_command_buffer, buffer, 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::bind_vertex_buffers(const std::vector<VkBuffer> &buffers) const {
    std::vector<VkDeviceSize> offsets(buffers.size(), 0);
    vkCmdBindVertexBuffers(m_command_buffer, 0, static_cast<std::uint32_t>(buffers.size()), buffers.data(),
                           offsets.data());
}

void CommandBuffer::draw(std::size_t vertex_count) const {
    vkCmdDraw(m_command_buffer, static_cast<std::uint32_t>(vertex_count), 1, 0, 0);
}

void CommandBuffer::draw_indexed(std::size_t index_count) const {
    vkCmdDrawIndexed(m_command_buffer, static_cast<std::uint32_t>(index_count), 1, 0, 0, 0);
}

void CommandBuffer::end_render_pass() const {
    vkCmdEndRenderPass(m_command_buffer);
}

} // namespace inexor::vulkan_renderer::wrapper
