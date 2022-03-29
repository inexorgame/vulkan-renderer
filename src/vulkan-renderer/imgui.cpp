#include "inexor/vulkan-renderer/imgui.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/texture/gpu_texture.hpp"
#include "inexor/vulkan-renderer/vk_tools/fill_vk_struct.hpp"
#include "inexor/vulkan-renderer/vk_tools/vert_attr_layout.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/shader_loader.hpp"

#include <cassert>
#include <stdexcept>
#include <utility>

namespace inexor::vulkan_renderer {

void ImGUIOverlay::setup_rendering_resources(RenderGraph *render_graph, TextureResource *back_buffer) {

    auto builder = wrapper::DescriptorBuilder(render_graph->device_wrapper());

    m_descriptor = builder.add_combined_image_sampler(*m_imgui_texture).build("imgui");

    // TODO: Unify into one call of the builder pattern
    m_stage = render_graph->add<GraphicsStage>("ImGui");

    m_stage->bind_buffer(vertex_buffer())
        ->bind_buffer(index_buffer())
        ->uses_shaders(m_shader_loader.shaders())
        ->writes_to(back_buffer)
        ->reads_from(vertex_buffer())
        ->reads_from(index_buffer())
        ->add_push_constant_range<PushConstBlock>(VK_SHADER_STAGE_VERTEX_BIT)
        ->add_descriptor_set_layout(m_descriptor->descriptor_set_layout);

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

ImGUIOverlay::ImGUIOverlay(RenderGraph *render_graph, const wrapper::Swapchain &swapchain, TextureResource *back_buffer)
    : m_device(render_graph->device_wrapper()), m_swapchain(swapchain),
      m_shader_loader(m_device, m_shader_files, "imgui"),
      GpuDataBase(render_graph->device_wrapper(), m_vertex_attribute_layout, "imgui") {

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

    // TODO: Move this data into a container class; have container class also support bold and italic.
    constexpr const char *FONT_FILE_PATH = "assets/fonts/NotoSans-Bold.ttf";
    constexpr float FONT_SIZE = 18.0f;

    spdlog::debug("Loading front '{}'", FONT_FILE_PATH);

    // Load font texture
    ImFont *font = io.Fonts->AddFontFromFileTTF(FONT_FILE_PATH, FONT_SIZE);

    unsigned char *font_texture_data{};
    int font_texture_width{0};
    int font_texture_height{0};
    const VkFormat font_texture_format = VK_FORMAT_R8G8B8A8_UNORM;

    io.Fonts->GetTexDataAsRGBA32(&font_texture_data, &font_texture_width, &font_texture_height);

    const auto image_ci = vk_tools::fill_image_ci(font_texture_format, font_texture_width, font_texture_height, 1,
                                                  VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

    const auto image_view_ci = vk_tools::fill_image_view_ci(font_texture_format);
    const auto sampler_ci = vk_tools::fill_sampler_ci();

    if (font == nullptr || font_texture_data == nullptr) {
        spdlog::error("Unable to load font {}. Using error texture as fallback.", FONT_FILE_PATH);

        m_imgui_texture =
            std::make_unique<texture::GpuTexture>(m_device, texture::CpuTexture(), image_ci, image_view_ci, sampler_ci);
    } else {
        spdlog::debug("Creating ImGUI font texture");

        // Our font textures always have 4 channels and a single mip level by definition.
        constexpr int FONT_TEXTURE_CHANNELS{4};
        constexpr int FONT_MIP_LEVELS{1};

        const VkDeviceSize upload_size = static_cast<VkDeviceSize>(font_texture_width) *
                                         static_cast<VkDeviceSize>(font_texture_height) *
                                         static_cast<VkDeviceSize>(FONT_TEXTURE_CHANNELS);

        m_imgui_texture = std::make_unique<texture::GpuTexture>(m_device, font_texture_data, upload_size, image_ci,
                                                                image_view_ci, sampler_ci, "ImGUI font texture");
    }

    setup_rendering_resources(render_graph, back_buffer);
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

    if (vertex_count() != imgui_draw_data->TotalVtxCount) {
        auto collect_new_vertices = [&]() {
            std::vector<ImDrawVert> vertices;
            vertices.reserve(imgui_draw_data->TotalVtxCount);

            for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
                const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i]; // NOLINT
                for (std::size_t j = 0; j < cmd_list->VtxBuffer.Size; j++) {
                    vertices.push_back(cmd_list->VtxBuffer.Data[j]); // NOLINT
                }
            }

            return std::move(vertices);
        };

        update_vertices(collect_new_vertices());
        should_update = true;
    }

    if (index_count() != imgui_draw_data->TotalIdxCount) {
        auto collect_new_indices = [&]() {
            std::vector<std::uint32_t> indices;
            indices.reserve(imgui_draw_data->TotalIdxCount);

            for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
                const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i]; // NOLINT
                for (std::size_t j = 0; j < cmd_list->IdxBuffer.Size; j++) {
                    indices.push_back(cmd_list->IdxBuffer.Data[j]); // NOLINT
                }
            }

            return std::move(indices);
        };

        update_indices(collect_new_indices());
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

        cmd_buf.bind_descriptor_set(m_descriptor->descriptor_set, physical.pipeline_layout())
            .push_constants(&m_push_const_block, physical.pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT);

        std::uint32_t index_offset = 0;
        std::int32_t vertex_offset = 0;
        for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
            const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i]; // NOLINT
            for (std::int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                const ImDrawCmd &draw_cmd = cmd_list->CmdBuffer[j];
                cmd_buf.draw_indexed(draw_cmd.ElemCount, index_offset, vertex_offset);
                index_offset += draw_cmd.ElemCount;
            }
            vertex_offset += cmd_list->VtxBuffer.Size;
        }
    });
}

} // namespace inexor::vulkan_renderer
