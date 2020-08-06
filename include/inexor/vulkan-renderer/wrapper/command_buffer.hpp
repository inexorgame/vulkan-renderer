#pragma once

// TODO(): Forward declare
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class ResourceDescriptor;

// TODO(): Make trivially copyable (this class doesn't really "own" the command buffer, more just an OOP wrapper)
class CommandBuffer {

private:
    VkCommandBuffer m_command_buffer{VK_NULL_HANDLE};

public:
    CommandBuffer(VkDevice device, VkCommandPool command_pool);
    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) noexcept;
    ~CommandBuffer() = default;

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) = default;

    // General commands
    void begin(VkCommandBufferUsageFlags flags = 0) const;
    void bind_descriptor(const ResourceDescriptor &descriptor, VkPipelineLayout layout) const;
    void end() const;

    // Graphics commands
    // TODO(): Switch to taking in OOP wrappers when we have them (e.g. bind_vertex_buffers takes in a VertexBuffer)
    void begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi) const;
    void bind_graphics_pipeline(VkPipeline pipeline) const;
    void bind_vertex_buffers(const std::vector<VkBuffer> &buffers) const;
    void draw(std::size_t vertex_count) const;
    void end_render_pass() const;

    [[nodiscard]] VkCommandBuffer get() const {
        return m_command_buffer;
    }

    [[nodiscard]] const VkCommandBuffer *ptr() const {
        return &m_command_buffer;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
