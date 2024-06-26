#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/push_constant_range.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchain.hpp"

#include <unordered_map>

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(Device &device, Swapchain &swapchain)
    : m_device(device), m_swapchain(swapchain), m_graphics_pipeline_builder(device) {}

std::weak_ptr<Buffer> RenderGraph::add_index_buffer(std::string name, std::optional<std::function<void()>> on_update) {
    m_buffers.emplace_back(
        std::make_shared<Buffer>(m_device, std::move(name), VK_INDEX_TYPE_UINT32, std::move(on_update)));
    return m_buffers.back();
}

std::weak_ptr<Buffer> RenderGraph::add_uniform_buffer(std::string name,
                                                      std::optional<std::function<void()>> on_update) {
    m_buffers.emplace_back(std::make_shared<Buffer>(m_device, std::move(name), std::move(on_update)));
    return m_buffers.back();
}

std::weak_ptr<Buffer>
RenderGraph::add_vertex_buffer(std::string name, std::vector<VkVertexInputAttributeDescription> vert_input_attr_descs,
                               std::optional<std::function<void()>> on_update) {
    m_buffers.emplace_back(std::make_shared<Buffer>(m_device, std::move(name), std::move(vert_input_attr_descs)));
    return m_buffers.back();
}

std::weak_ptr<Texture> RenderGraph::add_texture(std::string name, const TextureUsage usage, const VkFormat format,
                                                std::optional<std::function<void()>> on_update) {
    m_textures.emplace_back(std::make_shared<Texture>(std::move(name), usage, format, std::move(on_update)));
    return m_textures.back();
}

void RenderGraph::add_graphics_pass(GraphicsPassCreateCallable on_pass_create) {
    m_on_graphics_pass_create_callables.emplace_back(std::move(on_pass_create));
}

void RenderGraph::add_graphics_pipeline(GraphicsPipelineCreateCallable on_pipeline_create) {
    m_on_graphics_pipeline_create_callables.emplace_back(std::move(on_pipeline_create));
}

void RenderGraph::check_for_cycles() {
    m_log->warn("Implement RenderGraph::check_for_cycles()!");
}

void RenderGraph::compile() {
    m_log->trace("Compiling rendergraph");
    validate_render_graph();
    determine_pass_order();
    create_graphics_passes();
    create_buffers();
    create_descriptor_sets();
    create_graphics_pipeline_layouts();
    create_graphics_pipelines();
    // TODO: What to add more here?
}

void RenderGraph::create_buffers() {
    for (const auto &buffer : m_buffers) {
        // Call the update lambda of the buffer
        if (buffer->m_on_update) {
            std::invoke(buffer->m_on_update.value());
        }
        // Create the buffer
        buffer->create_buffer();
    }
    // TODO: Batch all updates which require staging buffer into one pipeline barrier call
}

void RenderGraph::create_descriptor_sets() {
    m_log->trace("Creating descriptor sets");
}

void RenderGraph::create_graphics_passes() {
    m_log->trace("Creating graphics passes");

    if (m_on_graphics_pass_create_callables.empty()) {
        throw std::runtime_error("Error: No graphics passes specified in rendergraph!");
    }
    m_graphics_passes.clear();
    m_graphics_passes.reserve(m_on_graphics_pass_create_callables.size());

    // Fill the vector of graphics passes by calling the corresponding callables (lambdas)
    for (const auto &on_pass_create_callable : m_on_graphics_pass_create_callables) {
        // Store the result of the graphics pass creation lambda
        m_graphics_passes.push_back(std::move(std::invoke(on_pass_create_callable, m_graphics_pass_builder)));
    }
}

void RenderGraph::create_graphics_pipeline_layouts() {
    m_log->trace("Creating graphics pipeline layouts");

    // TODO: This is wrong! First pipeline layout, then pipeline, right?
    // TODO: We can't iterate through m_graphics_pipelines if we haven't created them yet!

    m_graphics_pipeline_layouts.clear();
    m_graphics_pipeline_layouts.reserve(m_graphics_pipelines.size());

    for (const auto &pipeline : m_graphics_pipelines) {
// TODO: How to associate pipelines with passes?
#if 0
        m_graphics_pipeline_layouts.emplace_back(std::make_unique<PipelineLayout>(m_device,                           //
                                                                                  pipeline->descriptor_set_layouts(), //
                                                                                  pipeline->push_constant_ranges(),   //
                                                                                  pipeline->name()));
#endif
    }
}

void RenderGraph::create_graphics_pipelines() {
    m_log->trace("Creating graphics pipelines");
    if (m_on_graphics_pipeline_create_callables.empty()) {
        throw std::runtime_error("Error: No graphics pipelines specified in rendergraph!");
    }
    m_graphics_pipelines.clear();
    const auto pipeline_count = m_on_graphics_pipeline_create_callables.size();
    m_graphics_pipelines.reserve(pipeline_count);

    // Populate the vector of graphics pipelines by calling the corresponding callables (lambdas)
    // for (const auto &on_pipeline_create_callable : m_on_graphics_pipeline_create_callables) {
    for (std::size_t pipeline_index = 0; pipeline_index < pipeline_count; pipeline_index++) {
        m_graphics_pipelines.push_back(
            std::move(std::invoke(m_on_graphics_pipeline_create_callables.at(pipeline_index), //
                                  m_graphics_pipeline_builder,                                //
                                  m_graphics_pipeline_layouts.at(pipeline_index)->pipeline_layout())));
    }
}

void RenderGraph::determine_pass_order() {
    // TODO: Implement dfs
}

void RenderGraph::record_command_buffer_for_pass(const CommandBuffer &cmd_buf, const GraphicsPass &pass,
                                                 const bool is_first_pass, const bool is_last_pass,
                                                 const std::uint32_t img_index) {
    // Start a new debug label for this graphics pass (debug labels are visible in graphics debuggers like RenderDoc)
    // TODO: Generate color gradient depending on the number of passes? (Interpolate e.g. in 12 steps for 12 passes)
    cmd_buf.begin_debug_label_region(pass.m_name, {1.0f, 0.0f, 0.0f, 1.0f});

    // If this is the first graphics pass, we need to transform the swapchain image, which comes back in undefined
    // layout after presenting
    if (is_first_pass) {
        cmd_buf.change_image_layout(m_swapchain.image(img_index), //
                                    VK_IMAGE_LAYOUT_UNDEFINED,    //
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }

    // TODO: We need a really clever way to make MSAA easy here

    // TODO: FIX ME!
    VkImageView resolve_color{VK_NULL_HANDLE};

    const auto color_attachment = wrapper::make_info<VkRenderingAttachmentInfo>({
        .imageView = resolve_color,
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .resolveMode = VK_RESOLVE_MODE_AVERAGE_BIT,
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

    // TODO: bind descriptor set

    // TODO: push constants
    if (!pass.m_push_constant_ranges.empty()) {
        cmd_buf.push_constants();
    }
#endif

    // Call the user-defined on_record function of the graphics pass
    // This is where the real rendering of the pass is happening
    // NOTE: It is the responsibility of the programmer to bind the correct pipeline in the on_record lambda! This is
    // not part of the rendergraph abstraction because during rendering binding pipelines can be arbitrarily complex.
    // TODO: Implement binding only one (or several) pipelines at beginning of rendering, so there is partial
    // abstraction
    std::invoke(pass.m_on_record, cmd_buf);

    // End dynamic rendering
    cmd_buf.end_rendering();

    // If this is the last graphics pass, transition the image layout of the back buffer for presenting
    if (is_last_pass) {
        cmd_buf.change_image_layout(m_swapchain.image(img_index),             //
                                    VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, //
                                    VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    }

    // End the debug label for this graphics pass (debug labels are visible in graphics debuggers like RenderDoc)
    cmd_buf.end_debug_label_region();
}

void RenderGraph::record_command_buffers(const CommandBuffer &cmd_buf, const std::uint32_t img_index) {
    // TODO: Support multiple passes per command buffer, not just recording everything into one command buffer
    // TODO: Record command buffers in parallel

    // Loop through all graphics passes and record their command buffer
    for (std::size_t pass_index = 0; pass_index < m_graphics_passes.size(); pass_index++) {
        // This is important to know because of image layout transitions for back buffer for example
        const bool is_first_pass = (pass_index == 0);
        const bool is_last_pass = (pass_index == m_graphics_passes.size());
        record_command_buffer_for_pass(cmd_buf, *m_graphics_passes[pass_index], is_first_pass, is_last_pass, img_index);
    }
}

void RenderGraph::render() {
    const auto &cmd_buf = m_device.request_command_buffer("RenderGraph::render");
    record_command_buffers(cmd_buf, m_swapchain.acquire_next_image_index());
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

void RenderGraph::update_buffers() {
    for (const auto &buffer : m_buffers) {
        // Call the update lambda of the buffer (if specified)
        if (buffer->m_on_update) {
            std::invoke(buffer->m_on_update.value());
        }
    }
    // TODO: Batch barriers for updates which require staging buffer
}

void RenderGraph::update_textures() {
    for (const auto &texture : m_textures) {
        // Call the update lambda of the texture (if specified)
        if (texture->m_on_update) {
            std::invoke(texture->m_on_update.value());
        }
    }
    // TODO: Batch barriers for updates which require staging buffer
}

void RenderGraph::update_descriptor_sets() {
    // The problem is here that the binding is determined by the oder we call the add methods of the descriptor set
    // layout builder, but that is determined by the order we iterate through buffer and texture reads. Those reads
    // would either have to be in one struct or some other ordering must take place!!! If not, this will cause trouble
    // if a pass reads from both let's say a uniform buffer and a texture, but the order specified in descriptor set
    // layout builder results in a binding order that is incorrect!
}

void RenderGraph::update_push_constant_ranges() {
    // TODO: Update push constant ranges in parallel
    // Loop through all push constant ranges and call their update function
    for (const auto &push_constant : m_push_constant_ranges) {
        std::invoke(push_constant->m_on_update);
    }
}

void RenderGraph::validate_render_graph() {
    check_for_cycles();
}

} // namespace inexor::vulkan_renderer::render_graph
