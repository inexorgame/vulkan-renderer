#pragma once

#include <glm/vec2.hpp>
#include <imgui.h>
#include <volk.h>

#include <functional>
#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Shader;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::swapchains {
// Forward declaration
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper::swapchains

namespace inexor::vulkan_renderer::wrapper::descriptors {
// Forward declaration
class PerFrameDescriptorSets;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::render_graph {
// Forward declarations
class Buffer;
class GraphicsPass;
class RenderGraph;
class Texture;
} // namespace inexor::vulkan_renderer::render_graph

namespace inexor::vulkan_renderer::wrapper::pipelines {
// Forward declaration
class GraphicsPipeline;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_modules::imgui {

// Using declarations - only those needed by header declarations
using render_graph::Buffer;
using render_graph::GraphicsPass;
using render_graph::RenderGraph;
using render_graph::Texture;
using wrapper::Shader;
using wrapper::descriptors::PerFrameDescriptorSets;
using wrapper::pipelines::GraphicsPipeline;
using wrapper::swapchains::Swapchain;

// ImGui user interface integration
class ImGuiRenderer {
private:
    std::weak_ptr<Buffer> m_vertex_buffer;
    std::weak_ptr<Buffer> m_index_buffer;
    std::weak_ptr<GraphicsPass> m_imgui_pass;
    std::weak_ptr<Texture> m_imgui_texture;
    std::shared_ptr<GraphicsPipeline> m_imgui_pipeline;
    std::weak_ptr<Swapchain> m_swapchain;
    std::weak_ptr<PerFrameDescriptorSets> m_descriptor_set;

    bool m_imgui_font_texture_initialized2{false};
    VkDeviceSize m_upload_size{0};
    unsigned char *m_font_texture_data{};
    int m_font_texture_width{0};
    int m_font_texture_height{0};

    // The external user-defined ImGui update function
    std::function<void()> m_on_update_user_imgui_data;

    std::shared_ptr<Shader> m_vertex_shader;
    std::shared_ptr<Shader> m_fragment_shader;

    std::vector<std::uint32_t> m_index_data;
    std::vector<ImDrawVert> m_vertex_data;

    struct PushConstBlock {
        glm::vec2 scale;
        glm::vec2 translate{glm::vec2(-1.0f)};
    } m_push_const_block{};

public:
    /// Constructor
    /// @param render_graph
    /// @param swapchain
    /// @param on_update_user_imgui_data
    ImGuiRenderer(std::shared_ptr<RenderGraph> render_graph, std::weak_ptr<Swapchain> swapchain,
                  std::function<void()> on_update_user_imgui_data);

    ~ImGuiRenderer();
};

} // namespace inexor::vulkan_renderer::render_modules::imgui
