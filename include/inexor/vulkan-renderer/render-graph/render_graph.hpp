#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/push_constant_range_resource.hpp"

#include <volk.h>

#include <spdlog/spdlog.h>

#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
/// Forward declarations
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

// Forward declarations
enum class BufferUsage;
enum class DescriptorSetUpdateFrequencyCategory;
class BufferResource;
class RenderStage;
class GraphicsStage;
class PushConstantRangeResource;

/// A rendergraph is a generic solution for rendering
/// This is based on Yuriy O'Donnell's talk "FrameGraph: Extensible Rendering Architecture in Frostbite" from GDC 2017
/// Also check out Hans-Kristian Arntzen's blog post "Render graphs and Vulkan - a deep dive" (2017) and
/// Adam Sawicki's talk "Porting your engine to Vulkan or DX12" (2018)
class RenderGraph {
private:
    const wrapper::Device &m_device;

    // The rendergraph has its own logger
    std::shared_ptr<spdlog::logger> m_log{spdlog::default_logger()->clone("render-graph")};

    // The buffer resources of the rendergraph (vertex-, index-, and uniform buffers)
    // Note that we must keep the data as std::vector of std::unique_ptr in order to keep entries consistent
    std::vector<std::unique_ptr<BufferResource>> m_buffer_resources;
    std::vector<std::unique_ptr<PushConstantRangeResource>> m_push_constant_ranges;

    // TODO: Texture resources go here
    // std::vector<std::unique_ptr<TextureResource>> m_texture_resources;

    // TODO: Graphics pipelines go here
    // TODO: Stages go here
    // TODO: Support compute pipelines and compute stages

    /// Build the graphics pipeline of a certain render stage
    /// @param stage The stage to build the renderpass for
    void build_graphics_pipeline(const RenderStage *stage);

    // TODO: Support for compute pipelines

    /// Build the renderpass of a certain render stage
    /// @param stage The stage to build the renderpass for
    void build_render_pass(const RenderStage *stage);

    /// It is essential that the render graph is acyclic, meaning it must not have a cycle in it!
    /// @exception std::logic_error The rendergraph is not acyclic
    void check_for_cycles();

    /// Create the physical buffers of every buffer resource in the rendergraph
    void create_buffers();

    void create_graphics_stages();

    void create_textures();

    /// Determine the order of execution of the graphics stages
    void determine_stage_order();

    void record_command_buffer();

    /// Update the vertex-, index-, and uniform-buffers
    /// @note If a uniform buffer has been updated, an update of the associated descriptor set will be performed
    void update_buffers();

    /// Update the descriptor sets
    void update_descriptor_sets();

    /// Update the push constant ranges
    void update_push_constant_ranges();

public:
    /// Default constructor
    /// @param device The device wrapper
    explicit RenderGraph(const wrapper::Device &device);

    RenderGraph(const RenderGraph &) = delete;
    RenderGraph(RenderGraph &&) noexcept;
    ~RenderGraph() = default;

    RenderGraph &operator=(const RenderGraph &) = delete;
    RenderGraph &operator=(RenderGraph &&) = delete;

    /// Add a buffer resource to the rendergraph
    /// @param name The internal name of the buffer resource (must not be empty)
    /// @param usage The internal buffer usage of the buffer
    /// @param category The estimated descriptor set category depending on the update frequency of the buffer
    /// @note The update frequency of a buffer will be respected when grouping uniform buffers into descriptor sets
    /// @param on_update An buffer resource update function
    /// @exception std::runtime_error Internal debug name is empty
    /// @return A raw pointer to the buffer resource that was just created
    [[nodiscard]] BufferResource *add_buffer(std::string name, BufferUsage usage,
                                             DescriptorSetUpdateFrequencyCategory category,
                                             std::function<void()> on_update);

    /// Add a graphics stage to the rendergraph
    ///
    [[nodiscard]] GraphicsStage *add_graphics_stage();

    /// Add a push constant range resource to the rendergraph
    /// @tparam T The data type of the push constant range
    /// @param data A pointer to the data of the push constant range
    /// @param on_update The update function of the push constant range
    /// @param stage_flags The shader stage flags
    /// @param offset The offset in bytes (``0`` by default)
    /// @return The this pointer, allowing for methods to be chained as a builder pattern
    template <typename T>
    [[nodiscard]] RenderStage *add_push_constant_range(
        const T *data, std::function<void()> on_update = []() {},
        const VkShaderStageFlags stage_flags = VK_SHADER_STAGE_VERTEX_BIT, const std::uint32_t offset = 0) {
        m_push_constant_ranges.emplace_back(
            VkPushConstantRange{
                .stageFlags = stage_flags,
                .offset = offset,
                .size = sizeof(T),
            },
            data, std::move(on_update));
        return this;
    }

    /// Compile the rendergraph
    void compile();

    /// Render a frame with the rendergraph
    /// @param swapchain_img_index The index of the current image in the swapchain
    void render(std::uint32_t swapchain_img_index);

    /// Update the rendering data
    // TODO: Maybe do not expose this, but call is in render()
    void update_data();
};

} // namespace inexor::vulkan_renderer::render_graph
