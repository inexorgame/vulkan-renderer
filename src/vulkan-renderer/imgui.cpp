#include "inexor/vulkan-renderer/imgui.hpp"

#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/gpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/shader.hpp"

namespace inexor::vulkan_renderer {

ImGUIOverlay::ImGUIOverlay(const wrapper::Device &device, const wrapper::Swapchain &swapchain,
                           std::weak_ptr<Swapchain> swapchain2, RenderGraph *render_graph, TextureResource *back_buffer,
                           std::weak_ptr<GraphicsPass> previous_pass,
                           std::shared_ptr<render_graph::RenderGraph> render_graph2,
                           std::function<void()> on_update_user_imgui_data)
    : m_device(device), m_swapchain(swapchain), m_previous_pass(previous_pass),
      m_on_update_user_imgui_data(std::move(on_update_user_imgui_data)) {
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
    io.FontGlobalScale = m_scale;

    spdlog::trace("Loading ImGUI shaders");
    m_vertex_shader = std::make_shared<wrapper::Shader>(m_device, VK_SHADER_STAGE_VERTEX_BIT, "ImGUI vertex shader",
                                                        "shaders/ui.vert.spv");
    m_fragment_shader = std::make_shared<wrapper::Shader>(m_device, VK_SHADER_STAGE_FRAGMENT_BIT,
                                                          "ImGUI fragment shader", "shaders/ui.frag.spv");

    // Load font texture

    // TODO: Move this data into a container class; have container class also support bold and italic.
    constexpr const char *FONT_FILE_PATH = "assets/fonts/NotoSans-Bold.ttf";
    constexpr float FONT_SIZE = 18.0f;

    spdlog::trace("Loading front {}", FONT_FILE_PATH);

    ImFont *font = io.Fonts->AddFontFromFileTTF(FONT_FILE_PATH, FONT_SIZE);

    io.Fonts->GetTexDataAsRGBA32(&m_font_texture_data, &m_font_texture_width, &m_font_texture_height);

    if (font == nullptr || m_font_texture_data == nullptr) {
        spdlog::error("Unable to load font {}. Falling back to error texture", FONT_FILE_PATH);
        m_imgui_texture = std::make_unique<wrapper::GpuTexture>(m_device, wrapper::CpuTexture());

        // RENDERGRAPH2
        // @TODO: generate error Texture!
    } else {
        spdlog::trace("Creating ImGUI font texture");

        // Our font textures always have 4 channels and a single mip level by definition.
        constexpr int FONT_TEXTURE_CHANNELS{4};
        constexpr int FONT_MIP_LEVELS{1};

        m_upload_size = static_cast<VkDeviceSize>(m_font_texture_width) *
                        static_cast<VkDeviceSize>(m_font_texture_height) *
                        static_cast<VkDeviceSize>(FONT_TEXTURE_CHANNELS);

        m_imgui_texture = std::make_unique<wrapper::GpuTexture>(
            m_device, m_font_texture_data, m_upload_size, m_font_texture_width, m_font_texture_height,
            FONT_TEXTURE_CHANNELS, FONT_MIP_LEVELS, "ImGUI font texture");

        m_imgui_texture2 = render_graph2->add_texture(
            "ImGui|Texture", render_graph::TextureUsage::COLOR_ATTACHMENT, VK_FORMAT_R8G8B8A8_UNORM,
            m_font_texture_width, m_font_texture_height, FONT_TEXTURE_CHANNELS, VK_SAMPLE_COUNT_1_BIT, [&]() {
                // RENDERGRAPH2
                // Make sure the ImGui font texture is only updated once!
                if (!m_imgui_font_texture_initialized2) {
                    m_imgui_texture2.lock()->request_update(m_font_texture_data, m_upload_size);
                    m_imgui_font_texture_initialized2 = true;
                }
            });
    }

    // Create an instance of the resource descriptor builder.
    // This allows us to make resource descriptors with the help of a builder pattern.
    wrapper::descriptors::DescriptorBuilder descriptor_builder(m_device);

    // Make use of the builder to create a resource descriptor for the combined image sampler.
    m_descriptor = std::make_unique<wrapper::descriptors::ResourceDescriptor>(
        descriptor_builder.add_combined_image_sampler(m_imgui_texture->sampler(), m_imgui_texture->image_view(), 0)
            .build("ImGUI"));

    // RENDERGRAPH2
    render_graph2->add_resource_descriptor(
        [&](vulkan_renderer::wrapper::descriptors::DescriptorSetLayoutBuilder &builder) {
            m_descriptor_set_layout2 =
                builder.add(wrapper::descriptors::DescriptorType::COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                    .build("ImGui|Texture");
        },
        [&](vulkan_renderer::wrapper::descriptors::DescriptorSetAllocator &allocator) {
            m_descriptor_set2 = allocator.allocate("ImGui|Texture", m_descriptor_set_layout2);
        },
        [&](vulkan_renderer::wrapper::descriptors::WriteDescriptorSetBuilder &builder) {
            return builder.add(m_descriptor_set2, m_imgui_texture2).build();
        });

    // RENDERGRAPH2
    m_vertex_buffer2 = render_graph2->add_buffer("imgui vertices", render_graph::BufferType::VERTEX_BUFFER, [&]() {
        // @TODO: Update vertex buffer!
    });
    // RENDERGRAPH2
    m_index_buffer2 = render_graph2->add_buffer("imgui indices", render_graph::BufferType::INDEX_BUFFER, [&]() {
        // @TODO: Update vertex buffer!
    });

    // RENDERGRAPH2
    render_graph2->add_graphics_pipeline([&](GraphicsPipelineBuilder &builder) {
        m_imgui_pipeline2 = builder
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
                                .add_default_color_blend_attachment()
                                .add_color_attachment_format(m_swapchain.image_format())
                                .set_dynamic_states({
                                    VK_DYNAMIC_STATE_VIEWPORT,
                                    VK_DYNAMIC_STATE_SCISSOR,
                                })
                                .set_scissor(m_swapchain.extent())
                                .set_viewport(m_swapchain.extent())
                                .add_shader(m_vertex_shader)
                                .add_shader(m_fragment_shader)
                                .set_descriptor_set_layout(m_descriptor_set_layout2)
                                .add_push_constant_range(VK_SHADER_STAGE_VERTEX_BIT, sizeof(m_push_const_block))
                                .build("ImGui");
        // Return the pipeline which was just created
        return m_imgui_pipeline2;
    });

    // RENDERGRAPH2
    m_imgui_pass2 = render_graph2->add_graphics_pass(
        render_graph2->get_graphics_pass_builder()
            .writes_to(swapchain2)
            .conditionally_reads_from(m_previous_pass, !m_previous_pass.expired())
            .set_on_record([&](const wrapper::commands::CommandBuffer &cmd_buf) {
                ImDrawData *draw_data = ImGui::GetDrawData();
                if (draw_data == nullptr || draw_data->TotalIdxCount == 0 || draw_data->TotalVtxCount == 0) {
                    m_on_update_user_imgui_data();
                    return;
                }
                const ImGuiIO &io = ImGui::GetIO();
                m_push_const_block.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);

                cmd_buf.bind_pipeline(m_imgui_pipeline2)
                    .bind_descriptor_set(m_descriptor_set2, m_imgui_pipeline2)
                    .push_constant(m_imgui_pipeline2, m_push_const_block, VK_SHADER_STAGE_VERTEX_BIT)
                    .bind_vertex_buffer(m_vertex_buffer2)
                    .bind_index_buffer(m_index_buffer2)
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
            .build("ImGui", render_graph::DebugLabelColor::BLUE));

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
    m_stage->add_descriptor_layout(m_descriptor->descriptor_set_layout());

    // Setup push constant range for global translation and scale.
    m_stage->add_push_constant_range({
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .offset = 0,
        .size = sizeof(PushConstBlock),
    });

    // Setup blend attachment.
    m_stage->set_blend_attachment({
        .blendEnable = VK_TRUE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    });

    m_stage->set_on_record([&](const PhysicalStage &physical, const wrapper::commands::CommandBuffer &cmd_buf) {
        ImDrawData *imgui_draw_data = ImGui::GetDrawData();
        if (imgui_draw_data == nullptr) {
            return;
        }

        const ImGuiIO &io = ImGui::GetIO();
        m_push_const_block.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
        m_push_const_block.translate = glm::vec2(-1.0f);
        cmd_buf.bind_descriptor_sets(m_descriptor->descriptor_sets(), physical.m_pipeline->pipeline_layout())
            .push_constants(physical.m_pipeline->pipeline_layout(), VK_SHADER_STAGE_VERTEX_BIT, sizeof(PushConstBlock),
                            &m_push_const_block);

        std::uint32_t index_offset = 0;
        std::int32_t vertex_offset = 0;
        for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
            const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i]; // NOLINT
            for (std::int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                const ImDrawCmd &draw_cmd = cmd_list->CmdBuffer[j];
                vkCmdDrawIndexed(cmd_buf.cmd_buffer(), draw_cmd.ElemCount, 1, index_offset, vertex_offset, 0);
                index_offset += draw_cmd.ElemCount;
            }
            vertex_offset += cmd_list->VtxBuffer.Size;
        }
    });
}

ImGUIOverlay::~ImGUIOverlay() {
    ImGui::DestroyContext();
}

void ImGUIOverlay::update() {
    ImDrawData *imgui_draw_data = ImGui::GetDrawData();
    if (!imgui_draw_data || imgui_draw_data->TotalVtxCount == 0)
        return;

    m_vertex_data.clear();
    m_index_data.clear();

    for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
        const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i];
        m_vertex_data.insert(m_vertex_data.end(), cmd_list->VtxBuffer.Data,
                             cmd_list->VtxBuffer.Data + cmd_list->VtxBuffer.Size);
        m_index_data.insert(m_index_data.end(), cmd_list->IdxBuffer.Data,
                            cmd_list->IdxBuffer.Data + cmd_list->IdxBuffer.Size);
    }

    // Upload buffers every frame
    m_vertex_buffer->upload_data(m_vertex_data);
    m_index_buffer->upload_data(m_index_data);
}

} // namespace inexor::vulkan_renderer
