#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <unordered_map>

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(Device &device, Swapchain &swapchain)
    : m_device(device), m_swapchain(swapchain), m_graphics_pipeline_builder(device),
      m_descriptor_set_layout_cache(device), m_descriptor_set_layout_builder(device, m_descriptor_set_layout_cache) {}

void RenderGraph::add_graphics_pass(std::string pass_name, GraphicsPassCreateFunction on_pass_create) {
    if (pass_name.empty()) {
        throw std::invalid_argument("[RenderGraph::add_graphics_pass] Error: Parameter 'on_pass_create' is an empty "
                                    "std::string! You must give a name to every rendergraph resource!");
    }
    m_graphics_pass_create_functions.emplace_back(std::move(on_pass_create));
}

void RenderGraph::add_graphics_pipeline(std::string pipeline_name, GraphicsPipelineCreateFunction on_pipeline_create) {
    if (pipeline_name.empty()) {
        throw std::invalid_argument("[RenderGraph::add_graphics_pipeline] Error: Parameter 'pipeline_name' is an "
                                    "empty std::string! You must give a name to every rendergraph resource!");
    }
    m_pipeline_create_functions.emplace_back(std::move(on_pipeline_create));
}

std::shared_ptr<Buffer>
RenderGraph::add_buffer(std::string buffer_name, const BufferType buffer_type, std::function<void()> on_update) {
    m_buffers.emplace_back(
        std::make_shared<Buffer>(m_device, std::move(buffer_name), buffer_type, std::move(on_update)));
    return m_buffers.back();
}

std::shared_ptr<Shader>
RenderGraph::add_shader(std::string shader_name, const VkShaderStageFlagBits shader_stage, std::string file_name) {
    m_shaders.emplace_back(
        std::make_shared<Shader>(m_device, std::move(shader_name), shader_stage, std::move(file_name)));
    return m_shaders.back();
}

std::shared_ptr<Texture> RenderGraph::add_texture(std::string texture_name,
                                                  const TextureUsage usage,
                                                  const VkFormat format,
                                                  std::optional<std::function<void()>> on_init,
                                                  std::optional<std::function<void()>> on_update) {
    m_textures.emplace_back(std::make_shared<Texture>(m_device, std::move(texture_name), usage, format,
                                                      std::move(on_init), std::move(on_update)));
    return m_textures.back();
}

void RenderGraph::check_for_cycles() {
    // TODO: Implement me!
}

void RenderGraph::compile() {
    validate_render_graph();
    determine_pass_order();
    create_buffers();
    create_textures();
    create_graphics_passes();
    create_descriptor_sets();
    create_graphics_pipelines();
}

void RenderGraph::create_buffers() {
    m_device.execute("RenderGraph::create_buffers", [&](const CommandBuffer &cmd_buf) {
        for (const auto &buffer : m_buffers) {
            buffer->m_on_update();
            buffer->create_buffer(cmd_buf);
        }
    });
}

void RenderGraph::create_descriptor_sets() {
    // TODO: Implement me!
}

void RenderGraph::create_graphics_passes() {
    m_graphics_passes.clear();
    m_graphics_passes.reserve(m_graphics_pass_create_functions.size());
    for (const auto &create_func : m_graphics_pass_create_functions) {
        m_graphics_passes.emplace_back(create_func(m_graphics_pass_builder));
    }
}

void RenderGraph::create_graphics_pipelines() {
    m_graphics_pipelines.clear();
    m_graphics_pipelines.reserve(m_pipeline_create_functions.size());
    for (const auto &create_func : m_pipeline_create_functions) {
        m_graphics_pipelines.emplace_back(create_func(m_graphics_pipeline_builder, m_descriptor_set_layout_builder));
    }
}

void RenderGraph::create_textures() {
    m_device.execute("RenderGraph::create_textures", [&](const CommandBuffer &cmd_buf) {
        for (const auto &texture : m_textures) {
            if (texture->m_on_init) {
                // TODO: if(texture->update_requested)...
                // Call the initialization function of the texture (if specified)
                std::invoke(texture->m_on_init.value());
            }
            // TODO: Implement me!
            // texture->create_texture();
        }
    });
}

void RenderGraph::determine_pass_order() {
    // TODO: Implement me!
    // TODO: The data structure to determine pass order should be created before rendergraph compilation!
}

void RenderGraph::record_command_buffer_for_pass(const CommandBuffer &cmd_buf,
                                                 const GraphicsPass &pass,
                                                 const bool is_first_pass,
                                                 const bool is_last_pass,
                                                 const std::uint32_t img_index) {

    // TODO: Remove img_index and implement swapchain.get_current_image()
    // TODO: Or do we need the image index for buffers? (We want to automatically double or triple buffer them)

    // Start a new debug label for this graphics pass (debug labels are visible in graphics debuggers like RenderDoc)
    // TODO: Generate color gradient depending on the number of passes? (Interpolate e.g. in 12 steps for 12 passes)
    cmd_buf.begin_debug_label_region(pass.m_name, {1.0f, 0.0f, 0.0f, 1.0f});

    // If this is the first graphics pass, we need to transform the swapchain image, which comes back in undefined
    // layout after presenting
    if (is_first_pass) {
        cmd_buf.change_image_layout(m_swapchain.image(img_index), VK_IMAGE_LAYOUT_UNDEFINED,
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    // TODO: We need a really clever way to make MSAA easy here

    // TODO: FIX ME!
    VkImageView resolve_color{VK_NULL_HANDLE};

    const auto color_attachment = wrapper::make_info<VkRenderingAttachmentInfo>({
        .imageView = resolve_color,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,
        // TODO: Remove img_index and implement swapchain.get_current_image()
        .resolveImageView = m_swapchain.image_view(img_index),
        .resolveImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = pass.m_clear_values ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = pass.m_clear_values.value().color,
    });

    // TODO: FIX ME!
    VkImageView resolve_depth{VK_NULL_HANDLE};
    VkImageView depth_buffer{depth_buffer};

#if 0
    const auto depth_attachment = wrapper::make_info<VkRenderingAttachmentInfo>({
        .imageView = resolve_depth,
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_MIN_BIT,
        .resolveImageView = depth_buffer,
        .resolveImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        .loadOp = pass.m_clear_values ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = pass.m_clear_values.value().depthStencil,
    });

    const auto rendering_info = wrapper::make_info<VkRenderingInfo>({
        .renderingArea =
            {
                m_swapchain.extent(),
            },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attachment,
        // TODO: depth and stencil attachment
    });

    // Start dynamic rendering (we are no longer using renderpasses)
    cmd_buf.begin_rendering(&rendering_info);

    // Bind the vertex buffers of the pass
    // Note that vertex buffers are optional, meaning the user can either give vertex buffers
    if (!pass.m_vertex_buffers.empty()) {
        cmd_buf.bind_vertex_buffers(pass.m_vertex_buffers);
    }

    // Bind an index buffer if any is present
    if (pass.has_index_buffer()) {
        // Note that in Vulkan you can have a variable number of vertex buffers, but only one index buffer bound
        cmd_buf.bind_index_buffer(pass.m_index_buffer);
    }

#endif

    // Call the custom command buffer recording function of the graphics pass
    std::invoke(pass.m_on_record, cmd_buf);

    // End dynamic rendering
    cmd_buf.end_rendering();

    // If this is the last graphics pass, change the image layout of the back buffer for presenting
    if (is_last_pass) {
        cmd_buf.change_image_layout(m_swapchain.image(img_index), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    // End the debug label for this graphics pass (debug labels are visible in graphics debuggers like RenderDoc)
    cmd_buf.end_debug_label_region();
}

void RenderGraph::record_command_buffers(const CommandBuffer &cmd_buf, const std::uint32_t img_index) {
    // TODO: Find the balance between recording all passes into one big command buffer vs. recording one command buffer
    // per pass. Recording one command buffer per pass would allow us to do this in parallel using taskflow. Assuming
    // the programmer takes responsibility for synchronization of all on_record functions he provides, there should be a
    // performance benefit from recording command buffers in parallel. On the other hand, each command buffer introduces
    // new overhead (maybe even when all command buffers are submitted in batch). Another solution would be to let the
    // user request a command buffer manually and to let him specify which command buffer to use in which pass. Combined
    // with the user-defined order of passes, this will give more flexibility.

    // Loop through all graphics passes and record their command buffer
    for (std::size_t pass_index = 0; pass_index < m_graphics_passes.size(); pass_index++) {
        // This is important to know because of image layout transitions for back buffer for example
        const bool is_first_pass = (pass_index == 0);
        const bool is_last_pass = (pass_index == (m_graphics_passes.size() - 1));
        record_command_buffer_for_pass(cmd_buf, *m_graphics_passes[pass_index], is_first_pass, is_last_pass, img_index);
    }
}

void RenderGraph::render() {
    const auto &cmd_buf = m_device.request_command_buffer("RenderGraph::render()");
    record_command_buffers(cmd_buf, m_swapchain.acquire_next_image_index());

    // TODO: Further abstract this away?
    const std::array<VkPipelineStageFlags, 1> stage_mask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    cmd_buf.submit_and_wait(wrapper::make_info<VkSubmitInfo>({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = m_swapchain.image_available_semaphore(),
        .pWaitDstStageMask = stage_mask.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = cmd_buf.ptr(),
    }));

    m_swapchain.present();
}

void RenderGraph::reset() {
    // TODO: Implement me!
}

void RenderGraph::update_buffers() {
    m_device.execute("RenderGraph::update_buffers", [&](const CommandBuffer &cmd_buf) {
        for (const auto &buffer : m_buffers) {
            if (buffer->m_update_requested) {
                buffer->destroy_buffer();
                buffer->m_on_update();
                buffer->create_buffer(cmd_buf);
            }
        }
    });
}

void RenderGraph::update_textures() {
    for (const auto &texture : m_textures) {
        // If m_on_update is not std::nullopt, call the update function of the texture
        if (texture->m_on_update) {
            if (texture->m_update_requested) {
                std::invoke(texture->m_on_update.value());
            }
        }
        // TODO: Update texture (Implement me!)
    }
    // TODO: Batch barriers for updates which require staging buffer
}

void RenderGraph::update_descriptor_sets() {
    // The problem is here that the binding is determined by the oder we call the add methods of the descriptor set
    // layout builder, but that is determined by the order we iterate through buffer and texture reads. Those reads
    // would either have to be in one struct or some other ordering must take place!!! If not, this will cause trouble
    // if a pass reads from both let's say a uniform buffer and a texture, but the order specified in descriptor set
    // layout builder results in a binding order that is incorrect!

    // TODO: Implement me!
    // TODO: Builder pattern for descriptor writes?
}

void RenderGraph::validate_render_graph() {
    if (m_graphics_pass_create_functions.empty()) {
        throw std::runtime_error("[RenderGraph::validate_render_graph] Error: No graphics passes in rendergraph!");
    }
    if (m_pipeline_create_functions.empty()) {
        throw std::runtime_error("[RenderGraph::validate_render_graph] Error: No graphics pipelines in rendergraph!");
    }
    check_for_cycles();
}

} // namespace inexor::vulkan_renderer::render_graph
