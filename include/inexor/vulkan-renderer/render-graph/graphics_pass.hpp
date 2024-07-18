#pragma once

#include <volk.h>

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

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
using wrapper::Swapchain;
using wrapper::descriptors::DescriptorSetLayout;

/// A universal rendering attachment structure for color, depth, stencil attachment and swapchains
struct UniversalRenderingAttachment {
    // This pointer will point to either the image view of the attachment or to the image view of the current swapchain
    // image. This allows us to treat all attachment types and swapchain in a unified way.
    VkImageView *img_view_ptr{nullptr};
    /// The extent of the attachment or the swapchain
    VkExtent2D *extent_ptr{nullptr};
    /// The clear value of the attachment or the swapchain
    std::optional<VkClearValue> clear_value{std::nullopt};
};

// TODO: Move this to device wrapper(?)

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
    //
    friend class RenderGraph;

private:
    /// The name of the graphics pass
    std::string m_name;
    /// Add members which describe data related to graphics passes here
    std::function<void(const CommandBuffer &)> m_on_record_cmd_buffer{[](auto &) {}};

    /// The descriptor set layout of the pass (this will be created by rendergraph)
    std::unique_ptr<DescriptorSetLayout> m_descriptor_set_layout;
    /// The descriptor set of the pass (this will be created by rendergraph)
    VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};

    /// The color of the debug label region (visible in graphics debuggers like RenderDoc)
    std::array<float, 4> m_debug_label_color;

    // NOTE: The rendering info will be filled during rendergraph compilation with pointers to the members below. This
    // means we must make sure that the memory of the attachment infos below is still valid during rendering, which is
    // why we store them as members here.

    /// The graphics passes this pass reads from
    std::vector<std::weak_ptr<GraphicsPass>> m_graphics_pass_reads;

    /// The texture attachments of this pass (unified means color, depth, stencil attachment or a swapchain)
    std::vector<std::weak_ptr<Texture>> m_write_attachments{};
    /// The swapchains this graphics pass writes to
    std::vector<std::weak_ptr<Swapchain>> m_write_swapchains{};

    // All the data below will be filled and used by rendergraph only

    /// The rendering info will be filled during rendergraph compilation so we don't have to do this while rendering
    VkRenderingInfo m_rendering_info{};
    /// The color attachments inside of m_rendering_info
    std::vector<VkRenderingAttachmentInfo> m_color_attachment_infos{};
    /// The depth attachment inside of m_rendering_info
    VkRenderingAttachmentInfo m_depth_attachment_info{};
    /// The stencil attachment inside of m_rendering_info
    VkRenderingAttachmentInfo m_stencil_attachment_info{};

public:
    /// Default constructor
    /// @param name The name of the graphics pass
    /// @param on_record_cmd_buffer The command buffer recording function of the graphics pass
    /// @param graphics_pass_reads The graphics passes this graphics pass reads from
    /// @param write_attachments The attachment this graphics pass writes to
    /// @param write_swapchains The swapchains this graphics pass writes to
    /// @param pass_debug_label_color The debug label of the pass (visible in graphics debuggers like RenderDoc)
    GraphicsPass(std::string name,
                 std::function<void(const CommandBuffer &)> on_record_cmd_buffer,
                 std::vector<std::weak_ptr<GraphicsPass>> graphics_pass_reads,
                 std::vector<std::weak_ptr<Texture>> write_attachments,
                 std::vector<std::weak_ptr<Swapchain>> write_swapchains,
                 DebugLabelColor pass_debug_label_color);

    GraphicsPass(const GraphicsPass &) = delete;
    GraphicsPass(GraphicsPass &&other) noexcept;
    ~GraphicsPass() = default;

    GraphicsPass &operator=(const GraphicsPass &) = delete;
    GraphicsPass &operator=(GraphicsPass &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
