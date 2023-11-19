#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_stage.hpp"
#include "inexor/vulkan-renderer/render-graph/push_constant_range_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_update_frequency.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"

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
enum class BufferType;
class BufferResource;
class RenderStage;
class PushConstantRangeResource;

/// A rendergraph is a generic solution for rendering architecture
/// This is based on Yuriy O'Donnell's talk "FrameGraph: Extensible Rendering Architecture in Frostbite" from GDC 2017
/// Also check out Hans-Kristian Arntzen's blog post "Render graphs and Vulkan - a deep dive" (2017) and
/// Adam Sawicki's talk "Porting your engine to Vulkan or DX12" (2018)
class RenderGraph {
private:
    /// The device wrapper
    wrapper::Device &m_device;
    // The rendergraph has its own logger
    std::shared_ptr<spdlog::logger> m_log{spdlog::default_logger()->clone("render-graph")};

    /// Physical resources of the rendergraph
    /// The graphics stages of the rendergraph
    std::vector<std::shared_ptr<GraphicsStage>> m_graphics_stages;
    // The buffer resources of the rendergraph (vertex-, index-, and uniform buffers)
    // Note that we must keep the data as std::vector of std::unique_ptr in order to keep entries consistent
    std::vector<std::shared_ptr<BufferResource>> m_buffer_resources;
    /// The push constant resources of the rendergraph
    // TODO: Remember we need to squash all VkPushConstantRange of a stage into one std::vector in order to bind it!
    std::vector<std::shared_ptr<PushConstantRangeResource>> m_push_constant_ranges;
    /// The texture resources of the rendergraph
    std::vector<std::shared_ptr<TextureResource>> m_texture_resources;

    /// Descriptor management: For performance reasons, it is recommended to group descriptors into descriptor sets by
    /// update frequency. The descriptor sets below correspond to resource descriptos which do not change to frequently
    /// changed descriptors

    /// In this descriptor set, we keep resource descriptors which do not change frequently, such as static meshes,
    /// static textures, and static constant buffers. After an initial update of the descriptor set, it remains
    /// unchanged for most of the time.
    VkDescriptorSet m_static_descriptor_set{VK_NULL_HANDLE};
    /// In this descriptor set we keep resource descriptors which change once per frame.
    VkDescriptorSet m_per_frame_descriptor_set{VK_NULL_HANDLE};
    /// In this descriptor set we keep resource descriptors that change on a per-batch basis, meaning there could be a
    /// group of objects, while the resource descriptors stay constant within one batch. The descriptro set will be
    /// updated when switching to another batch. This is likely done several times in one frame.
    VkDescriptorSet m_per_batch_descriptor_set{VK_NULL_HANDLE};
    /// In this descriptor set we keep resource descriptors that changes multiple times per frame. This could be
    /// per-object data or per-instance data.
    VkDescriptorSet m_dynamic_descriptor_set{VK_NULL_HANDLE};

    // TODO: Support compute pipelines and compute stages
    // TODO: Graphics pipelines go here
    // TODO: Stages go here

    /// Build the graphics pipeline of a certain render stage
    /// @param stage The stage to build the renderpass for
    void build_graphics_pipeline(const RenderStage *stage);

    /// Build the renderpass of a certain render stage
    /// @param stage The stage to build the renderpass for
    void build_render_pass(const RenderStage *stage);

    /// The rendergraph must not have any cycles in it!
    /// @exception std::logic_error The rendergraph is not acyclic
    void check_for_cycles();

    /// Create the buffers of every buffer resource in the rendergraph
    void create_buffers();

    /// Create the textures of every texture resoruce in the rendergraph
    void create_textures();

    /// Determine the order of execution of the graphics stages
    void determine_stage_order();

    /// Record a certain command buffer of a stage
    /// @param graphics_stage The graphics stage to record the command buffer for
    /// @param cmd_buf The command buffer to record the stage into
    /// @param is_first_stage ``true`` if this is the first stage in the stage stack
    /// @param is_last_stage ``true`` if this is the last stage in the stage stack
    void record_command_buffer(std::shared_ptr<GraphicsStage> graphics_stage, const wrapper::CommandBuffer &cmd_buf,
                               bool is_first_stage, bool is_last_stage);

    /// Record all command buffers required for the stages
    void record_command_buffers();

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
    explicit RenderGraph(wrapper::Device &device);

    RenderGraph(const RenderGraph &) = delete;
    RenderGraph(RenderGraph &&) noexcept;
    ~RenderGraph() = default;

    RenderGraph &operator=(const RenderGraph &) = delete;
    RenderGraph &operator=(RenderGraph &&) = delete;

    /// Add a buffer resource to the rendergraph
    /// @param name The internal name of the buffer resource (must not be empty)
    /// @param type The internal buffer usage of the buffer
    /// @param category The estimated descriptor set category depending on the update frequency of the buffer
    /// @note The update frequency of a buffer will be respected when grouping uniform buffers into descriptor sets
    /// @param on_update An optional buffer resource update function (``std::nullopt`` by default)
    /// @exception std::runtime_error Internal debug name is empty
    /// @return A raw pointer to the buffer resource that was just created
    [[nodiscard]] std::weak_ptr<BufferResource>
    add_buffer(std::string name, BufferType type, DescriptorSetUpdateFrequency category,
               std::optional<std::function<void()>> on_update = std::nullopt);

    // TODO: Compute stages

    /// Add a new graphics stage to the rendergraph
    /// @param graphics_stage The graphics stage which will be added to the rendergraph
    [[nodiscard]] void add_graphics_stage(std::shared_ptr<GraphicsStage> graphics_stage);

    /// Add a push constant range resource to the rendergraph
    /// @tparam T The data type of the push constant range
    /// @param data A pointer to the data of the push constant range
    /// @param on_update The update function of the push constant range
    /// @param stage_flags The shader stage flags
    /// @param offset The offset in bytes (``0`` by default)
    /// @return The this pointer, allowing for methods to be chained as a builder pattern
    template <typename PushConstantDataType>
    [[nodiscard]] std::weak_ptr<RenderStage> add_push_constant_range(
        const PushConstantDataType *data, std::function<void()> on_update = []() {},
        const VkShaderStageFlags stage_flags = VK_SHADER_STAGE_VERTEX_BIT, const std::uint32_t offset = 0) {
        m_push_constant_ranges.emplace_back(
            VkPushConstantRange{
                .stageFlags = stage_flags,
                .offset = offset,
                .size = sizeof(PushConstantDataType),
            },
            data, std::move(on_update));
        return this;
    }

    /// Add a texture resource to the rendergraph
    /// @param name The name of the texture
    /// @param uage The texture usage inside of rendergraph
    /// @param format The image format of the texture
    /// @return A std::wek_ptr to the texture resource that was created
    [[nodiscard]] std::weak_ptr<TextureResource> add_texture(std::string name, TextureUsage usage, VkFormat format);

    /// Compile the rendergraph
    void compile();

    /// Render a frame with the rendergraph
    /// @param swapchain_img_index The index of the current image in the swapchain
    void render(std::uint32_t swapchain_img_index, const VkSemaphore *img_available);

    /// Update the rendering data
    // TODO: Maybe do not expose this, but call is in render()?
    void update_data();
};

} // namespace inexor::vulkan_renderer::render_graph
