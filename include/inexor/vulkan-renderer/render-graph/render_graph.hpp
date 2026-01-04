#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

/// Using declaration
using wrapper::DebugLabelColor;

class RenderGraph {
private:
    // The device wrapper
    const Device &m_device;
    /// The buffers (vertex buffers, index buffers, uniform buffers...)
    std::vector<std::shared_ptr<Buffer>> m_buffers;
    /// The textures (back buffers, depth buffers, textures...)
    std::vector<std::shared_ptr<Texture>> m_textures;
    /// The graphics passes
    std::vector<std::shared_ptr<GraphicsPass>> m_graphics_passes;
    /// An instance of the graphics pass builder
    GraphicsPassBuilder m_graphics_pass_builder{};

public:
    /// Default constructor
    /// @param device The device wrapper
    RenderGraph(const Device &device);

    /// Add a buffer to the rendergraph
    /// @param name The buffer name
    /// @param type The buffer type
    /// @param on_update The buffer update function
    /// @return A weak pointer to the buffer resource which was created
    [[nodiscard]] std::weak_ptr<Buffer> add_buffer(std::string name, BufferType type, std::function<void()> on_update);

    /// Add a graphics pass to the rendergraph
    /// @param graphics_pass The graphics pass which was created
    /// @return A weak pointer to the graphics pass which was created
    [[nodiscard]] std::weak_ptr<GraphicsPass> add_graphics_pass(std::shared_ptr<GraphicsPass> graphics_pass);

    // @TODO How to handle optional texture update depending on texture type?

    /// Add a texture to the rendergraph
    /// @param name The texture name
    /// @param usage The texture usage
    /// @param format The texture format
    /// @param width The texture width
    /// @param height The texture height
    /// @param channels The number of channels
    /// @param sample_count The number of samples
    /// @param on_update The texture update function
    /// @return A weak pointer to the texture which was created
    [[nodiscard]] std::weak_ptr<Texture> add_texture(
        std::string name, TextureUsage usage, VkFormat format, std::uint32_t width, std::uint32_t height,
        std::uint32_t channels, VkSampleCountFlagBits sample_count = VK_SAMPLE_COUNT_1_BIT,
        std::function<void()> m_on_check_for_updates = []() {});

    /// Compile the rendergraph
    void compile();

    [[nodiscard]] GraphicsPassBuilder &get_graphics_pass_builder() {
        return m_graphics_pass_builder;
    }

    /// Render the rendergraph
    void render();

    /// Reset the entire rendergraph
    void reset();
};

} // namespace inexor::vulkan_renderer::render_graph
