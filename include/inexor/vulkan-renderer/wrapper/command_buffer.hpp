#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;
class ResourceDescriptor;

/// @brief RAII wrapper class for VkCommandBuffer.
/// @todo Make trivially copyable (this class doesn't really "own" the command buffer, more just an OOP wrapper).
class CommandBuffer {
    VkCommandBuffer m_cmd_buf{VK_NULL_HANDLE};
    const wrapper::Device &m_device;
    std::string m_name;

public:
    /// @brief Default constructor.
    /// @param device The const reference to the device RAII wrapper class.
    /// @param command_pool The command pool from which the command buffer will be allocated.
    /// @param name The internal debug marker name of the command buffer. This must not be an empty string.
    CommandBuffer(const wrapper::Device &device, VkCommandPool command_pool, std::string name);

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) noexcept;
    ~CommandBuffer() = default;

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) = delete;

    /// @brief Call vkBeginCommandBuffer.
    /// @note Sometimes it's useful to pass VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT to specify that a command
    /// buffer can be resubmitted to a queue while it is in the pending state, and recorded into multiple primary
    /// command buffers. Otherwise, synchronization must be done using a VkFence.
    /// @param flags The command buffer usage flags, 0 by default.
    void begin(VkCommandBufferUsageFlags flags = 0) const;

    /// @brief Call vkCmdBindDescriptorSets.
    /// @param descriptor_set The const reference to the resource descriptor RAII wrapper instance
    /// @param layout The pipeline layout which will be used to bind the resource descriptor
    /// @param first_set The first set to use
    void bind_descriptor(VkDescriptorSet descriptor_set, VkPipelineLayout layout) const;

    /// @brief Update push constant data.
    /// @param layout The pipeline layout
    /// @param stage The shader stage that will be accepting the push constants
    /// @param size The size of the push constant data in bytes
    /// @param data A pointer to the push constant data
    template <typename T>
    void push_constants(const T *data, VkPipelineLayout layout,
                        VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT) const {
        assert(data);
        vkCmdPushConstants(m_cmd_buf, layout, stage, 0, sizeof(T), data);
    }

    template <typename T>
    void push_constants(const T &data, VkPipelineLayout pipeline_layout,
                        VkShaderStageFlags stage = VK_SHADER_STAGE_VERTEX_BIT) const {
        vkCmdPushConstants(m_cmd_buf, pipeline_layout, stage, 0, sizeof(T), &data);
    }

    /// @brief Call vkEndCommandBuffer.s
    void end() const;

    // Graphics commands
    // TODO(): Switch to taking in OOP wrappers when we have them (e.g. bind_vertex_buffers takes in a VertexBuffer)

    /// @brief Call vkCmdBeginRenderPass.
    /// @param render_pass_bi The const reference to the VkRenderPassBeginInfo which is used.
    void begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi) const;

    /// @brief Call vkCmdBindPipeline.
    /// @param pipeline The graphics pipeline to bind.
    void bind_graphics_pipeline(VkPipeline pipeline) const;

    /// @brief Call vkCmdBindIndexBuffer.
    /// @todo Don't hardcode 16 for index bit width here.
    /// @param buffer The index buffer to bind.
    void bind_index_buffer(VkBuffer buffer) const;

    /// @brief Call vkCmdBindVertexBuffers.
    /// @param buffers A std::vector of vertex buffers to bind.
    /// @todo Expose more parameters from vkCmdBindVertexBuffers as method arguments.
    void bind_vertex_buffers(const std::vector<VkBuffer> &buffers) const;

    /// @brief Call vkCmdDraw.
    /// @param vertex_count The number of vertices to draw.
    void draw(std::size_t vertex_count) const;

    /// @brief Call vkCmdDrawIndexed.
    /// @param index_count The number of indices to draw.
    void draw_indexed(std::size_t index_count, std::uint32_t first_index = 0) const;

    /// @brief Call vkCmdEndRenderPass.
    void end_render_pass() const;

    // TODO: Refactor: unified get syntax!
    [[nodiscard]] VkCommandBuffer get() const {
        return m_cmd_buf;
    }

    // TODO: Refactor: unified get syntax!
    [[nodiscard]] const VkCommandBuffer *ptr() const {
        return &m_cmd_buf;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
