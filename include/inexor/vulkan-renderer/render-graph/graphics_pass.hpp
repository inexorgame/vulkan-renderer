#pragma once

#include <volk.h>

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
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

// Using declarations
using wrapper::Swapchain;
using wrapper::descriptors::DescriptorSetLayout;

/// Using declaration
using OnRecordCommandBufferForPass = std::function<void(const CommandBuffer &)>;

/// A wrapper for graphics passes inside of rendergraph
class GraphicsPass {
    friend class RenderGraph;

private:
    /// The name of the graphics pass
    std::string m_name;

    /// The command buffer recording function of the graphics pass
    OnRecordCommandBufferForPass m_on_record_cmd_buffer{[](auto &) {}};

    /// The descriptor set layout of the pass (this will be created by rendergraph)
    std::unique_ptr<DescriptorSetLayout> m_descriptor_set_layout;
    /// The descriptor set of the pass (this will be created by rendergraph)
    VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};

    /// The color of the debug label region (visible in graphics debuggers like RenderDoc)
    std::array<float, 4> m_debug_label_color;

    /// The extent
    VkExtent2D m_extent{0, 0};
    /// The graphics passes this pass reads from
    std::vector<std::weak_ptr<GraphicsPass>> m_graphics_pass_reads;

    /// The texture attachments of this pass (unified means color, depth, stencil attachment or a swapchain)
    std::vector<std::pair<std::weak_ptr<Texture>, std::optional<VkClearValue>>> m_write_attachments{};
    /// The swapchains this graphics pass writes to
    std::vector<std::pair<std::weak_ptr<Swapchain>, std::optional<VkClearValue>>> m_write_swapchains{};

    // All the data below will be filled and used by rendergraph only

    /// The rendering info will be filled during rendergraph compilation so we don't have to do this while rendering.
    /// This means we must make sure that the memory of the attachment infos below is still valid during rendering,
    /// which is why we store them as members here.
    VkRenderingInfo m_rendering_info{};
    /// The color attachments inside of m_rendering_info
    std::vector<VkRenderingAttachmentInfo> m_color_attachments{};
    /// Does this graphics pass have any depth attachment?
    bool m_has_depth_attachment{false};
    /// The depth attachment inside of m_rendering_info
    VkRenderingAttachmentInfo m_depth_attachment{};
    /// Does this graphics pass have any stencil attachment?
    bool m_has_stencil_attachment{false};
    /// The stencil attachment inside of m_rendering_info
    VkRenderingAttachmentInfo m_stencil_attachment{};

    /// Reset the rendering info
    void reset_rendering_info();

public:
    /// Default constructor
    /// @param name The name of the graphics pass
    /// @param on_record_cmd_buffer The command buffer recording function of the graphics pass
    /// @param graphics_pass_reads The graphics passes this graphics pass reads from
    /// @param write_attachments The attachment this graphics pass writes to
    /// @param write_swapchains The swapchains this graphics pass writes to
    /// @param pass_debug_label_color The debug label of the pass (visible in graphics debuggers like RenderDoc)
    GraphicsPass(std::string name,
                 OnRecordCommandBufferForPass on_record_cmd_buffer,
                 std::vector<std::weak_ptr<GraphicsPass>> graphics_pass_reads,
                 std::vector<std::pair<std::weak_ptr<Texture>, std::optional<VkClearValue>>> write_attachments,
                 std::vector<std::pair<std::weak_ptr<Swapchain>, std::optional<VkClearValue>>> write_swapchains,
                 wrapper::DebugLabelColor pass_debug_label_color);

    GraphicsPass(const GraphicsPass &) = delete;
    GraphicsPass(GraphicsPass &&other) noexcept;
    ~GraphicsPass() = default;

    GraphicsPass &operator=(const GraphicsPass &) = delete;
    GraphicsPass &operator=(GraphicsPass &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
