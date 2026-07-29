#include "inexor/vulkan-renderer/render-modules/imgui/imgui_renderer.hpp"

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/per_frame_descriptor_sets.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

namespace inexor::vulkan_renderer::render_modules::imgui {

// Using declarations for types only used in the implementation
using render_graph::BufferType;
using render_graph::DebugLabelColor;
using render_graph::GraphicsPipelineBuilder;
using wrapper::commands::CommandBuffer;
using wrapper::core::Device;
using wrapper::descriptors::DescriptorSetAllocator;
using wrapper::descriptors::DescriptorSetLayoutBuilder;
using wrapper::descriptors::DescriptorType;
using wrapper::descriptors::WriteDescriptorSetBuilder;

ImGuiRenderer::ImGuiRenderer(std::shared_ptr<RenderGraph> render_graph, std::weak_ptr<Swapchain> swapchain,
                             std::function<void()> on_update_user_imgui_data)
    : m_swapchain(swapchain), m_on_update_user_imgui_data(std::move(on_update_user_imgui_data)) {
    spdlog::trace("Creating ImGUI context");
    ImGui::CreateContext();

    ImGuiStyle &style = ImGui::GetStyle();
    style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
    style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
    style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
    style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

    ImGuiIO &io = ImGui::GetIO();
    io.FontGlobalScale = 1.0f;

    spdlog::trace("Loading ImGUI shaders");
    m_vertex_shader =
        std::make_shared<Shader>(render_graph->device(), VK_SHADER_STAGE_VERTEX_BIT, "shaders/ui.vert.spv");
    m_fragment_shader =
        std::make_shared<Shader>(render_graph->device(), VK_SHADER_STAGE_FRAGMENT_BIT, "shaders/ui.frag.spv");

    // Load font texture

    // @TODO Move this data into a container class; have container class also support bold and italic.
    constexpr const char *FONT_FILE_PATH = "assets/fonts/NotoSans-Bold.ttf";
    constexpr float FONT_SIZE = 18.0f;

    spdlog::trace("Loading front {}", FONT_FILE_PATH);

    // NOTE: We do not need to free this pointer because the memory is freed by ImGui internally again
    // This illustrates another reason why to use smart pointers instead nowadays: memory ownership clarity
    ImFont *font = io.Fonts->AddFontFromFileTTF(FONT_FILE_PATH, FONT_SIZE);
    io.Fonts->GetTexDataAsRGBA32(&m_font_texture_data, &m_font_texture_width, &m_font_texture_height);

    if (font == nullptr || m_font_texture_data == nullptr) {
        spdlog::error("Unable to load font {}. Falling back to error texture", FONT_FILE_PATH);
        // @TODO generate error texture for rendergraph!
    } else {
        spdlog::trace("Creating ImGUI font texture");

        // Our font textures always have 4 channels and a single mip level by definition.
        constexpr int FONT_TEXTURE_CHANNELS{4};
        constexpr int FONT_MIP_LEVELS{1};

        m_upload_size = static_cast<VkDeviceSize>(m_font_texture_width) *
                        static_cast<VkDeviceSize>(m_font_texture_height) *
                        static_cast<VkDeviceSize>(FONT_TEXTURE_CHANNELS);

        m_imgui_texture = render_graph->add_texture(
            "ImGui|Texture", render_graph::TextureUsage::DEFAULT, VK_FORMAT_R8G8B8A8_UNORM, m_font_texture_width,
            m_font_texture_height, FONT_TEXTURE_CHANNELS, VK_SAMPLE_COUNT_1_BIT, [&]() {
                // Make sure the ImGui font texture is only updated once!
                if (!m_imgui_font_texture_initialized2) {
                    m_imgui_texture.lock()->request_update(m_font_texture_data, m_upload_size);
                    m_imgui_font_texture_initialized2 = true;
                }
            });
    }

    m_descriptor_set = render_graph->add_resource_descriptor(m_imgui_texture, VK_SHADER_STAGE_FRAGMENT_BIT);

    // ImGui vertex/index data is rewritten in full every frame, so these buffers use per-frame-slot,
    // host-visible mapped memory (one backing allocation per frame-in-flight) instead of a single
    // device-local buffer. This avoids a staging-buffer copy and transfer barrier every frame, and avoids
    // writing into memory the GPU might still be reading from a previous frame.
    m_vertex_buffer = render_graph->add_buffer(
        "ImGui vertices", BufferType::VERTEX_BUFFER,
        [&]() {
            ImDrawData *imgui_draw_data = ImGui::GetDrawData();
            if (!imgui_draw_data || imgui_draw_data->TotalVtxCount == 0) {
                return;
            }
            m_vertex_data.clear();
            m_index_data.clear();
            for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
                const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i];
                m_vertex_data.insert(m_vertex_data.end(), cmd_list->VtxBuffer.Data,
                                     cmd_list->VtxBuffer.Data + cmd_list->VtxBuffer.Size);
                m_index_data.insert(m_index_data.end(), cmd_list->IdxBuffer.Data,
                                    cmd_list->IdxBuffer.Data + cmd_list->IdxBuffer.Size);
            }
            if (!m_vertex_data.empty()) {
                m_vertex_buffer.lock()->request_update(m_vertex_data);
            }
        },
        render_graph::BufferUpdateMode::PER_FRAME_HOST_VISIBLE);

    m_index_buffer = render_graph->add_buffer(
        "ImGui indices", BufferType::INDEX_BUFFER,
        [&]() {
            if (!m_index_data.empty()) {
                m_index_buffer.lock()->request_update(m_index_data);
            }
        },
        render_graph::BufferUpdateMode::PER_FRAME_HOST_VISIBLE);

    // Add the ImGui graphics pipeline to rendergraph
    render_graph->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        const auto swapchain = m_swapchain.lock();
        const auto descriptor_set = m_descriptor_set.lock();
        m_imgui_pipeline = builder
                               .set_vertex_input_bindings({
                                   {
                                       .binding = 0,
                                       .stride = sizeof(ImDrawVert),
                                       .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
                                   },
                               })
                               .set_vertex_input_attributes({
                                   {
                                       .location = 0,
                                       .format = VK_FORMAT_R32G32_SFLOAT,
                                       .offset = offsetof(ImDrawVert, pos),
                                   },
                                   {
                                       .location = 1,
                                       .format = VK_FORMAT_R32G32_SFLOAT,
                                       .offset = offsetof(ImDrawVert, uv),
                                   },
                                   {
                                       .location = 2,
                                       .format = VK_FORMAT_R8G8B8A8_UNORM,
                                       .offset = offsetof(ImDrawVert, col),
                                   },
                               })
                               .add_standard_alpha_blend_attachment()
                               .add_color_attachment_format(swapchain->image_format())
                               .set_dynamic_scissor()
                               .set_dynamic_viewport()
                               .add_shader(m_vertex_shader)
                               .add_shader(m_fragment_shader)
                               .set_descriptor_set_layout(descriptor_set->layout())
                               .add_descriptor_set(m_descriptor_set)
                               .add_push_constant_range(VK_SHADER_STAGE_VERTEX_BIT, sizeof(m_push_const_block))
                               .build("ImGui");
    });

    using render_graph::GraphicsPassBuilder;

    // Add the ImGui graphics pass to rendergraph
    m_imgui_pass = render_graph->add_graphics_pass([&](GraphicsPassBuilder &builder) {
        return builder.writes_to(swapchain)
            .reads_from(m_vertex_buffer)
            .reads_from(m_index_buffer)
            .set_on_record([&](const CommandBuffer &cmd_buf) {
                ImDrawData *draw_data = ImGui::GetDrawData();
                if (draw_data == nullptr || draw_data->TotalVtxCount == 0 || draw_data->TotalIdxCount == 0) {
                    return;
                }

                const auto vertex_buffer = m_vertex_buffer.lock();
                const auto index_buffer = m_index_buffer.lock();
                if (!vertex_buffer || !index_buffer || vertex_buffer->buffer() == VK_NULL_HANDLE ||
                    index_buffer->buffer() == VK_NULL_HANDLE) {
                    // During resize/rebuild there can be a transient frame where ImGui draw data exists
                    // but GPU buffers are not uploaded yet. Skip recording in that case.
                    return;
                }

                const ImGuiIO &io = ImGui::GetIO();
                m_push_const_block.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);

                cmd_buf.bind_pipeline(m_imgui_pipeline)
                    .bind_descriptor_set(m_descriptor_set, m_imgui_pipeline)
                    .push_constant(m_imgui_pipeline, m_push_const_block, VK_SHADER_STAGE_VERTEX_BIT)
                    .bind_vertex_buffer(m_vertex_buffer)
                    .bind_index_buffer(m_index_buffer)
                    .set_viewport({
                        .width = ImGui::GetIO().DisplaySize.x,
                        .height = ImGui::GetIO().DisplaySize.y,
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f,
                    });

                std::uint32_t index_offset = 0;
                std::int32_t vertex_offset = 0;
                for (std::size_t i = 0; i < draw_data->CmdListsCount; i++) {
                    const ImDrawList *cmd_list = draw_data->CmdLists[i];
                    for (std::int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                        const ImDrawCmd &draw_cmd = cmd_list->CmdBuffer[j];
                        cmd_buf
                            .set_scissor({
                                .offset{
                                    .x = std::max(static_cast<int32_t>(draw_cmd.ClipRect.x), 0),
                                    .y = std::max(static_cast<int32_t>(draw_cmd.ClipRect.y), 0),
                                },
                                .extent{
                                    .width = static_cast<uint32_t>(draw_cmd.ClipRect.z - draw_cmd.ClipRect.x),
                                    .height = static_cast<uint32_t>(draw_cmd.ClipRect.w - draw_cmd.ClipRect.y),
                                },
                            })
                            .draw_indexed(draw_cmd.ElemCount, 1, index_offset, vertex_offset);
                        index_offset += draw_cmd.ElemCount;
                    }
                    vertex_offset += cmd_list->VtxBuffer.Size;
                }
            })
            .build("ImGui", DebugLabelColor::BLUE);
    });
}

ImGuiRenderer::~ImGuiRenderer() {
    ImGui::DestroyContext();
}

} // namespace inexor::vulkan_renderer::render_modules::imgui
