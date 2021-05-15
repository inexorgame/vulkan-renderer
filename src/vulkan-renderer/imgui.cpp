#include "inexor/vulkan-renderer/imgui.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <stdexcept>

namespace inexor::vulkan_renderer {

ImGUIOverlay::ImGUIOverlay(const wrapper::Device &device, const wrapper::Swapchain &swapchain,
                           RenderGraph *render_graph, TextureResource *back_buffer)
    : m_device(device), m_swapchain(swapchain) {
    spdlog::debug("Creating ImGUI context");
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
    io.FontGlobalScale = m_scale;

    spdlog::debug("Loading ImGUI shaders");
    m_vertex_shader = std::make_unique<wrapper::Shader>(m_device, VK_SHADER_STAGE_VERTEX_BIT, "ImGUI vertex shader",
                                                        "shaders/ui.vert.spv");
    m_fragment_shader = std::make_unique<wrapper::Shader>(m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                          "ImGUI fragment shader", "shaders/ui.frag.spv");

    // Load font texture

    // TODO: Move this data into a container class; have container class also support bold and italic.
    constexpr const char *FONT_FILE_PATH = "assets/fonts/NotoSans-Bold.ttf";
    constexpr float FONT_SIZE = 18.0f;

    spdlog::debug("Loading front '{}'", FONT_FILE_PATH);

    ImFont *font = io.Fonts->AddFontFromFileTTF(FONT_FILE_PATH, FONT_SIZE);

    unsigned char *font_texture_data{};
    int font_texture_width{0};
    int font_texture_height{0};
    io.Fonts->GetTexDataAsRGBA32(&font_texture_data, &font_texture_width, &font_texture_height);

    if (font == nullptr || font_texture_data == nullptr) {
        spdlog::error("Unable to load font {}.  Falling back to error texture.", FONT_FILE_PATH);
        m_imgui_texture = std::make_unique<wrapper::GpuTexture>(m_device, wrapper::CpuTexture());
    } else {
        spdlog::debug("Creating ImGUI font texture");

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

    // Create an instance of the resource descriptor builder.
    // This allows us to make resource descriptors with the help of a builder pattern.
    wrapper::DescriptorBuilder descriptor_builder(m_device, m_swapchain.image_count());

    // Make use of the builder to create a resource descriptor for the combined image sampler.
    m_descriptor = std::make_unique<wrapper::ResourceDescriptor>(
        descriptor_builder.add_combined_image_sampler(m_imgui_texture->sampler(), m_imgui_texture->image_view(), 0)
            .build("ImGUI"));

    m_index_buffer = render_graph->add<BufferResource>("imgui index buffer", BufferUsage::INDEX_BUFFER);
    m_vertex_buffer = render_graph->add<BufferResource>("imgui vertex buffer", BufferUsage::VERTEX_BUFFER);
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, pos));
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R32G32_SFLOAT, offsetof(ImDrawVert, uv));
    m_vertex_buffer->add_vertex_attribute(VK_FORMAT_R8G8B8A8_UNORM, offsetof(ImDrawVert, col));
    m_vertex_buffer->set_element_size(sizeof(ImDrawVert));

    m_stage = render_graph->add<GraphicsStage>("imgui stage");
    m_stage->writes_to(back_buffer);
    m_stage->reads_from(m_index_buffer);
    m_stage->reads_from(m_vertex_buffer);
    m_stage->bind_buffer(m_vertex_buffer, 0);
    m_stage->uses_shader(*m_vertex_shader);
    m_stage->uses_shader(*m_fragment_shader);

    // Setup push constant range for global translation and scale.
    VkPushConstantRange push_constant_range{};
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(PushConstBlock);
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    m_stage->add_descriptor_layout(m_descriptor->descriptor_set_layout());
    m_stage->add_push_constant_range(push_constant_range);

    // Setup blend attachment.
    VkPipelineColorBlendAttachmentState blend_attachment;
    blend_attachment.blendEnable = VK_TRUE;
    blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    m_stage->set_blend_attachment(blend_attachment);
}

ImGUIOverlay::~ImGUIOverlay() {
    ImGui::DestroyContext();
}

void ImGUIOverlay::update() {
    ImDrawData *imgui_draw_data = ImGui::GetDrawData();
    if (imgui_draw_data == nullptr) {
        return;
    }

    if (imgui_draw_data->TotalIdxCount == 0 || imgui_draw_data->TotalVtxCount == 0) {
        return;
    }

    bool should_update = false;
    if (m_index_data.size() != imgui_draw_data->TotalIdxCount) {
        m_index_data.clear();
        for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
            const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i]; // NOLINT
            for (std::size_t j = 0; j < cmd_list->IdxBuffer.Size; j++) {
                m_index_data.push_back(cmd_list->IdxBuffer.Data[j]); // NOLINT
            }
        }
        m_index_buffer->upload_data(m_index_data);
        should_update = true;
    }

    if (m_vertex_data.size() != imgui_draw_data->TotalVtxCount) {
        m_vertex_data.clear();
        for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
            const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i]; // NOLINT
            for (std::size_t j = 0; j < cmd_list->VtxBuffer.Size; j++) {
                m_vertex_data.push_back(cmd_list->VtxBuffer.Data[j]); // NOLINT
            }
        }
        m_vertex_buffer->upload_data(m_vertex_data);
        should_update = true;
    }

    if (!should_update) {
        return;
    }

    m_stage->set_on_record([this](const PhysicalStage &physical, const wrapper::CommandBuffer &cmd_buf) {
        ImDrawData *imgui_draw_data = ImGui::GetDrawData();
        if (imgui_draw_data == nullptr) {
            return;
        }

        const ImGuiIO &io = ImGui::GetIO();
        m_push_const_block.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
        m_push_const_block.translate = glm::vec2(-1.0f);
        cmd_buf.bind_descriptor(*m_descriptor, physical.pipeline_layout());
        cmd_buf.push_constants(physical.pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock),
                               &m_push_const_block);

        std::uint32_t index_offset = 0;
        std::int32_t vertex_offset = 0;
        for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
            const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i]; // NOLINT
            for (std::int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                const ImDrawCmd &draw_cmd = cmd_list->CmdBuffer[j];
                vkCmdDrawIndexed(cmd_buf.get(), draw_cmd.ElemCount, 1, index_offset, vertex_offset, 0);
                index_offset += draw_cmd.ElemCount;
            }
            vertex_offset += cmd_list->VtxBuffer.Size;
        }
    });
}

} // namespace inexor::vulkan_renderer
