#pragma once

#include "inexor/vulkan-renderer/wrapper/shader.hpp"

#include <glm/vec2.hpp>
#include <imgui.h>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::render_components {

/// A renderer for ImGui
class ImGuiRenderer {
private:
    ImDrawData *imgui_draw_data{nullptr};

    // The vertex shader and fragment shader for ImGui
    wrapper::Shader m_vertex_shader;
    wrapper::Shader m_fragment_shader;

    // The vertex buffer and index buffer resource for ImGui
    render_graph::BufferResource *m_vertex_buffer{nullptr};
    render_graph::BufferResource *m_index_buffer{nullptr};

    std::vector<ImDrawVert> m_vertex_data;
    std::vector<std::uint32_t> m_index_data;

    struct PushConstBlock {
        glm::vec2 scale{};
        glm::vec2 translate{};
    } m_push_const_block{};

    void initialize_imgui();

    // TODO: Better name?
    void update_imgui_windows();

public:
    /// Default constructor
    /// @param device The device wrapper
    /// @param render_graph The render graph
    explicit ImGuiRenderer(const wrapper::Device &device, render_graph::RenderGraph *render_graph);
    ImGuiRenderer(const ImGuiRenderer &) = delete;
    ImGuiRenderer(ImGuiRenderer &&) = delete;
    ~ImGuiRenderer() override = default;

    ImGuiRenderer &operator=(const ImGuiRenderer &) = delete;
    ImGuiRenderer &operator=(ImGuiRenderer &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_components
