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

using wrapper::commands::CommandBuffer;

/// A builder class for graphics passes in the rendergraph
/// @warning Make sure that the order or add calls for buffers and textures matches the binding order!
class GraphicsPassBuilder {
private:
    /// Add members which describe data related to graphics passes here
    std::function<void(const CommandBuffer &)> m_on_record;
    /// Depth testing
    bool m_enable_depth_test{false};
    /// Multisample anti-aliasing (MSAA)
    bool m_enable_msaa{false};
    /// Clear the color attachment
    bool m_clear_color{false};
    /// Clear the stencil attachment
    bool m_clear_stencil{false};
    /// Indicates if the screen is cleared at the beginning of the pass
    VkClearValue m_clear_value{};

    // TODO: Multiple color attachments!
    std::weak_ptr<Texture> m_color_attachment;
    std::weak_ptr<Texture> m_depth_attachment;
    std::weak_ptr<Texture> m_stencil_attachment;
    std::weak_ptr<Texture> m_msaa_color_attachment;
    std::weak_ptr<Texture> m_msaa_depth_attachment;

    /// Reset all data of the graphics pass builder
    void reset();

public:
    GraphicsPassBuilder();
    GraphicsPassBuilder(const GraphicsPassBuilder &) = delete;
    // TODO: Implement me!
    GraphicsPassBuilder(GraphicsPassBuilder &&) noexcept;
    ~GraphicsPassBuilder() = default;

    GraphicsPassBuilder &operator=(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder &operator=(GraphicsPassBuilder &&) noexcept;

    /// Add a color attachment to the pass
    /// @param color_attachment The color attachment
    /// @param clear_color The clear color for the color attachment (``std::nullopt`` by default)
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &add_color_attachment(std::weak_ptr<Texture> color_attachment,
                                             std::optional<VkClearColorValue> clear_color = std::nullopt) {
        if (color_attachment.expired()) {
            throw std::invalid_argument(
                "[GraphicsPassBuilder::add_color_attachment] Error: 'color_attachment' is expired!");
        }
        m_color_attachment = color_attachment;
        if (clear_color) {
            m_clear_color = true;
            m_clear_value.color = clear_color.value();
        }
        return *this;
    }

    /// Build the graphics pass
    /// @param name The name of the graphics pass
    /// @return The graphics pass that was just created
    [[nodiscard]] auto build(std::string name) {
        auto graphics_pass = std::make_shared<GraphicsPass>(
            std::move(name), std::move(m_on_record), std::move(m_color_attachment), std::move(m_depth_attachment),
            std::move(m_stencil_attachment), std::move(m_msaa_color_attachment), std::move(m_msaa_depth_attachment),
            m_enable_msaa, m_clear_color, m_clear_stencil, std::move(m_clear_value));

        // Reset the builder so the builder can be re-used
        reset();
        // Return the graphics pass that was created
        return graphics_pass;
    }

    /// Enable depth testing for the pass
    /// @param depth_buffer
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &enable_depth_test(std::weak_ptr<Texture> depth_attachment) {
        if (depth_attachment.expired()) {
            throw std::invalid_argument("[GraphicsPassBuilder::enable_depth_test] Error: 'depth_buffer' is expired!");
        }
        m_enable_depth_test = true;
        m_depth_attachment = depth_attachment;
        return *this;
    }

    /// Enable multisample anti-aliasing (MSAA) for the pass
    /// @param sample_count The MSAA sample count
    /// @param msaa_back_attachment The MSAA attachment
    /// @param msaa_depth_attachment The MSAA depth attachment
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &enable_msaa(VkSampleCountFlagBits sample_count,
                                    std::weak_ptr<Texture> msaa_back_attachment,
                                    std::weak_ptr<Texture> msaa_depth_attachment) {
        if (msaa_back_attachment.expired()) {
            throw std::invalid_argument("[GraphicsPassBuilder::enable_msaa] Error: 'msaa_back_buffer' is expired!");
        }
        if (msaa_depth_attachment.expired()) {
            throw std::invalid_argument("[GraphicsPassBuilder::enable_msaa] Error: 'msaa_depth_buffer' is expired!");
        }
        m_enable_msaa = true;
        m_msaa_color_attachment = msaa_back_attachment;
        m_msaa_depth_attachment = msaa_depth_attachment;
        return *this;
    }

    /// Set the function which will be called when the command buffer for rendering of the pass is being recorded
    /// @param on_record The command buffer recording function
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &set_on_record(std::function<void(const CommandBuffer &)> on_record) {
        m_on_record = std::move(on_record);
        return *this;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
