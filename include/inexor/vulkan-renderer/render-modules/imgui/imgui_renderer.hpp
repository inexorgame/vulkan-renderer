#pragma once

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"

#include <glm/vec2.hpp>
#include <imgui.h>

#include <memory>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declarations
class Device;
class Shader;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::swapchains {
// Forward declaration
class Swapchain;
} // namespace inexor::vulkan_renderer::wrapper::swapchains

namespace inexor::vulkan_renderer::wrapper::descriptors {
// Forward declaration
class PerFrameDescriptorSets;
} // namespace inexor::vulkan_renderer::wrapper::descriptors

namespace inexor::vulkan_renderer::render_modules::imgui {

// @TODO: Simplify the using declarations! Which ones do we need?

// Using declarations
using render_graph::Buffer;
using render_graph::BufferType;
using render_graph::DebugLabelColor;
using render_graph::GraphicsPass;
using render_graph::GraphicsPipelineBuilder;
using render_graph::RenderGraph;
using render_graph::Texture;
using wrapper::Device;
using wrapper::Shader;
using wrapper::commands::CommandBuffer;
using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::DescriptorType;
using wrapper::descriptors::PerFrameDescriptorSets;
using wrapper::descriptors::WriteDescriptorSetBuilder;
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
