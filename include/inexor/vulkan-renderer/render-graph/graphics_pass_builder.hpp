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
    /// The color attachments of the graphics pass
    std::vector<Attachment> m_color_attachments{};
    /// The depth attachment of the graphics pass
    Attachment m_depth_attachment{};
    /// The stencil attachment of the graphics pass
    Attachment m_stencil_attachment{};

    /// Reset all data of the graphics pass builder
    void reset();

public:
    GraphicsPassBuilder();
    GraphicsPassBuilder(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder(GraphicsPassBuilder &&) noexcept;
    ~GraphicsPassBuilder() = default;

    GraphicsPassBuilder &operator=(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder &operator=(GraphicsPassBuilder &&) = delete;

    /// Add a color attachment to the pass
    /// @param color_attachment The color attachment
    /// @param clear_value The clear value for the color attachment (``std::nullopt`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &add_color_attachment(std::weak_ptr<Texture> color_attachment,
                                                            std::optional<VkClearValue> clear_value = std::nullopt);

    /// Enable depth testing for the pass
    /// @param depth_attachment The depth attachment
    /// @param clear_value The clear value for the depth attachment (``std::nullopt`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &add_depth_attachment(std::weak_ptr<Texture> depth_attachment,
                                                            std::optional<VkClearValue> clear_value = std::nullopt);

    /// Add a stencil attachment to the pass
    /// @param stencil_attachment The stencil attachment
    /// @param clear_value The clear value for the stencil attachment (``std::nullopt`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &add_stencil_attachment(std::weak_ptr<Texture> stencil_attachment,
                                                              std::optional<VkClearValue> clear_value = std::nullopt);

    /// Build the graphics pass
    /// @param name The name of the graphics pass
    /// @return The graphics pass that was just created
    [[nodiscard]] GraphicsPass build(std::string name);

    /// Set the function which will be called when the command buffer for rendering of the pass is being recorded
    /// @param on_record_cmd_buffer The command buffer recording function
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &set_on_record(std::function<void(const CommandBuffer &)> on_record_cmd_buffer);
};

} // namespace inexor::vulkan_renderer::render_graph
