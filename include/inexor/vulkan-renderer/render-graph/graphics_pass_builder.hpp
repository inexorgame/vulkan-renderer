#pragma once

#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"

#include <functional>
#include <memory>
#include <utility>
#include <variant>

namespace inexor::vulkan_renderer::wrapper::commands {
// Forward declaration
class CommandBufferBuilder;
} // namespace inexor::vulkan_renderer::wrapper::commands

namespace inexor::vulkan_renderer::wrapper::swapchains {
// Forward declaration
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper::swapchains

namespace inexor::vulkan_renderer::render_graph {
// Forward declarations
class Buffer;
class GraphicsPass;
class Texture;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::render_graph {

// Using declarations
using wrapper::commands::CommandBufferBuilder;
using wrapper::core::DebugLabelColor;
using wrapper::swapchains::Swapchain;

// Using declaration
using TextureOrSwapchain = std::variant<std::weak_ptr<Texture>, std::weak_ptr<Swapchain>>;

/// A builder class for graphics passes in the rendergraph
class GraphicsPassBuilder {
private:
    /// The command buffer recording function
    std::function<void(CommandBufferBuilder &)> m_on_record_cmd_buffer;
    /// The textures to which this graphics pass writes to
    std::vector<std::pair<std::weak_ptr<Texture>, std::optional<VkClearValue>>> m_texture_writes;
    /// The swapchains to which this graphics pass writes to
    std::vector<std::pair<std::weak_ptr<Swapchain>, std::optional<VkClearValue>>> m_swapchain_writes;
    /// The buffers which are read by this graphics pass
    std::vector<std::weak_ptr<Buffer>> m_buffer_reads;
    /// The buffers which are written to by this graphics pass
    std::vector<std::weak_ptr<Buffer>> m_buffer_writes;

    /// Reset the data of the graphics pass builder
    void reset();

public:
    GraphicsPassBuilder();

    GraphicsPassBuilder(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder(GraphicsPassBuilder &&) noexcept;

    /// Build the graphics pass
    /// @param name The name of the graphics pass
    /// @param color The debug label color (debug labels are specified per pass and are visible in RenderDoc debugger)
    /// @return The graphics pass that was just created
    [[nodiscard]] std::shared_ptr<GraphicsPass> build(std::string name, DebugLabelColor color);

    /// Conditionally reads from a buffer
    /// @param read_buffer The buffer which is read by this graphics pass
    /// @param condition The condition that must be true for the read to happen
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &conditionally_reads_from(std::weak_ptr<Buffer> buffer, bool condition);

    /// Conditionally writes to a texture or swapchain
    /// @param write_attachment The texture or swapchain to which this graphics pass writes to
    /// @param condition The condition that must be true for the write to happen
    /// @param clear_value The clear value to use for this attachment (if any)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &conditionally_writes_to(TextureOrSwapchain write_attachment, bool condition,
                                                               std::optional<VkClearValue> clear_value = std::nullopt);

    /// Specify that this graphics pass reads from a buffer
    /// @param buffer The buffer which is read by this graphics pass
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &reads_from(std::weak_ptr<Buffer> buffer);

    /// Set the function which will be called when the command buffer for rendering of the pass is being recorded
    /// @param on_record_cmd_buffer The command buffer recording function
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &set_on_record(std::function<void(CommandBufferBuilder &)> on_record_cmd_buffer);

    /// Specify that this graphics pass writes to a buffer
    /// @brief buffer The buffer that is written to
    /// @note This feature might be used in the future if a pass writes to a buffer
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &writes_to(std::weak_ptr<Buffer> buffer);

    /// Specify that this graphics pass writes to an either a std::weak_ptr<Texture> or a std::weak_ptr<Swapchain>
    /// @param attachment The attachment (either a std::weak_ptr<Texture> or a std::weak_ptr<Swapchain>)
    /// @param clear_value The optional clear value of the attachment (``std::nullopt`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsPassBuilder &writes_to(TextureOrSwapchain write_attachment,
                                                 std::optional<VkClearValue> clear_value = std::nullopt);
};

} // namespace inexor::vulkan_renderer::render_graph
