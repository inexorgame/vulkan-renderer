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

/// A builder class for graphics passes in the rendergraph
/// @warning Make sure that the order or add calls for buffers and textures matches the binding order!
class GraphicsPassBuilder {
private:
    /// Indicates if the screen is cleared at the beginning of this pass
    std::optional<VkClearValue> m_clear_value;
    /// Add members which describe data related to graphics passes here
    std::function<void(const wrapper::commands::CommandBuffer &)> m_on_record;
    /// Depth testing
    bool m_depth_test;

    /// The buffers which are read by the graphics pass
    /// If the buffer's ``BufferType`` is ``UNIFORM_BUFFER``, a value for the shader stage flag must be specified,
    /// because uniform buffers can be read from vertex or fragment stage bit.
    BufferReads m_buffer_reads;
    /// The textures the graphics pass reads from
    TextureReads m_texture_reads;
    /// The textures the graphics pass writes to
    TextureWrites m_texture_writes;
    /// The push constant ranges of the graphics pass
    std::vector<std::pair<VkPushConstantRange, std::function<void()>>> m_push_constant_ranges;

    // TODO: Merge push constant ranges into one block and put it as member here?
    // TODO: Copy all data into one piece of memory and call vkCmdPushConstants only once?
    void compile_push_constants();

    /// Reset all data of the graphics pass builder
    void reset();

public:
    GraphicsPassBuilder();
    GraphicsPassBuilder(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder(GraphicsPassBuilder &&) noexcept;
    ~GraphicsPassBuilder() = default;

    GraphicsPassBuilder &operator=(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder &operator=(GraphicsPassBuilder &&) noexcept;

    /// Add a push constant range to the graphics pass
    /// @param shader_stage The shader stage for the push constant range
    /// @param push_constant The push constant data
    /// @param on_update The update callable
    /// @param offset The offset in the push constant range
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    template <typename PushConstantDataType>
    [[nodiscard]] auto &add_push_constant_range(const VkShaderStageFlags shader_stage,
                                                const PushConstantDataType &push_constant,
                                                std::function<void()> on_update, const std::uint32_t offset = 0) {
        m_push_constant_ranges.emplace_back(
            VkPushConstantRange{
                .stageFlags = shader_stage,
                .offset = offset,
                .size = sizeof(push_constant),
            },
            std::move(on_update));
        return *this;
    }

    /// Build the graphics pass
    /// @param name The name of the graphics pass
    /// @return The graphics pass that was just created
    [[nodiscard]] auto build(std::string name) {
        auto graphics_pass = std::make_shared<GraphicsPass>(std::move(name), std::move(m_buffer_reads),
                                                            std::move(m_texture_reads), std::move(m_texture_writes),
                                                            std::move(m_on_record), std::move(m_clear_value));
        // Don't forget to reset the builder automatically before returning the graphics pass that was just created
        reset();
        // Return the graphics pass that was created
        return graphics_pass;
    }

    // TODO: We must specify buffer reads for vertex and index buffers, but bind manually... is that good?
    // TODO: std::optional<VkShaderStageFlagBits> or better default VkShaderStageFlagBits to VK_SHADER_STAGE_VERTEX_BIT?

    /// Specify that the pass reads from a buffer
    /// @param buffer The buffer the pass reads from
    /// @param shader_stage The shader stage the buffer is read from
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &reads_from_buffer(std::weak_ptr<Buffer> buffer,
                                          std::optional<VkShaderStageFlagBits> shader_stage = std::nullopt) {
        m_buffer_reads.emplace_back(std::move(buffer), shader_stage);
        return *this;
    }

    /// Specify that the pass reads from a texture
    /// @param texture The texture the pass reads from
    /// @param shader_stage The shader stage the texture is read from
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &reads_from_texture(std::weak_ptr<Texture> texture,
                                           std::optional<VkShaderStageFlagBits> shader_stage = std::nullopt) {
        m_texture_reads.emplace_back(std::move(texture), shader_stage);
        return *this;
    }

    /// Specify that the pass writes to a texture
    /// @param texture The texture the pass writes to
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &writes_to_texture(std::weak_ptr<Texture> texture) {
        m_texture_writes.emplace_back(texture);
        return *this;
    }

    /// Set the clear status for the pass
    /// @param clear_value The clear value for color and depth
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &set_clear_value(VkClearValue clear_value) {
        m_clear_value = clear_value;
        return *this;
    }

    /// Enable or disable depth testing
    /// @param depth_test ``true`` if depth testing is enabled for this pass
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &set_depth_test(bool depth_test) {
        m_depth_test = depth_test;
        return *this;
    }

    /// Set the function which will be called when the command buffer for rendering of the pass is being recorded
    /// @param on_record The command buffer recording function
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &set_on_record(std::function<void(const wrapper::commands::CommandBuffer &)> on_record) {
        m_on_record = std::move(on_record);
        return *this;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
