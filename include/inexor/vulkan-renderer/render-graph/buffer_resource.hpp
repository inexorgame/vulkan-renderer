#pragma once

#include "inexor/vulkan-renderer/wrapper/buffer.hpp"

#include <functional>
#include <optional>
#include <string>

namespace inexor::vulkan_renderer::render_graph {

/// The usage of the buffer inside of the rendergraph
enum class BufferUsage {
    VERTEX_BUFFER,
    INDEX_BUFFER,
    UNIFORM_BUFFER,
};

/// Wrapper for buffer resources inside of the rendergraph
// TODO: Make sure not to std::move() out any data from this class, as we might need to recreate physical resources
class BufferResource {
    friend class RenderGraph;

    // TODO: Add wrapper::Buffer here, or even std::vector<wrapper::Buffer> for automated double/triple buffering of
    // resources(!)
private:
    /// The internal debug name of the buffer
    std::string m_name;
    /// The internal usage of the buffer in the rendergraph
    BufferUsage m_usage{};
    /// The size of the buffer
    VkDeviceSize m_buffer_size{0};
    /// An optional function that updates the buffer
    OptionalBufferUpdateFunction m_on_update_buffer{std::nullopt};

    // TODO: Implement
    /// This specifies if we need a staging buffer to update the data
    bool m_requires_staging_buffer{false};

public:
    /// @note Make the constructor private so only ``RenderGraph`` can use it because it is declared as friend class

    /// Default constructor
    /// @param usage The internal usage of the buffer in the rendergraph
    /// @param size The size of the buffer in bytes (must not be 0!)
    /// @param name The internal debug name of the buffer (must not be empty)
    BufferResource(BufferUsage usage, VkDeviceSize size, std::string name);

    BufferResource(const BufferResource &) = delete;
    BufferResource(BufferResource &&other) noexcept;
    ~BufferResource() = default;

    BufferResource &operator=(const BufferResource &) = delete;
    BufferResource &operator=(BufferResource &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
