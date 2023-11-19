#pragma once

#include <volk.h>

#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class CommandBuffer;
}

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class RenderGraph;

/// A wrapper for graphics stages inside of rendergraph
class GraphicsStage {
    friend class RenderGraph;

private:
    std::string m_name;
    /// An optional clear value
    std::optional<VkClearValue> m_clear_values{std::nullopt};
    /// Add members which describe data related to graphics stages here
    std::function<void(const wrapper::CommandBuffer &)> m_on_record{[](auto &) {}};

    /// The buffers the graphics stage reads from
    /// If the buffer's ``BufferType`` is ``UNIFORM_BUFFER``, a value for the shader stage flag must be specified,
    /// because uniform buffers can be read from vertex or fragment stage bit.
    std::vector<std::pair<std::weak_ptr<BufferResource>, std::optional<VkShaderStageFlagBits>>> m_buffer_reads;

    /// The textures the graphics stage reads from
    std::vector<std::pair<std::weak_ptr<TextureResource>, std::optional<VkShaderStageFlagBits>>> m_texture_reads;
    /// The textures the graphics stage writes to
    std::vector<std::weak_ptr<TextureResource>> m_texture_writes;

public:
    /// Default constructor
    /// @param name The name of the graphics stage
    /// @param buffer_reads The buffers (vertex-, index-, or uniform buffers) the graphics stage reads from
    /// @param texture_reads The textures the graphics stage reads from
    /// @param texture_writes The textures the graphics stage writes to
    /// @param on_record The function which is called when the command buffer of the stage is being recorded
    /// @param clear_screen If specified, ``VkAttachmentLoadOp`` in ``VkRenderingAttachmentInfo`` will be set to
    /// ``VK_ATTACHMENT_LOAD_OP_CLEAR``, and the clear values specified here are used (``std::nullopt`` by default, in
    /// which case ``VK_ATTACHMENT_LOAD_OP_LOAD`` is used)
    GraphicsStage(
        std::string name,
        std::vector<std::pair<std::weak_ptr<BufferResource>, std::optional<VkShaderStageFlagBits>>> buffer_reads,
        std::vector<std::pair<std::weak_ptr<TextureResource>, std::optional<VkShaderStageFlagBits>>> texture_reads,
        std::vector<std::weak_ptr<TextureResource>> texture_writes,
        std::function<void(const wrapper::CommandBuffer &)> on_record, std::optional<VkClearValue> clear_values);

    GraphicsStage(const GraphicsStage &) = delete;
    GraphicsStage(GraphicsStage &&other) noexcept;
    ~GraphicsStage() = default;

    GraphicsStage &operator=(const GraphicsStage &) = delete;
    GraphicsStage &operator=(GraphicsStage &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
