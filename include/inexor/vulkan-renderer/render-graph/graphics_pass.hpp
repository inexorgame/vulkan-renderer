#pragma once

#include <volk.h>

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"

#include <array>
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

// Using declaration
using wrapper::descriptors::DescriptorSetLayout;

/// An attachment is just a texture paired with an optional clear value
using Attachment = std::pair<std::weak_ptr<Texture>, std::optional<VkClearValue>>;

/// The debug label colors for vkCmdBeginDebugUtilsLabelEXT
enum class DebugLabelColor {
    RED,
    BLUE,
    GREEN,
    YELLOW,
    PURPLE,
    ORANGE,
    MAGENTA,
    CYAN,
    BROWN,
    PINK,
    LIME,
    TURQUOISE,
    BEIGE,
    MAROON,
    OLIVE,
    NAVY,
    TEAL,
};

/// Convert a DebugLabelColor to an array of RGBA float values to pass to vkCmdBeginDebugUtilsLabelEXT
/// @param color The DebugLabelColor
/// @return An array of RGBA float values to be passed into vkCmdBeginDebugUtilsLabelEXT
[[nodiscard]] std::array<float, 4> get_debug_label_color(const DebugLabelColor color);

/// A wrapper for graphics passes inside of rendergraph
class GraphicsPass {
    friend class RenderGraph;

private:
    /// The name of the graphics pass
    std::string m_name;
    /// Add members which describe data related to graphics passes here
    std::function<void(const CommandBuffer &)> m_on_record_cmd_buffer{[](auto &) {}};

    /// NOTE: We do not have members like m_has_depth_buffer or m_has_stencil_buffer because we can simply check if the
    /// std::weak_ptr<Texture> of the Attachment is expire() instead!

    /// The color attachments of the graphics pass
    std::vector<Attachment> m_color_attachments{};
    /// The depth attachment of the graphics pass
    Attachment m_depth_attachment{};
    /// The stencil attachment of the graphics pass
    Attachment m_stencil_attachment{};

    /// The descriptor set layout of the pass (this will be created by rendergraph)
    std::unique_ptr<DescriptorSetLayout> m_descriptor_set_layout;
    /// The descriptor set of the pass (this will be created by rendergraph)
    VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};

    /// The rendering info will be filled during rendergraph compilation so we don't have to do this while rendering
    VkRenderingInfo m_rendering_info{};

    // NOTE: The rendering info will be filled during rendergraph compilation with pointers to the members below. This
    // means we must make sure that the memory of the attachment infos below is still valid during rendering, which is
    // why we store them as members here.

    /// The color attachments inside of m_rendering_info
    std::vector<VkRenderingAttachmentInfo> m_color_attachment_infos{};
    /// The depth attachment inside of m_rendering_info
    VkRenderingAttachmentInfo m_depth_attachment_info{};
    /// The stencil attachment inside of m_rendering_info
    VkRenderingAttachmentInfo m_stencil_attachment_info{};

    /// The color of the debug label region (visible in graphics debuggers like RenderDoc)
    std::array<float, 4> m_debug_label_color;

public:
    /// Default constructor
    /// @param name The name of the graphics pass
    /// @param on_record_cmd_buffer The command buffer recording function of the graphics pass
    /// @param color_attachments The color attachments of the graphics pass
    /// @param depth_attachment The depth attachment of the graphics pass
    /// @param stencil_attachment The stencil attachment of the graphics pass
    GraphicsPass(std::string name,
                 std::function<void(const CommandBuffer &)> on_record_cmd_buffer,
                 std::vector<Attachment> color_attachments,
                 Attachment depth_attachment,
                 Attachment stencil_attachment,
                 DebugLabelColor color);

    GraphicsPass(const GraphicsPass &) = delete;
    GraphicsPass(GraphicsPass &&other) noexcept;
    ~GraphicsPass() = default;

    GraphicsPass &operator=(const GraphicsPass &) = delete;
    GraphicsPass &operator=(GraphicsPass &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
