#pragma once

#include "inexor/vulkan-renderer/imgui.hpp"
#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor.hpp"

#include <glm/vec2.hpp>
#include <imgui.h>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declarations
class Device;
class GpuTexture;
class Shader;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::swapchains {
// Forward declaration
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper::swapchains

namespace inexor::vulkan_renderer {

// Using declaration
using wrapper::Device;
using wrapper::swapchains::Swapchain;

class ImGUIOverlay {
    const Device &m_device;
    const Swapchain &m_swapchain;
    float m_scale{1.0f};

    // RENDERGRAPH2
    std::weak_ptr<inexor::vulkan_renderer::render_graph::Buffer> m_vertex_buffer2;
    std::weak_ptr<inexor::vulkan_renderer::render_graph::Buffer> m_index_buffer2;
    std::weak_ptr<inexor::vulkan_renderer::render_graph::GraphicsPass> m_imgui_pass2;
    VkDescriptorSetLayout m_descriptor_set_layout2{VK_NULL_HANDLE};
    VkDescriptorSet m_descriptor_set2{VK_NULL_HANDLE};
    std::weak_ptr<inexor::vulkan_renderer::render_graph::Texture> m_imgui_texture2;
    bool m_imgui_font_texture_initialized2{false};
    VkDeviceSize m_upload_size{0};
    unsigned char *m_font_texture_data{};
    int m_font_texture_width{0};
    int m_font_texture_height{0};
    std::shared_ptr<GraphicsPipeline> m_imgui_pipeline2;

    BufferResource *m_index_buffer{nullptr};
    BufferResource *m_vertex_buffer{nullptr};
    GraphicsStage *m_stage{nullptr};

    std::unique_ptr<wrapper::GpuTexture> m_imgui_texture;
    std::unique_ptr<wrapper::Shader> m_vertex_shader;
    std::unique_ptr<wrapper::Shader> m_fragment_shader;
    std::unique_ptr<wrapper::descriptors::ResourceDescriptor> m_descriptor;
    std::vector<std::uint32_t> m_index_data;
    std::vector<ImDrawVert> m_vertex_data;

    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate;
    } m_push_const_block{};

public:
    /// @brief Construct a new ImGUI overlay.
    /// @param device A reference to the device wrapper
    /// @param swapchain A reference to the swapchain
    /// @param render_graph A pointer to the render graph
    /// @param back_buffer A pointer to the target of the ImGUI rendering
    /// @param render_graph2
    ImGUIOverlay(const Device &device, const Swapchain &swapchain, RenderGraph *render_graph,
                 TextureResource *back_buffer, std::shared_ptr<render_graph::RenderGraph> render_graph2);
    ImGUIOverlay(const ImGUIOverlay &) = delete;
    ImGUIOverlay(ImGUIOverlay &&) = delete;
    ~ImGUIOverlay();

    ImGUIOverlay &operator=(const ImGUIOverlay &) = delete;
    ImGUIOverlay &operator=(ImGUIOverlay &&) = delete;

    void update();

    [[nodiscard]] float scale() const {
        return m_scale;
    }
};

} // namespace inexor::vulkan_renderer
