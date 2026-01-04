#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
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

public:
    /// Default constructor
    /// @param device The device wrapper
    RenderGraph(const Device &device);

    /// Add a buffer to the rendergraph
    /// @param name The buffer name
    /// @param type The buffer type
    /// @param on_update The buffer update function
    /// @return A weak pointer to the buffer resource which was created
    std::weak_ptr<Buffer> add_buffer(std::string name, BufferType type, std::function<void()> on_update);

    /// Add a graphics pass to the rendergraph
    /// @param graphics_pass The graphics pass which was created
    /// @return A weak pointer to the graphics pass which was created
    std::weak_ptr<GraphicsPass> add_graphics_pass(std::shared_ptr<GraphicsPass> graphics_pass);

    // @TODO How to handle optional texture update depending on texture type?

    /// Add a texture to the rendergraph
    /// @param name The texture name
    /// @param type The texture type
    /// @param on_update The texture update function (``std::nullopt`` by default)
    /// @return A weak pointer to the texture resource which was created
    std::weak_ptr<Texture> add_texture(std::string name, TextureType type,
                                       std::optional<std::function<void()>> on_update = std::nullopt);

    /// Compile the rendergraph
    void compile();

    /// Render the rendergraph
    void render();

    /// Reset the entire rendergraph
    void reset();
};

} // namespace inexor::vulkan_renderer::render_graph
