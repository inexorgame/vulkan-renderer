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

    /// Enable MSAA for this pass
    bool m_enable_msaa{false};
    /// Clear the color attachment
    bool m_clear_color_attachment{false};
    /// Clear the stencil attachment
    bool m_clear_stencil_attachment{false};

    std::weak_ptr<Texture> m_color_attachment;
    std::weak_ptr<Texture> m_depth_attachment;
    std::weak_ptr<Texture> m_stencil_attachment;
    std::weak_ptr<Texture> m_msaa_color_attachment;
    std::weak_ptr<Texture> m_msaa_depth_attachment;

    /// The descriptor set layout of the pass (will be created by rendergraph)
    std::unique_ptr<DescriptorSetLayout> m_descriptor_set_layout;

    /// The descriptor set of the pass (will be created by rendergraph)
    VkDescriptorSet m_descriptor_set{VK_NULL_HANDLE};

public:
    GraphicsPass(std::string name,
                 std::function<void(const CommandBuffer &)> on_record,
                 std::weak_ptr<Texture> m_color_attachment,
                 std::weak_ptr<Texture> m_depth_attachment,
                 std::weak_ptr<Texture> m_stencil_attachment,
                 std::weak_ptr<Texture> m_msaa_color_attachment,
                 std::weak_ptr<Texture> m_msaa_depth_attachment,
                 bool enable_msaa,
                 bool clear_color_attachment,
                 bool clear_stencil_attachment,
                 std::optional<VkClearValue> clear_values);

    GraphicsPass(const GraphicsPass &) = delete;
    // TODO: Fix me!
    GraphicsPass(GraphicsPass &&other) noexcept;
    ~GraphicsPass() = default;

    GraphicsPass &operator=(const GraphicsPass &) = delete;
    GraphicsPass &operator=(GraphicsPass &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
