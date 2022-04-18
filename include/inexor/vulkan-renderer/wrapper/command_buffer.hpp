#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declarations (in alphabetical order)
class Device;
class GraphicsPipeline;
class PipelineLayout;
class ResourceDescriptor;

/// RAII wrapper class for VkCommandBuffer
/// @todo Make trivially copyable (this class doesn't really "own" the command buffer, more just an OOP wrapper).
class CommandBuffer {
protected:
    const Device &m_device;
    VkCommandBuffer m_command_buffer{VK_NULL_HANDLE};
    std::string m_name;

    CommandBuffer(const Device &device, std::string name);
    void create_command_buffer(const VkCommandPool command_pool);

public:
    /// @brief Default constructor
    /// @param device The const reference to the device RAII wrapper class
    /// @param command_pool The command pool from which the command buffer will be allocated
    /// @param name The internal debug marker name of the command buffer. This must not be an empty string
    CommandBuffer(const Device &device, VkCommandPool command_pool, std::string name);

    CommandBuffer(const CommandBuffer &) = delete;
    CommandBuffer(CommandBuffer &&) noexcept;
    ~CommandBuffer() = default;

    CommandBuffer &operator=(const CommandBuffer &) = delete;
    CommandBuffer &operator=(CommandBuffer &&) noexcept = default;

    /// @brief Call vkBeginCommandBuffer
    /// @note Sometimes it's useful to pass VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT to specify that a command
    /// buffer can be resubmitted to a queue while it is in the pending state, and recorded into multiple primary
    /// command buffers. Otherwise, synchronization must be done using a VkFence
    /// @param flags The command buffer usage flags, 0 by default
    const CommandBuffer &begin_command_buffer(VkCommandBufferUsageFlags flags = 0) const;

    /// @brief Call vkCmdBindDescriptorSets
    /// @param descriptor_set The const reference to the resource descriptor RAII wrapper instance
    /// @param layout The pipeline layout which will be used to bind the resource descriptor
    /// @param first_set The first set to use
    const CommandBuffer &bind_descriptor_set(VkDescriptorSet descriptor_set, VkPipelineLayout layout,
                                             VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

    const CommandBuffer &bind_descriptor_set(VkDescriptorSet descriptor_set, const PipelineLayout &layout,
                                             VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

    /// @brief Call vkCmdBindDescriptorSets
    /// @param descriptor_sets
    /// @param layout
    /// @param bind_point
    const CommandBuffer &bind_descriptor_sets(const std::vector<VkDescriptorSet> &descriptor_sets,
                                              VkPipelineLayout layout,
                                              VkPipelineBindPoint bind_point = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

    /// @brief Update push constant data
    /// @tparam T the type of the push constants
    /// @param data The data
    /// @param layout The pipeline layout
    /// @param stage_flags The shader stage(s) that will accept the push constants
    /// @param offset The data offset
    template <typename DataType>
    const CommandBuffer &push_constant(const DataType *data, const VkPipelineLayout layout,
                                       const VkShaderStageFlags stage_flags, const std::uint32_t offset = 0) const {
        assert(data);
        assert(layout);
        vkCmdPushConstants(m_command_buffer, layout, stage_flags, offset, sizeof(DataType), data);
        return *this;
    }

    /// @brief Update push constant data
    /// @tparam T the type of the push constants
    /// @param data The data
    /// @param layout The pipeline layout
    /// @param stage_flags The shader stage(s) that will accept the push constants
    /// @param offset The data offset
    template <typename DataType>
    const CommandBuffer &push_constant(const DataType &data, const VkPipelineLayout layout,
                                       const VkShaderStageFlags stage_flags, const std::uint32_t offset = 0) const {
        assert(layout);
        vkCmdPushConstants(m_command_buffer, layout, stage_flags, offset, sizeof(DataType), &data);
        return *this;
    }

    template <typename DataType, typename PipelineWrapperType>
    const CommandBuffer &push_constant(const DataType &data, const PipelineWrapperType &pipeline_wrapper,
                                       const VkShaderStageFlags stage_flags, const std::uint32_t offset = 0) const {
        return push_constant(data, pipeline_wrapper.pipeline_layout(), stage_flags, offset);
    }

    const CommandBuffer &copy_buffer(VkBuffer source_buffer, VkBuffer target_buffer,
                                     const VkBufferCopy &copy_region) const;

    const CommandBuffer &copy_buffer(VkBuffer source_buffer, VkBuffer target_buffer,
                                     VkDeviceSize source_buffer_size) const;

    const CommandBuffer &copy_buffer_to_image(VkBuffer src_buffer, VkImage target_image,
                                              const VkBufferImageCopy &regions) const;

    const CommandBuffer &copy_buffer_to_image(VkBuffer src_buffer, VkImage target_image,
                                              const std::vector<VkBufferImageCopy> &regions) const;

    const CommandBuffer &copy_image(VkImage source_image, VkImage target_image, const VkImageCopy &copy_region) const;

    const CommandBuffer &set_scissor(const VkRect2D &scissor) const;

    const CommandBuffer &set_scissor(std::uint32_t width, std::uint32_t height) const;

    const CommandBuffer &set_viewport(const VkViewport &viewport) const;

    const CommandBuffer &set_viewport(std::uint32_t width, std::uint32_t height, float min_depth = 0.0f,
                                      float max_depth = 1.0f) const;

    const CommandBuffer &pipeline_barrier(VkPipelineStageFlags source_stage_flags,
                                          VkPipelineStageFlags destination_stage_flags,
                                          const VkImageMemoryBarrier &barrier) const;

    const CommandBuffer &pipeline_barrier(const VkImageMemoryBarrier &barrier) const;

    /// @brief Call vkCmdBeginRenderPass
    /// @param render_pass_bi The const reference to the VkRenderPassBeginInfo which is used
    const CommandBuffer &begin_render_pass(const VkRenderPassBeginInfo &render_pass_bi,
                                           const VkSubpassContents subpass_contents = VK_SUBPASS_CONTENTS_INLINE) const;

    // TODO: Implement bind_compute_pipeline!

    /// @brief Call vkCmdBindPipeline
    /// @param pipeline The graphics pipeline to bind
    const CommandBuffer &bind_graphics_pipeline(VkPipeline pipeline) const;

    template <typename PipelineWrapperType>
    const CommandBuffer &bind_graphics_pipeline(const PipelineWrapperType &wrapper) const {
        return bind_graphics_pipeline(wrapper.pipeline());
    }

    /// @brief Call vkCmdBindIndexBuffer
    /// @param buffer The index buffer to bind
    /// @param index_type The index type, ``VK_INDEX_TYPE_UINT32`` by default
    /// @param offset The buffer offset, ``0`` by default
    const CommandBuffer &bind_index_buffer(VkBuffer buffer, VkIndexType index_type = VK_INDEX_TYPE_UINT32,
                                           std::uint32_t offset = 0) const;

    template <typename IndexBufferWrapperType>
    const CommandBuffer &bind_index_buffer(const IndexBufferWrapperType &object) const {
        return bind_index_buffer(object.index_buffer());
    }

    /// Call vkCmdBindVertexBuffers
    // TODO: Expose more parameters and use C++20 std::span!
    const CommandBuffer &bind_vertex_buffer(VkBuffer buffer) const;

    /// @brief Call vkCmdBindVertexBuffers
    /// @param buffers A std::vector of vertex buffers to bind
    /// @todo Expose more parameters from vkCmdBindVertexBuffers as method arguments
    const CommandBuffer &bind_vertex_buffers(const std::vector<VkBuffer> &buffer) const;

    template <typename T>
    const CommandBuffer &bind_vertex_buffer(const T &object) const {
        return bind_vertex_buffer(object.vertex_buffer());
    }

    /// @brief Call vkCmdDraw
    /// @param vertex_count The number of vertices to draw
    /// @param first_vertex
    /// @param instance_count
    /// @param first_instance
    const CommandBuffer &draw(std::size_t vertex_count, std::uint32_t first_vertex = 0,
                              std::uint32_t instance_count = 1, std::uint32_t first_instance = 0) const;

    /// @brief Call vkCmdDrawIndexed
    /// @param index_count The number of indices to draw
    /// @param first_index The first index, ``0`` by default
    /// @param vertex_offset The vertex offset, ``0`` by default
    /// @param instance_count The number of instances to draw, ``1`` by default
    /// @param first_instance The first instance, ``0`` by default
    const CommandBuffer &draw_indexed(std::size_t index_count, std::uint32_t first_index = 0,
                                      std::uint32_t vertex_offset = 0, std::uint32_t instance_count = 1,
                                      std::uint32_t first_instance = 0) const;

    /// Call vkCmdEndRenderPass
    const CommandBuffer &end_render_pass() const;

    /// Call vkEndCommandBuffer
    const CommandBuffer &end_command_buffer() const;

    const CommandBuffer &flush_command_buffer_and_wait() const;

    const CommandBuffer &free_command_buffer(VkCommandPool cmd_pool) const;

    // TODO: Refactor: unified get syntax!
    [[nodiscard]] const VkCommandBuffer *ptr() const {
        return &m_command_buffer;
    }

    [[nodiscard]] VkCommandBuffer get() const {
        return m_command_buffer;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
