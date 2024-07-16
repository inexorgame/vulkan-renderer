#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <functional>
#include <memory>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBuffer;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::commands::CommandBuffer;

/// A builder class for graphics passes in the rendergraph
/// @warning Make sure that the order or add calls for buffers and textures matches the binding order!
class GraphicsPassBuilder {
private:
    /// Add members which describe data related to graphics passes here
    std::function<void(const CommandBuffer &)> m_on_record_cmd_buffer{};

    /// The texture resources this graphics pass writes to
    std::vector<RenderingAttachment> m_write_attachments{};

    /// The graphics passes which are read by this graphics pass. Based on this, rendergraph can automatically determine
    /// the correct pass order based on depth first search algorithm (DFS)
    std::vector<std::weak_ptr<GraphicsPass>> m_graphics_pass_reads{};

    /// Reset all data of the graphics pass builder
    void reset();

public:
    GraphicsPassBuilder();
    GraphicsPassBuilder(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder(GraphicsPassBuilder &&) noexcept;
    ~GraphicsPassBuilder() = default;

    GraphicsPassBuilder &operator=(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder &operator=(GraphicsPassBuilder &&) = delete;

    /// Build the graphics pass
    /// @param name The name of the graphics pass
    /// @param color The debug label color (debug labels are specified per pass and are visible in RenderDoc debugger)
    /// @return The graphics pass that was just created
    [[nodiscard]] std::shared_ptr<GraphicsPass> build(std::string name, DebugLabelColor color);

    /// Specify that this graphics pass A reads from another graphics pass B, meaning B should be rendered before A
    /// @param graphics_pass The graphics pass which is read by this graphics pass
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &reads_from(std::weak_ptr<GraphicsPass> graphics_pass);

    /// Set the function which will be called when the command buffer for rendering of the pass is being recorded
    /// @param on_record_cmd_buffer The command buffer recording function
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &set_on_record(std::function<void(const CommandBuffer &)> on_record_cmd_buffer);

    ///
    /// @param color_attachment
    /// @param clear_value
    /// @return
    [[nodiscard]] GraphicsPassBuilder &writes_to(std::weak_ptr<Texture> color_attachment,
                                                 std::optional<VkClearValue> clear_value = std::nullopt);
};

} // namespace inexor::vulkan_renderer::render_graph
