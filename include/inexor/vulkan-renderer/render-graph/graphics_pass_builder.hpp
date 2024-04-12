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

/// A builder class for graphics stages in the rendergraph
/// @warning Make sure that the order or add calls for buffers and textures matches the binding order!
class GraphicsPassBuilder {
private:
    /// Indicates if the screen is cleared at the beginning of this stage
    std::optional<VkClearValue> m_clear_value;
    /// Add members which describe data related to graphics stages here
    std::function<void(const wrapper::commands::CommandBuffer &)> m_on_record;
    /// Depth testing
    bool m_depth_test;

    /// The buffers the graphics stage reads from
    /// If the buffer's ``BufferType`` is ``UNIFORM_BUFFER``, a value for the shader stage flag must be specified,
    /// because uniform buffers can be read from vertex or fragment stage bit.
    BufferReads m_buffer_reads;
    /// The textures the graphics stage reads from
    TextureReads m_texture_reads;
    /// The textures the graphics stage writes to
    TextureWrites m_texture_writes;
    /// The push constant ranges of the graphics stage
    std::vector<std::pair<VkPushConstantRange, std::function<void()>>> m_push_constant_ranges;

    // TODO: Merge push constant ranges into one block and put it as member here?
    // TODO: Copy all data into one piece of memory and call vkCmdPushConstants only once?
    void compile_push_constants();

    /// Reset all data of the graphics stage builder
    void reset();

public:
    GraphicsPassBuilder();
    GraphicsPassBuilder(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder(GraphicsPassBuilder &&) noexcept;
    ~GraphicsPassBuilder() = default;

    GraphicsPassBuilder &operator=(const GraphicsPassBuilder &) = delete;
    GraphicsPassBuilder &operator=(GraphicsPassBuilder &&) noexcept;

    /// Add a push constant range to the graphics stage
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

    /// Build the graphics stage and reset the builder's data
    /// @param name The name of the graphics stage
    /// @return The graphics stage which was created
    [[nodiscard]] std::shared_ptr<GraphicsPass> build(std::string name);

    // TODO: We must specify buffer reads for vertex and index buffers, but bind manually... is that good?

    /// Specifies that this pass reads from a buffer
    /// @param buffer The buffer the pass reads from
    /// @param shader_stage The shader stage the buffer is read from
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &reads_from_buffer(std::weak_ptr<Buffer> buffer,
                                          std::optional<VkShaderStageFlagBits> shader_stage = std::nullopt) {
        m_buffer_reads.emplace_back(std::move(buffer), shader_stage);
        return *this;
    }

    /// Specifies that this pass reads from a texture
    /// @param texture The texture the pass reads from
    /// @param shader_stage The shader stage the texture is read from
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &reads_from_texture(std::weak_ptr<Texture> texture,
                                           std::optional<VkShaderStageFlagBits> shader_stage = std::nullopt) {
        m_texture_reads.emplace_back(std::move(texture), shader_stage);
        return *this;
    }

    /// Specifies that this pass writes to a texture
    /// @param texture The texture the pass writes to
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &writes_to_texture(std::weak_ptr<Texture> texture) {
        m_texture_writes.emplace_back(texture);
        return *this;
    }

    /// Set the clear status for the stage
    /// @param clear_value The clear value for color and depth
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &set_clear_value(VkClearValue clear_value) {
        m_clear_value = clear_value;
        return *this;
    }

    /// Enable or disable depth testing
    /// @param depth_test ``true`` if depth testing is enabled for this stage
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &set_depth_test(bool depth_test) {
        m_depth_test = depth_test;
        return *this;
    }

    /// Set the function which will be called when the stage's command buffer is being recorded
    /// @param on_record The function which will be called when the stage's command buffer is being recorded
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] auto &set_on_record(std::function<void(const wrapper::commands::CommandBuffer &)> on_record) {
        m_on_record = std::move(on_record);
        return *this;
    }
};

} // namespace inexor::vulkan_renderer::render_graph
