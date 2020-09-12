#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;
class ResourceDescriptor;

/// @class CommandBuffer
/// @brief RAII wrapper class for VkCommandBuffer.
/// @todo Make trivially copyable (this class doesn't really "own" the command buffer, more just an OOP wrapper)
class CommandBuffer {
    VkCommandBuffer m_command_buffer{VK_NULL_HANDLE};
    const wrapper::Device &m_device;
    const std::string m_name;

public:
    /// @brief Default constructor.
    /// @param device [in] The const reference to a device RAII wrapper instance.
    /// @param command_pool [in] The command pool from which the command buffer will be allocated.
    /// @param name [in] The internal debug marker name of the VkCommandBuffer.
    CommandBuffer(const Device &device, VkCommandPool command_pool, const std::string &name);
    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) noexcept;
    ~CommandBuffer() = default;

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) = default;

    /// General commands

    /// @brief Calls vkBeginCommandBuffer.
    /// @param flags [in] The command buffer usage flags, 0 by default.
    /// @note If a command buffer is required to be used in several
    /// @note Sometimes it's useful to pass VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT to specify that a command
    /// buffer can be resubmitted to a queue while it is in the pending state, and recorded into multiple primary
    /// command buffers. Otherwise, synchronization must be done using a VkFence.
    void begin(VkCommandBufferUsageFlags flags = 0) const;

    /// @brief Calls vkCmdBindDescriptorSets to bind resource descriptors to the command buffers.
    /// @param descriptor [in] The const reference to a resource descriptor RAII wrapper instance.
    /// @param layout [in] The pipeline layout which is used to bind the resource descriptors.
    void bind_descriptor(const ResourceDescriptor &descriptor, VkPipelineLayout layout) const;

    /// @brief Calls vkEndCommandBuffer.
    void end() const;

    // Graphics commands
    // TODO(): Switch to taking in OOP wrappers when we have them (e.g. bind_vertex_buffers takes in a VertexBuffer)

    /// @brief Calls vkCmdBeginRenderPass.
    /// @param render_pass_bi [in] The const reference to the VkRenderPassBeginInfo which is used.
    void begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi) const;

    /// @brief Calls vkCmdBindPipeline.
    /// @param pipeline [in] The pipeline which will be bound.
    /// @todo Add overloaded method which accepts a GraphicsPipeline reference.
    void bind_graphics_pipeline(VkPipeline pipeline) const;

    /// @brief Calls vkCmdBindIndexBuffer.
    /// @param buffer [in] The vertex buffer which will be bound.
    /// @todo Add overloaded method which accepts a MeshBuffer reference.
    void bind_index_buffer(VkBuffer buffer) const;

    /// @brief Calls vkCmdBindVertexBuffers on multiple vertex buffers.
    /// @param buffers [in] The std::vector of vertex bufers which will be bound.
    /// @todo Add overloaded method which accepts a MeshBuffer reference.
    void bind_vertex_buffers(const std::vector<VkBuffer> &buffers) const;

    /// @brief Calls vkCmdDraw.
    /// @param vertex_count [in] The number of vertices to draw.
    /// @todo Expose more arguments of vkCmdDraw in the argument list.
    /// @note Prefer vertex buffers with associated index buffers over vertex buffers with no index buffer.
    /// Not using an index buffer will decrease the performance drastically!
    void draw(std::size_t vertex_count) const;

    /// @brief Calls vkCmdDrawIndexed.
    /// @param index_count [in] The number of indices to draw.
    /// @todo Expose more arguments of vkCmdDrawIndexed in the argument list.
    void draw_indexed(std::size_t index_count) const;

    /// @brief Calls vkCmdEndRenderPass.
    void end_render_pass() const;

    [[nodiscard]] VkCommandBuffer get() const {
        return m_command_buffer;
    }

    [[nodiscard]] const VkCommandBuffer *ptr() const {
        return &m_command_buffer;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
