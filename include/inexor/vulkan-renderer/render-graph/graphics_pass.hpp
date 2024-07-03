#pragma once

#include <volk.h>

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class RenderGraph;

// These using instructions make our life easier
// TODO: The second part of the pair is std::optional because not all buffers are read in some specific shader stage(?)
using BufferRead = std::pair<std::weak_ptr<Buffer>, std::optional<VkShaderStageFlagBits>>;
using BufferReads = std::vector<BufferRead>;
using TextureRead = std::pair<std::weak_ptr<Texture>, std::optional<VkShaderStageFlagBits>>;
using TextureReads = std::vector<TextureRead>;
using TextureWrites = std::vector<std::weak_ptr<Texture>>;

using wrapper::descriptors::DescriptorSetLayout;

/// A wrapper for graphics passes inside of rendergraph
class GraphicsPass {
    friend RenderGraph;

private:
    /// The name of the graphics pass
    std::string m_name;
    /// An optional clear value
    std::optional<VkClearValue> m_clear_values{std::nullopt};
    /// Add members which describe data related to graphics passes here
    std::function<void(const CommandBuffer &)> m_on_record{[](auto &) {}};

    /// The buffers the graphics passes reads from
    /// If the buffer's ``BufferType`` is ``UNIFORM_BUFFER``, a value for the shader stage flag must be specified,
    /// because uniform buffers can be read from vertex or fragment stage bit.
    BufferReads m_buffer_reads;
    /// The textures the graphics passes reads from
    TextureReads m_texture_reads;
    /// The textures the graphics passes writes to
    TextureWrites m_texture_writes;

    /// The vertex buffers (will be set by the rendergraph)
    std::vector<VkBuffer> m_vertex_buffers;
    /// The index buffer (will be set by the rendergraph)
    VkBuffer m_index_buffer{VK_NULL_HANDLE};

    /// The descriptor set layout of the pass (will be created by rendergraph)
    std::unique_ptr<DescriptorSetLayout> m_descriptor_set_layout;

    /// The descriptor set of the pass (will be created by rendergraph)
    VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};

    [[nodiscard]] bool has_index_buffer() const noexcept {
        return m_index_buffer != VK_NULL_HANDLE;
    }

public:
    /// Default constructor
    /// @param name The name of the graphics pass
    /// @param buffer_reads The buffers (vertex-, index-, or uniform buffers) the graphics passes reads from
    /// @param texture_reads The textures the graphics passes reads from
    /// @param texture_writes The textures the graphics passes writes to
    /// @param on_record The function which is called when the command buffer of the passes is being recorded
    /// @param clear_screen If specified, ``VkAttachmentLoadOp`` in ``VkRenderingAttachmentInfo`` will be set to
    /// ``VK_ATTACHMENT_LOAD_OP_CLEAR``, and the clear values specified here are used (``std::nullopt`` by default, in
    /// which case ``VK_ATTACHMENT_LOAD_OP_LOAD`` is used)
    /// @exception std::runtime_error More than one index buffer is specified
    GraphicsPass(std::string name,
                 BufferReads buffer_reads,
                 TextureReads texture_reads,
                 TextureWrites texture_writes,
                 std::function<void(const CommandBuffer &)> on_record,
                 std::optional<VkClearValue> clear_values);

    GraphicsPass(const GraphicsPass &) = delete;
    GraphicsPass(GraphicsPass &&other) noexcept;
    ~GraphicsPass() = default;

    GraphicsPass &operator=(const GraphicsPass &) = delete;
    GraphicsPass &operator=(GraphicsPass &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
