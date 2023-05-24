#include "inexor/vulkan-renderer/render-graph/buffer_resource.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_stage.hpp"
#include "inexor/vulkan-renderer/render-graph/texture_resource.hpp"
#include "inexor/vulkan-renderer/wrapper/buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/image.hpp"

#include <spdlog/spdlog.h>

#include <memory>
#include <optional>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
/// Forward declarations
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_graph {

/// A rendergraph is a generic solution fo rendering
/// This is based on Yuriy O'Donnell's talk "FrameGraph: Extensible Rendering Architecture in Frostbite" from GDC 2017
/// Also check out Hans-Kristian Arntzen's blog post "Render graphs and Vulkan � a deep dive" (2017) and
/// Adam Sawicki's talk "Porting your engine to Vulkan or DX12" (2018)
class RenderGraph {
private:
    const wrapper::Device &m_device;

    // The rendergraph has its own logger
    std::shared_ptr<spdlog::logger> m_log{spdlog::default_logger()->clone("render-graph")};

    std::vector<std::unique_ptr<BufferResource>> m_buffer_resources;
    std::vector<std::unique_ptr<TextureResource>> m_texture_resources;

    // TODO: When creating the buffers, create them based on the on_update_buffer == std::nullopt?
    std::vector<std::unique_ptr<wrapper::Buffer>> m_constant_buffers;
    std::vector<std::unique_ptr<wrapper::Buffer>> m_dynamic_buffers;
    std::vector<std::unique_ptr<wrapper::Image>> m_textures;

    std::vector<std::unique_ptr<GraphicsStage>> m_graphics_stages;
    // TODO: Support compute pipelines and compute stages

    /// It is essential that the render graph is acyclic, meaning it must not have a cycle in it!
    /// @exception std::logic_error The rendergraph is not acyclic
    void check_for_cycles();
    void build_graphics_pipeline();
    void build_render_pass();
    void create_buffers();
    void create_graphics_stages();
    void create_textures();
    void determine_stage_order();
    void record_command_buffer();
    void update_dynamic_buffers();

public:
    /// Default constructor
    /// @param device The device wrapper
    explicit RenderGraph(const wrapper::Device &device);

    RenderGraph(const RenderGraph &) = delete;
    RenderGraph(RenderGraph &&) noexcept;
    ~RenderGraph() = default;

    RenderGraph &operator=(const RenderGraph &) = delete;
    RenderGraph &operator=(RenderGraph &&) = delete;

    /// Add a new buffer resource to the rendergraph. Note that doing so does not perform any allocation of a physical
    /// buffer yet. This will be done when compile() will be called.
    /// @param usage The internal buffer usage in the rendergraph
    /// @param size The size of the buffer in bytes (must be greater than 0!)
    /// @param name The internal debug name of the buffer (must not be empty!)
    /// @return A raw pointer to the buffer resource that was created during
    [[nodiscard]] BufferResource *add_buffer(const BufferUsage usage, const VkDeviceSize size, std::string name) {
        m_buffer_resources.emplace_back(std::make_unique<BufferResource>(usage, std::move(name), size));
        return m_buffer_resources.back().get();
    }

    /// Create a buffer of fixed size which has an update function
    /// @param usage The internal buffer usage in the rendergraph
    /// @param name The internal debug name of the buffer (must not be empty!)
    /// @return A raw pointer to the buffer resource that was created during
    template <typename BufferDataType>
    [[nodiscard]] BufferResource *add_buffer(const BufferUsage usage, std::string name) {
        return add_buffer(usage, sizeof(BufferDataType), std::move(name));
    }

    // TODO: Implement a texture update mechanism similar to buffer updates!

    /// Add a new texture resource to the rendergraph
    /// @param usage The internal usage of the texture in the rendergraph
    /// @param format The image format of the texture
    /// @param name The internal debug name of the texture
    [[nodiscard]] TextureResource *add_texture(const TextureUsage usage, const VkFormat format, std::string name) {
        m_texture_resources.emplace_back(std::make_unique<TextureResource>(usage, format, std::move(name)));
        return m_texture_resources.back().get();
    }

    /// Compile the rendergraph
    void compile();

    /// Recompile in case the rendergraph has been invalidated
    void recompile();

    /// Render a frame with the rendergraph
    /// @param swapchain_img_index The index of the current image in the swapchain
    void render(std::uint32_t swapchain_img_index);

    /// Update the rendering data
    void update_data();
};

} // namespace inexor::vulkan_renderer::render_graph
