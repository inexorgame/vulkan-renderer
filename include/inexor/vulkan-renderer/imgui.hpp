#pragma once

#include "inexor/vulkan-renderer/gpu_data_base.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"
#include "inexor/vulkan-renderer/wrapper/shader_loader.hpp"

#include <glm/vec2.hpp>
#include <imgui.h>
#include <vulkan/vulkan_core.h>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration
class Device;
class Swapchain;

} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer {

class ImGUIOverlay : public GpuDataBase<ImDrawVert> {
private:
    const wrapper::Device &m_device;
    const wrapper::Swapchain &m_swapchain;
    float m_scale{1.0f};

    std::vector<vk_tools::VertexAttributeLayout> m_vertex_attribute_layout{
        {VK_FORMAT_R32G32_SFLOAT, sizeof(ImDrawVert::pos), offsetof(ImDrawVert, pos)},
        {VK_FORMAT_R32G32_SFLOAT, sizeof(ImDrawVert::uv), offsetof(ImDrawVert, uv)},
        {VK_FORMAT_R8G8B8A8_UNORM, sizeof(ImDrawVert::col), offsetof(ImDrawVert, col)},
    };

    GraphicsStage *m_stage{nullptr};

    std::unique_ptr<texture::GpuTexture> m_imgui_texture;

    std::unique_ptr<wrapper::ResourceDescriptor> m_descriptor;

    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate;
    } m_push_const_block{};

    const std::vector<wrapper::ShaderLoaderJob> m_shader_files{
        {"shaders/imgui/ui.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "ImGUI vertex shader"},
        {"shaders/imgui/ui.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "ImGUI fragment shader"}};

    wrapper::ShaderLoader m_shader_loader;

    void setup_rendering_resources(RenderGraph *render_graph, TextureResource *back_buffer);

public:
    ImGUIOverlay(RenderGraph *render_graph, const wrapper::Swapchain &swapchain, TextureResource *back_buffer);

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
