#include "inexor/vulkan-renderer/imgui.hpp"

#include "inexor/vulkan-renderer/wrapper/cpu_texture.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <cassert>
#include <imgui.h>
#include <stdexcept>

namespace inexor::vulkan_renderer {

ImGUIOverlay::ImGUIOverlay(ImGUIOverlay &&other) noexcept
    : m_device(other.m_device), m_swapchain(other.m_swapchain), m_scale(other.m_scale),
      m_imgui_mesh(std::exchange(other.m_imgui_mesh, nullptr)),
      m_imgui_texture(std::exchange(other.m_imgui_texture, nullptr)),
      m_renderpass(std::exchange(other.m_renderpass, nullptr)),
      m_vert_shader(std::exchange(other.m_vert_shader, nullptr)),
      m_frag_shader(std::exchange(other.m_frag_shader, nullptr)),
      m_command_pool(std::exchange(other.m_command_pool, nullptr)),
      m_descriptor(std::exchange(other.m_descriptor, nullptr)), m_pipeline(std::exchange(other.m_pipeline, nullptr)),
      m_subpass(other.m_subpass), m_vertex_count(other.m_vertex_count), m_index_count(other.m_index_count),
      m_shaders(other.m_shaders), m_command_buffers(std::move(other.m_command_buffers)),
      m_framebuffers(std::move(other.m_framebuffers)), m_push_const_block(other.m_push_const_block) {}

ImGUIOverlay::ImGUIOverlay(const wrapper::Device &device, const wrapper::Swapchain &swapchain)
    : m_device(device), m_swapchain(swapchain) {
    assert(device.device());
    assert(device.physical_device());
    assert(device.allocator());
    assert(device.graphics_queue());

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

    spdlog::debug("Loading ImGUI vertex shader");

    m_vert_shader = std::make_unique<wrapper::Shader>(m_device, VK_SHADER_STAGE_VERTEX_BIT, "ImGUI vertex shader",
                                                      "shaders/ui.vert.spv");

    VkPipelineShaderStageCreateInfo shader_info{};
    shader_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
    shader_info.module = m_vert_shader->module();
    shader_info.pName = m_vert_shader->entry_point().c_str();

    m_shaders.push_back(shader_info);

    spdlog::debug("Loading ImGUI fragment shader");

    m_frag_shader = std::make_unique<wrapper::Shader>(m_device, VK_SHADER_STAGE_FRAGMENT_BIT, "ImGUI fragment shader",
                                                      "shaders/ui.frag.spv");

    shader_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shader_info.module = m_frag_shader->module();
    shader_info.pName = m_frag_shader->entry_point().c_str();

    m_shaders.push_back(shader_info);

    // Initialize push constant block
    m_push_const_block.scale = glm::vec2(0.0f, 0.0f);
    m_push_const_block.translate = glm::vec2(0.0f, 0.0f);

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

    m_command_pool = std::make_unique<wrapper::CommandPool>(m_device, m_device.graphics_queue_family_index());

    VkDescriptorImageInfo font_desc{};
    font_desc.sampler = m_imgui_texture->sampler();
    font_desc.imageView = m_imgui_texture->image_view();
    font_desc.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    std::vector<VkDescriptorSetLayoutBinding> layout_bindings(1);
    layout_bindings[0].binding = 0;
    layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_bindings[0].descriptorCount = 1;
    layout_bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::vector<VkWriteDescriptorSet> desc_writes(1);
    desc_writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    desc_writes[0].dstSet = nullptr;
    desc_writes[0].dstBinding = 0;
    desc_writes[0].dstArrayElement = 0;
    desc_writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    desc_writes[0].descriptorCount = 1;
    desc_writes[0].pImageInfo = &font_desc;

    std::initializer_list<VkDescriptorType> pool_types = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER};

    m_descriptor = std::make_unique<wrapper::ResourceDescriptor>(m_device, swapchain.image_count(), pool_types,
                                                                 layout_bindings, desc_writes, "ImGUI");

    // TODO() Use pipeline cache!
    std::vector<VkAttachmentDescription> attachments(1);
    attachments[0].format = m_swapchain.image_format();
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_ref = {};
    color_ref.attachment = 0;
    color_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_desc = {};
    subpass_desc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_desc.colorAttachmentCount = 1;
    subpass_desc.pColorAttachments = &color_ref;

    std::vector<VkSubpassDependency> subpass_deps(1);
    subpass_deps[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    subpass_deps[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    subpass_deps[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpass_deps[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    spdlog::debug("Creating ImGUI renderpass");

    m_renderpass = std::make_unique<wrapper::RenderPass>(m_device, attachments, subpass_deps, subpass_desc, "ImGUI");

    VkPushConstantRange push_constant_range{};
    push_constant_range.offset = 0;
    push_constant_range.size = sizeof(PushConstBlock);
    push_constant_range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    auto *descriptor_set_layout = m_descriptor->descriptor_set_layout();

    VkPipelineLayoutCreateInfo pipeline_layout_ci{};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &descriptor_set_layout;
    pipeline_layout_ci.pushConstantRangeCount = 1;
    pipeline_layout_ci.pPushConstantRanges = &push_constant_range;

    if (vkCreatePipelineLayout(m_device.device(), &pipeline_layout_ci, nullptr, &m_pipeline_layout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout for ImGUI!");
    }

    VkVertexInputBindingDescription vertex_input_bind_desc{};
    vertex_input_bind_desc.binding = 0;
    vertex_input_bind_desc.stride = sizeof(ImDrawVert);
    vertex_input_bind_desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    std::vector<VkVertexInputBindingDescription> vertex_input_bindings = {vertex_input_bind_desc};

    std::vector<VkVertexInputAttributeDescription> vertex_input_attrs(3);

    // Location 0: Position
    vertex_input_attrs[0].binding = 0;
    vertex_input_attrs[0].location = 0;
    vertex_input_attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_input_attrs[0].offset = offsetof(ImDrawVert, pos);

    // Location 1: UV
    vertex_input_attrs[1].binding = 0;
    vertex_input_attrs[1].location = 1;
    vertex_input_attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
    vertex_input_attrs[1].offset = offsetof(ImDrawVert, uv);

    // Location 2: Color
    vertex_input_attrs[2].binding = 0;
    vertex_input_attrs[2].location = 2;
    vertex_input_attrs[2].format = VK_FORMAT_R8G8B8A8_UNORM;
    vertex_input_attrs[2].offset = offsetof(ImDrawVert, col);

    spdlog::debug("Creating ImGUI graphics pipeline");

    m_pipeline = std::make_unique<wrapper::GraphicsPipeline>(
        m_device, m_pipeline_layout, m_renderpass->get(), m_shaders, vertex_input_bindings, vertex_input_attrs,
        m_swapchain.extent().width, m_swapchain.extent().height, "ImGUI");

    m_ui_rendering_finished = std::make_unique<wrapper::Fence>(m_device, "ImGUI rendering done", true);
}

ImGUIOverlay::~ImGUIOverlay() {
    spdlog::trace("Destroying ImGUI overlay");
    ImGui::DestroyContext();
    vkDestroyPipelineLayout(m_device.device(), m_pipeline_layout, nullptr);
}

void ImGUIOverlay::update() {
    ImDrawData *imgui_draw_data = ImGui::GetDrawData();
    bool update_command_buffers = false;

    if (imgui_draw_data == nullptr) {
        return;
    }

    const VkDeviceSize vertex_buffer_size = imgui_draw_data->TotalVtxCount * sizeof(ImDrawVert);
    const VkDeviceSize index_buffer_size = imgui_draw_data->TotalIdxCount * sizeof(ImDrawIdx);

    if ((vertex_buffer_size == 0) || (index_buffer_size == 0)) {
        return;
    }

    // TODO() Do not allocate memory at runtime!
    if (!m_imgui_mesh) {
        m_imgui_mesh = std::make_unique<wrapper::MeshBuffer>(m_device, "imgui_mesh_buffer", sizeof(ImDrawVert),
                                                             imgui_draw_data->TotalVtxCount, sizeof(ImDrawIdx),
                                                             imgui_draw_data->TotalIdxCount);
    }

    if ((m_imgui_mesh->get_vertex_buffer() == VK_NULL_HANDLE) || (m_vertex_count != imgui_draw_data->TotalVtxCount)) {
        m_imgui_mesh.reset();

        spdlog::debug("Creating ImGUI vertex buffer");

        m_imgui_mesh = std::make_unique<wrapper::MeshBuffer>(m_device, "imgui_mesh_buffer", sizeof(ImDrawVert),
                                                             imgui_draw_data->TotalVtxCount, sizeof(ImDrawIdx),
                                                             imgui_draw_data->TotalIdxCount);

        m_vertex_count = imgui_draw_data->TotalVtxCount;

        update_command_buffers = true;
    }

    if ((m_imgui_mesh->get_index_buffer() == VK_NULL_HANDLE) ||
        (m_index_count < static_cast<std::uint32_t>(imgui_draw_data->TotalIdxCount))) {
        m_imgui_mesh.reset();

        spdlog::debug("Creating ImGUI index buffer");

        m_imgui_mesh = std::make_unique<wrapper::MeshBuffer>(m_device, "imgui_mesh_buffer", sizeof(ImDrawVert),
                                                             imgui_draw_data->TotalVtxCount, sizeof(ImDrawIdx),
                                                             imgui_draw_data->TotalIdxCount);

        m_index_count = imgui_draw_data->TotalIdxCount;

        update_command_buffers = true;
    }

    if (update_command_buffers) {
        // TODO: Implement an update method for MeshBuffer !
        auto *vertex_buffer_address = static_cast<ImDrawVert *>(m_imgui_mesh->get_vertex_buffer_address());
        auto *index_buffer_address = static_cast<ImDrawIdx *>(m_imgui_mesh->get_index_buffer_address());

        for (std::size_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
            const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i];
            std::memcpy(vertex_buffer_address, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            std::memcpy(index_buffer_address, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vertex_buffer_address += cmd_list->VtxBuffer.Size;
            index_buffer_address += cmd_list->IdxBuffer.Size;
        }

        const ImGuiIO &io = ImGui::GetIO();

        spdlog::debug("Creating frame buffers for ImGUI");

        // TODO: Do we even need to recreate framebuffers?
        m_framebuffers.clear();
        m_framebuffers.resize(m_swapchain.image_count());

        m_command_buffers.clear();
        m_command_buffers.resize(m_swapchain.image_count());

        spdlog::debug("Creating ImGUI renderpass");

        // Record command buffer for every image in swapchain
        for (std::size_t k = 0; k < m_swapchain.image_count(); k++) {
            std::vector<VkImageView> image_views;

            image_views.push_back(m_swapchain.image_view(k));

            m_framebuffers[k] = std::make_unique<wrapper::Framebuffer>(m_device, m_renderpass->get(), image_views,
                                                                       m_swapchain, "ImGUI Framebuffer");

            m_command_buffers[k] = std::make_unique<wrapper::CommandBuffer>(m_device, m_command_pool->get(), "ImGUI");

            m_command_buffers[k]->begin();

            std::array<VkClearValue, 1> clear_values{};
            clear_values[0].color = {0.0f, 0.0f, 0.0f};

            auto render_pass_bi = wrapper::make_info<VkRenderPassBeginInfo>();
            render_pass_bi.framebuffer = m_framebuffers[k]->get();
            render_pass_bi.renderArea.extent = m_swapchain.extent();
            render_pass_bi.renderPass = m_renderpass->get();
            render_pass_bi.clearValueCount = 1;
            render_pass_bi.pClearValues = clear_values.data();

            m_command_buffers[k]->begin_render_pass(render_pass_bi);
            m_command_buffers[k]->bind_graphics_pipeline(m_pipeline->get());

            auto descriptor_sets = m_descriptor->descriptor_sets();

            vkCmdBindDescriptorSets(m_command_buffers[k]->get(), VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline_layout, 0,
                                    1, &descriptor_sets[0], 0, nullptr);

            m_push_const_block.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
            m_push_const_block.translate = glm::vec2(-1.0f);

            vkCmdPushConstants(m_command_buffers[k]->get(), m_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0,
                               sizeof(PushConstBlock), &m_push_const_block);

            std::array<VkDeviceSize, 1> offsets{0};

            // TODO() Refactor this!
            auto *vertex_buffer = m_imgui_mesh->get_vertex_buffer();

            vkCmdBindVertexBuffers(m_command_buffers[k]->get(), 0, 1, &vertex_buffer, offsets.data());

            vkCmdBindIndexBuffer(m_command_buffers[k]->get(), m_imgui_mesh->get_index_buffer(), 0,
                                 VK_INDEX_TYPE_UINT16);

            std::int32_t vertex_offset{0};
            std::int32_t index_offset{0};

            for (int32_t i = 0; i < imgui_draw_data->CmdListsCount; i++) {
                const ImDrawList *cmd_list = imgui_draw_data->CmdLists[i];
                for (int32_t j = 0; j < cmd_list->CmdBuffer.Size; j++) {
                    const ImDrawCmd *imgui_draw_command = &cmd_list->CmdBuffer[j];
                    vkCmdDrawIndexed(m_command_buffers[k]->get(), imgui_draw_command->ElemCount, 1, index_offset,
                                     vertex_offset, 0);

                    index_offset += imgui_draw_command->ElemCount;
                }
                vertex_offset += cmd_list->VtxBuffer.Size;
            }

            m_command_buffers[k]->end_render_pass();
            m_command_buffers[k]->end();
        }

        // We must reset the VkFence before we can vkQueueSubmit in ImGUIOverlay::render.
        m_ui_rendering_finished->reset();
    }
}

void ImGUIOverlay::render(const std::uint32_t image_index) {
    if (m_command_buffers.empty()) {
        return;
    }

    auto *cmd_buf = m_command_buffers[image_index]->get();
    auto submit_info = wrapper::make_info<VkSubmitInfo>();
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &cmd_buf;

    std::array<VkPipelineStageFlags, 1> wait_stage_mask = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.pWaitDstStageMask = wait_stage_mask.data();

    vkQueueSubmit(m_device.graphics_queue(), 1, &submit_info, m_ui_rendering_finished->get());

    m_ui_rendering_finished->block();
    m_ui_rendering_finished->reset();
}

} // namespace inexor::vulkan_renderer
