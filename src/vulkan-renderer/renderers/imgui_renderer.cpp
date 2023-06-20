#include "inexor/vulkan-renderer/renderer-components/imgui_renderer.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::renderer_components {

ImGuiRenderer::ImGuiRenderer(const wrapper::Device &device, render_graph::RenderGraph *render_graph)
    // Load the vertex shader and fragment shader for ImGui rendering
    : m_vertex_shader(m_device, VK_SHADER_STAGE_VERTEX_BIT, "ImGUI", "shaders/ui.vert.spv"),
      m_fragment_shader(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "ImGUI", "shaders/ui.frag.spv") {

    initialize_imgui();

    spdlog::trace("Setting graphics stage for ImGui");

    // Create the vertex and index buffer resource for ImGui in the rendergraph
    m_vertex_buffer = render_graph->add_buffer(render_graph::BufferUsage::VERTEX_BUFFER, "ImGui");
    m_index_buffer = render_graph->add_buffer(render_graph::BufferUsage::INDEX_BUFFER, "ImGui");

    // Give me the graphics stage builder
    const auto &builder = render_graph->graphics_stage_builder();

    // TODO: Abstract descriptor builder into rendergraph!

    // Create the graphics stage for ImGui
    render_graph->add_stage(
        builder.uses_shader(m_vertex_shader)
            .uses_shader(m_fragment_shader)
            .add_push_constant_block<PushConstBlock>()
            .reads_from(m_vertex_buffer)
            .reads_from(m_index_buffer)
            .writes_to(depth_buffer)
            .writes_to(back_buffer)
            .set_vertex_attribute_layout({
                {VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos)},
                {VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv)},
                {VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col)},
            })
            .set_on_record([&](const wrapper::CommandBuffer &cmd_buf) {
                // Adjust the push constant block according to the window size
                const ImGuiIO &io = ImGui::GetIO();
                m_push_const_block.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
                m_push_const_block.translate = glm::vec2(-1.0f);

                // TODO: Move push constants into rendergraph?
                cmd_buf.bind_descriptor_sets(m_descriptor->descriptor_sets(), physical.pipeline_layout());
                cmd_buf.push_constants(physical.pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock),
                                       &m_push_const_block);

                std::uint32_t index_offset = 0;
                std::int32_t vertex_offset = 0;
                for (std::size_t i = 0; i < m_imgui_draw_data->CmdListsCount; i++) {
                    const ImDrawList *cmd_list = m_imgui_draw_data->CmdLists[i]; // NOLINT
                    for (std::int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                        const ImDrawCmd &draw_cmd = cmd_list->CmdBuffer[j];
                        vkCmdDrawIndexed(cmd_buf.get(), draw_cmd.ElemCount, 1, index_offset, vertex_offset, 0);
                        index_offset += draw_cmd.ElemCount;
                    }
                    vertex_offset += cmd_list->VtxBuffer.Size;
                }
            })
            .set_on_update([&]() {
                //
                m_imgui_draw_data = ImGui::GetDrawData();
                if (m_imgui_draw_data == nullptr) {
                    return;
                }
                if (m_imgui_draw_data->TotalVtxCount != 0) {
                    m_update_vertices = true;
                }
                if (m_imgui_draw_data->TotalIdxCount != 0) {
                    m_update_indices = true;
                }

                if (m_vertex_data.size() != imgui_draw_data->TotalVtxCount) {
                    m_vertex_data.clear();
                    for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
                        const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i]; // NOLINT
                        for (std::size_t j = 0; j < cmd_list->VtxBuffer.Size; j++) {
                            m_vertex_data.push_back(cmd_list->VtxBuffer.Data[j]); // NOLINT
                        }
                    }
                }

                if (m_index_data.size() != imgui_draw_data->TotalIdxCount) {
                    m_index_data.clear();
                    for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
                        const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i]; // NOLINT
                        for (std::size_t j = 0; j < cmd_list->IdxBuffer.Size; j++) {
                            m_index_data.push_back(cmd_list->IdxBuffer.Data[j]); // NOLINT
                        }
                    }
                }
            })
            .build("ImGui"));
}

void ImGuiRenderer::initialize_imgui() {
    spdlog::trace("Creating ImGUI context");
    ImGui::CreateContext();

    spdlog::trace("Setting ImGUI styles");
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

    constexpr const char *FONT_FILE_PATH = "assets/fonts/NotoSans-Bold.ttf";
    constexpr float FONT_SIZE = 18.0f;
    spdlog::trace("Loading ImGui front {}", FONT_FILE_PATH);

    ImFont *font = io.Fonts->AddFontFromFileTTF(FONT_FILE_PATH, FONT_SIZE);

    unsigned char *font_texture_data{};
    int font_texture_width{0};
    int font_texture_height{0};
    io.Fonts->GetTexDataAsRGBA32(&font_texture_data, &font_texture_width, &font_texture_height);

    if (font == nullptr || font_texture_data == nullptr) {
        spdlog::error("Unable to load font {}.  Falling back to error texture", FONT_FILE_PATH);
        m_imgui_texture = std::make_unique<wrapper::GpuTexture>(m_device, wrapper::CpuTexture());
    } else {
        spdlog::trace("Creating ImGUI font texture");

        // Our font textures always have 4 channels and a single mip level by definition.
        constexpr int FONT_TEXTURE_CHANNELS{4};
        constexpr int FONT_MIP_LEVELS{1};

        VkDeviceSize upload_size = static_cast<VkDeviceSize>(font_texture_width) *
                                   static_cast<VkDeviceSize>(font_texture_height) *
                                   static_cast<VkDeviceSize>(FONT_TEXTURE_CHANNELS);

        m_imgui_texture = std::make_unique<wrapper::GpuTexture>(
            m_device, font_texture_data, upload_size, font_texture_width, font_texture_height, FONT_TEXTURE_CHANNELS,
            FONT_MIP_LEVELS, "ImGUI font texture");
    }
}

void ImGuiRenderer::setup_stage(render_graph::RenderGraph *render_graph, GraphicsStageBuilder &stage_builder,
                                DescriptorBuilder &descriptor_builder) {

    render_graph->add_graphics_stage(
        stage_builder.uses_shader(m_vertex_shader)
            .uses_shader(m_fragment_shader)
            .reads_from(vertex_buffer)
            .reads_from(index_buffer)
            .bind_buffer(vertex_buffer, 0)
            .writes_to(back_buffer)
            .writes_to(depth_buffer)
            .set_blend_attachment({
                .blendEnable = VK_TRUE,
                .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
                .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
                .colorBlendOp = VK_BLEND_OP_ADD,
                .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
                .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                .alphaBlendOp = VK_BLEND_OP_ADD,
            })

            .set_on_update([&] {
                update_imgui(); // TODO: Can we use a class method as std::function and remove the lambda?
            })
            .add_descriptor_set_layout(
                // TODO: Further abstract descriptors?
                descriptor_builder
                    .add_combined_image_sampler(m_imgui_texture->sampler(), m_imgui_texture->image_view(), 0)
                    .build("ImGui"))
            .build("ImGui"));
}

void ImGuiRenderer::update_imgui() {
    m_imgui_draw_data = ImGui::GetDrawData();
    if (m_imgui_draw_data == nullptr) {
        return;
    }
    if (m_imgui_draw_data->TotalVtxCount != 0) {
        m_update_vertices = true;
    }
    if (m_imgui_draw_data->TotalIdxCount != 0) {
        m_update_indices = true;
    }
}

} // namespace inexor::vulkan_renderer::renderer_components
