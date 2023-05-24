#pragma once

#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <glm/vec2.hpp>
#include <imgui.h>
#include <volk.h>

#include <memory>
#include <vector>

// Forward declarations
namespace inexor::vulkan_renderer::wrapper {
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer {

class ImGUIOverlay {
    const wrapper::Device &m_device;

    BufferResource *m_index_buffer{nullptr};
    BufferResource *m_vertex_buffer{nullptr};
    GraphicsStage *m_stage{nullptr};

    std::unique_ptr<wrapper::GpuTexture> m_imgui_texture;
    wrapper::Shader m_vertex_shader;
    wrapper::Shader m_fragment_shader;
    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;
    std::vector<std::uint32_t> m_index_data;
    std::vector<ImDrawVert> m_vertex_data;

    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate;
    } m_push_const_block{};

    std::function<void()> m_update_overlay{[]() {}};

public:
    /// @brief Construct a new ImGUI overlay.
    /// @param device A reference to the device wrapper
    /// @param render_graph A pointer to the render graph
    /// @param back_buffer A pointer to the target of the ImGUI rendering
    ImGUIOverlay(
        const wrapper::Device &device, RenderGraph *render_graph, TextureResource *back_buffer,
        std::function<void()> update_overlay = []() {});
    ImGUIOverlay(const ImGUIOverlay &) = delete;
    ImGUIOverlay(ImGUIOverlay &&) = delete;
    ~ImGUIOverlay();

    ImGUIOverlay &operator=(const ImGUIOverlay &) = delete;
    ImGUIOverlay &operator=(ImGUIOverlay &&) = delete;
};

} // namespace inexor::vulkan_renderer
