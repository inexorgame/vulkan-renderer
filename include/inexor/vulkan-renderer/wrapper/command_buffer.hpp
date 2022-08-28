#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;

/// @brief RAII wrapper class for VkCommandBuffer.
/// @todo Make trivially copyable (this class doesn't really "own" the command buffer, more just an OOP wrapper).
class CommandBuffer {
    VkCommandBuffer m_command_buffer{VK_NULL_HANDLE};
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

    /// Call vkBeginCommandBuffer
    /// @param flags The command buffer usage flags, 0 by default
    const CommandBuffer &begin_command_buffer(VkCommandBufferUsageFlags flags = 0) const; // NOLINT

    /// Call vkCmdBeginRenderPass
    /// @param render_pass_bi The renderpass begin info
    /// @param subpass_contents The subpass contents (``VK_SUBPASS_CONTENTS_INLINE`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi, // NOLINT
                                           VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) const;

    /// Call vkCmdBindDescriptorSets
    /// @param desc_sets The descriptor sets to bind
    /// @param layout The pipeline layout
    /// @param bind_point the pipeline bind point (``VK_PIPELINE_BIND_POINT_GRAPHICS`` by default)
    /// @param first_set The first descriptor set (``0`` by default)
    /// @param dyn_offsets The dynamic offset values (empty by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    const CommandBuffer &bind_descriptor_sets(std::span<const VkDescriptorSet> desc_sets, // NOLINT
                                              VkPipelineLayout layout,
                                              VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS,
                                              std::uint32_t first_set = 0,
                                              std::span<const std::uint32_t> dyn_offsets = {}) const;

    /// @brief Update push constant data.
    /// @param layout The pipeline layout
    /// @param stage The shader stage that will be accepting the push constants
    /// @param size The size of the push constant data in bytes
    /// @param data A pointer to the push constant data
    void push_constants(VkPipelineLayout layout, VkShaderStageFlags stage, std::uint32_t size, void *data) const;

    /// @brief Call vkEndCommandBuffer.
    void end() const;

    // Graphics commands
    // TODO(): Switch to taking in OOP wrappers when we have them (e.g. bind_vertex_buffers takes in a VertexBuffer)

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
    void draw_indexed(std::size_t index_count) const;

    /// @brief Call vkCmdEndRenderPass.
    void end_render_pass() const;

    [[nodiscard]] VkCommandBuffer get() const {
        return m_command_buffer;
    }

    [[nodiscard]] const VkCommandBuffer *ptr() const {
        return &m_command_buffer;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
