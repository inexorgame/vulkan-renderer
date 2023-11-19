#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_stage.hpp"
#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <functional>
#include <memory>
#include <utility>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class CommandBuffer;
}

namespace inexor::vulkan_renderer::render_graph {

/// A builder class for graphics stages in the rendergraph
class GraphicsStageBuilder {
private:
    /// Indicates if the screen is cleared at the beginning of this stage
    std::optional<VkClearValue> m_clear_value;
    /// Add members which describe data related to graphics stages here
    std::function<void(const wrapper::CommandBuffer &)> m_on_record;
    /// Depth testing
    bool m_depth_test;

    /// The buffers the graphics stage reads from
    /// If the buffer's ``BufferType`` is ``UNIFORM_BUFFER``, a value for the shader stage flag must be specified,
    /// because uniform buffers can be read from vertex or fragment stage bit.
    std::vector<std::pair<std::weak_ptr<BufferResource>, std::optional<VkShaderStageFlagBits>>> m_buffer_reads;
    /// The textures the graphics stage reads from
    std::vector<std::pair<std::weak_ptr<TextureResource>, std::optional<VkShaderStageFlagBits>>> m_texture_reads;
    /// The textures the graphics stage writes to
    std::vector<std::weak_ptr<TextureResource>> m_texture_writes;
    /// The push constant ranges of the graphics stage
    std::vector<std::pair<VkPushConstantRange, std::function<void()>>> m_push_constant_ranges;

    // TODO: Merge push constant ranges into one block and put it as member here?
    // TODO: Copy all data into one piece of memory and call vkCmdPushConstants only once! (ChatGPT says yes to this)
    void compile_push_constants();

    /*
        [[nodiscard]] GraphicsStageBuilder &finalize_push_constants(VkCommandBuffer command_buffer) {
            // Calculate the total size needed for push constants
            std::size_t total_size = 0;
            for (const auto &push_constant_range : m_push_constant_ranges) {
                total_size += push_constant_range.first.size;
            }

            // Allocate a single block of memory to hold all push constant data
            std::vector<std::uint8_t> push_constants_data(total_size);

            // Loop through push constant ranges and copy data into the combined memory block
            std::size_t offset = 0;
            for (const auto &push_constant_range : m_push_constant_ranges) {
                std::memcpy(push_constants_data.data() + offset,
                            reinterpret_cast<const void *>(&push_constant_range.second),
                            push_constant_range.first.size);
                offset += push_constant_range.first.size;
            }

            // Loop through push constant ranges and calculate offsets and stage flags
            std::vector<VkPushConstantRange> vk_push_constant_ranges;
            std::vector<VkShaderStageFlags> stage_flags;
            std::size_t vk_offset = 0;
            for (const auto &push_constant_range : m_push_constant_ranges) {
                vk_push_constant_ranges.push_back({
                    .stageFlags = push_constant_range.first.stageFlags,
                    .offset = vk_offset,
                    .size = push_constant_range.first.size,
                });

                // Track shader stages for combining stage flags
                stage_flags.push_back(push_constant_range.first.stageFlags);

                vk_offset += push_constant_range.first.size;
            }

            // Combine all shader stages
            VkShaderStageFlags combined_stage_flags = 0;
            for (auto stage : stage_flags) {
                combined_stage_flags |= stage;
            }

            // Issue a single vkCmdPushConstants call
            vkCmdPushConstants(command_buffer,
                               /* pipelineLayout / your_pipeline_layout,
                               combined_stage_flags,
                               0, // Start offset in combined data block
                               total_size,
                               push_constants_data.data());

            // Optionally, clear the push constant ranges if you don't need them anymore
            m_push_constant_ranges.clear();

            return *this;
        }
    */

    /// Reset all data of the graphics stage builder
    void reset();

public:
    GraphicsStageBuilder();
    GraphicsStageBuilder(const GraphicsStageBuilder &) = delete;
    GraphicsStageBuilder(GraphicsStageBuilder &&) noexcept;
    ~GraphicsStageBuilder() = default;

    GraphicsStageBuilder &operator=(const GraphicsStageBuilder &) = delete;
    GraphicsStageBuilder &operator=(GraphicsStageBuilder &&) noexcept;

    /// Add a push constant range to the graphics stage
    /// @param shader_stage The shader stage for the push constant range
    /// @param push_constant The push constant data
    /// @param on_update The update callable
    /// @param offset The offset in the push constant range
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    template <typename PushConstantDataType>
    [[nodiscard]] GraphicsStageBuilder &
    add_push_constant_range(VkShaderStageFlags shader_stage, const PushConstantDataType &push_constant,
                            std::function<void()> on_update, std::uint32_t offset = 0) {
        m_push_constant_ranges.push_back(std::make_pair(
            VkPushConstantRange{
                .stageFlags = shader_stage,
                .offset = offset,
                .size = sizeof(PushConstantDataType),
            },
            std::move(on_update)));
        return *this;
    }

    /// Build the graphics stage and reset the builder's data
    /// @param name The name of the graphics stage
    /// @return The graphics stage which was created
    [[nodiscard]] std::shared_ptr<GraphicsStage> build(std::string name);

    [[nodiscard]] GraphicsStageBuilder &
    reads_from_buffer(std::weak_ptr<BufferResource> buffer,
                      std::optional<VkShaderStageFlagBits> shader_stage = std::nullopt) {
        m_buffer_reads.push_back(std::make_pair(std::move(buffer), shader_stage));
        return *this;
    }

    [[nodiscard]] GraphicsStageBuilder &
    reads_from_texture(std::weak_ptr<TextureResource> texture,
                       std::optional<VkShaderStageFlagBits> shader_stage = std::nullopt) {
        m_texture_reads.push_back(std::make_pair(std::move(texture), shader_stage));
        return *this;
    }

    [[nodiscard]] GraphicsStageBuilder &writes_to_texture(std::weak_ptr<TextureResource> texture) {
        m_texture_writes.push_back(std::move(texture));
        return *this;
    }

    /// Set the clear status for the stage
    /// @param clear_value The clear value for color and depth
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsStageBuilder &set_clear_value(VkClearValue clear_value);

    /// Enable or disable depth testing
    /// @param depth_test ``true`` if depth testing is enabled for this stage
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsStageBuilder &set_depth_test(bool depth_test);

    /// Set the function which will be called when the stage's command buffer is being recorded
    /// @param on_record The function which will be called when the stage's command buffer is being recorded
    /// @return A const reference to the this pointer (allowing method calls to be chained)
    [[nodiscard]] GraphicsStageBuilder &set_on_record(std::function<void(const wrapper::CommandBuffer &)> on_record);
};

} // namespace inexor::vulkan_renderer::render_graph
