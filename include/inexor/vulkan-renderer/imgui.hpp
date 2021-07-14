#pragma once

#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <glm/vec2.hpp>
#include <imgui.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

class Device;
class Swapchain;

} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer {

class ImGUIOverlay {
    const wrapper::Device &m_device;
    const wrapper::Swapchain &m_swapchain;
    float m_scale{1.0f};

    BufferResource *m_index_buffer{nullptr};
    BufferResource *m_vertex_buffer{nullptr};
    GraphicsStage *m_stage{nullptr};

    std::unique_ptr<wrapper::GpuTexture> m_imgui_texture;
    std::unique_ptr<wrapper::Shader> m_vertex_shader;
    std::unique_ptr<wrapper::Shader> m_fragment_shader;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;
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
    ImGUIOverlay(const wrapper::Device &device, const wrapper::Swapchain &swapchain, RenderGraph *render_graph,
                 TextureResource *back_buffer);
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
