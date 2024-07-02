#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"
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

void RenderGraph::add_graphics_pipeline(std::string name, GraphicsPipelineCreateFunction on_pipeline_create) {
    if (name.empty()) {
        throw std::invalid_argument("[RenderGraph::add_graphics_pipeline] Error: Parameter 'on_pipeline_create' is an "
                                    "empty std::string! You must give a name to every rendergraph resource!");
    }
    m_pipeline_create_functions.emplace_back(std::move(on_pipeline_create));
}

std::shared_ptr<Buffer> RenderGraph::add_uniform_buffer(std::string uniform_buffer_name,
                                                        std::optional<std::function<void()>> on_update) {
    if (uniform_buffer_name.empty()) {
        throw std::runtime_error("[RenderGraph::add_uniform_buffer] Error: Parameter 'uniform_buffer_name' is an empty "
                                 "std::string! You must give a name to every rendergraph resource!");
    }
    m_buffers.emplace_back(std::make_shared<Buffer>(m_device, std::move(uniform_buffer_name), std::move(on_update)));
    return m_buffers.back();
}

std::shared_ptr<Buffer> RenderGraph::add_index_buffer(std::string index_buffer_name,
                                                      std::optional<std::function<void()>> on_update) {
    if (index_buffer_name.empty()) {
        throw std::invalid_argument("[RenderGraph::add_index_buffer] Error: Parameter 'index_buffer_name' is an empty "
                                    "std::string! You must give a name to every rendergraph resource!");
    }
    m_buffers.emplace_back(
        std::make_shared<Buffer>(m_device, std::move(index_buffer_name), VK_INDEX_TYPE_UINT32, std::move(on_update)));
    return m_buffers.back();
}

std::shared_ptr<Shader>
RenderGraph::add_shader(std::string shader_name, const VkShaderStageFlagBits shader_stage, std::string file_name) {
    // TODO: Check if shader names are unique or not?
    if (shader_name.empty()) {
        throw std::invalid_argument("[RenderGraph::add_shader] Error: Parameter 'shader_name' is an empty std::string! "
                                    "You must give a name to every rendergraph resource!");
    }
    m_shaders.emplace_back(
        std::make_shared<Shader>(m_device, std::move(shader_name), shader_stage, std::move(file_name)));
    return m_shaders.back();
}

std::shared_ptr<Texture> RenderGraph::add_texture(std::string texture_name,
                                                  const TextureUsage usage,
                                                  const VkFormat format,
                                                  std::optional<std::function<void()>> on_init,
                                                  std::optional<std::function<void()>> on_update) {
    if (texture_name.empty()) {
        throw std::invalid_argument(
            "[RenderGraph::add_texture] Error: Parameter 'texture_name' ist an empty std::string! "
            "You must give a name to every rendergraph resource!");
    }
    m_textures.emplace_back(
        std::make_shared<Texture>(std::move(texture_name), usage, format, std::move(on_init), std::move(on_update)));
    return m_textures.back();
}

std::shared_ptr<Buffer> RenderGraph::add_vertex_buffer(std::string vertex_buffer_name,
                                                       std::vector<VkVertexInputAttributeDescription> vertex_attributes,
                                                       std::optional<std::function<void()>> on_update) {
    if (vertex_buffer_name.empty()) {
        throw std::invalid_argument(
            "[RenderGraph::add_vertex_buffer] Error: Parameter 'vertex_buffer_name' is an empty std::string! "
            "You must give a name to every rendergraph resource!");
    }
    if (vertex_attributes.empty()) {
        throw std::invalid_argument(
            "[RenderGraph::add_vertex_buffer] Error: Parameter 'vertex_attributes' is an empty std::vector!");
    }
    m_buffers.emplace_back(
        std::make_shared<Buffer>(m_device, std::move(vertex_buffer_name), std::move(vertex_attributes)));
    return m_buffers.back();
}

void RenderGraph::check_for_cycles() {
    m_log->warn("Implement RenderGraph::check_for_cycles()!");
    // TODO: Implement!
}

void RenderGraph::compile() {
    m_log->trace("Compiling rendergraph");
    validate_render_graph();
    determine_pass_order();
    create_buffers();
    create_textures();
    // Buffers and textures must be created before graphics passes are created!
    create_graphics_passes();
    create_descriptor_sets();
    // Graphics pipelines must be created before calling set_on_record
    create_graphics_pipelines();
}

void RenderGraph::create_buffers() {
    for (const auto &buffer : m_buffers) {
        // Call the update lambda of the buffer (if specified)
        if (buffer->m_on_update) {
            std::invoke(buffer->m_on_update.value());
        }
        // TODO: Do we need an on_init here?
        buffer->create_buffer();
    }
    // TODO: Batch all updates which require staging buffers into one pipeline barrier call!
}

void RenderGraph::create_descriptor_sets() {
    m_log->trace("Creating descriptor sets");
    // TODO: Implement!
}

void RenderGraph::create_graphics_passes() {
    m_log->trace("Creating graphics passes");

    if (m_graphics_pass_create_functions.empty()) {
        throw std::runtime_error("Error: No graphics passes specified in rendergraph!");
    }
    m_graphics_passes.clear();
    m_graphics_passes.reserve(m_graphics_pass_create_functions.size());

    // Fill the vector of graphics passes by calling the corresponding callables (lambdas)
    for (const auto &pass_create_function : m_graphics_pass_create_functions) {
        // Call the graphics pass create function
        auto new_graphics_pass = std::invoke(pass_create_function, m_graphics_pass_builder);
        // We must check if the graphics pass that was created is valid!
        if (!new_graphics_pass) {
            throw std::runtime_error("Error: Failed to create graphics pass!");
        }
        // Store the graphics pass that was created
        m_graphics_passes.push_back(std::move(new_graphics_pass));
    }
}

void RenderGraph::create_graphics_pipelines() {
    m_log->trace("Creating graphics pipelines");

    m_graphics_pipelines.clear();
    const auto pipeline_count = m_pipeline_create_functions.size();
    m_graphics_pipelines.reserve(pipeline_count);

    // Call all graphics pipeline create functions
    for (std::size_t pipeline_index = 0; pipeline_index < pipeline_count; pipeline_index++) {
        // Call the graphics pipeline create function
        auto new_graphics_pipeline = std::invoke(m_pipeline_create_functions[pipeline_index],
                                                 m_graphics_pipeline_builder, m_descriptor_set_layout_builder);
        // Store the graphics pipeline that was created
        m_graphics_pipelines.push_back(std::move(new_graphics_pipeline));
    }
}

void RenderGraph::create_textures() {
    m_log->trace("Creating textures");

    for (const auto &texture : m_textures) {
        texture->create_texture();
    }
    // TODO: Batch all updates which require staging buffers into one pipeline barrier call!
}

void RenderGraph::determine_pass_order() {
    m_log->trace("Determing pass order using depth first search (DFS)");
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
        const bool is_last_pass = (pass_index == m_graphics_passes.size() - 1);
        record_command_buffer_for_pass(cmd_buf, *m_graphics_passes[pass_index], is_first_pass, is_last_pass, img_index);
    }
}

void RenderGraph::render() {
    const auto &cmd_buf = m_device.request_command_buffer("RenderGraph::render");
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
    for (const auto &buffer : m_buffers) {
        // If m_on_update is not std::nullopt, call the update function of the buffer
        if (buffer->m_on_update) {
            std::invoke(buffer->m_on_update.value());
        }
    }
    // TODO: Batch barriers for updates which require staging buffer
}

void RenderGraph::update_textures() {
    for (const auto &texture : m_textures) {
        // If m_on_update is not std::nullopt, call the update function of the texture
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

    // TODO: Builder pattern for descriptor writes?
}

void RenderGraph::validate_render_graph() {
    if (m_graphics_pass_create_functions.empty()) {
        throw std::runtime_error("Error: No graphics passes in rendergraph! Use add_graphics_pass!");
    }
    if (m_pipeline_create_functions.empty()) {
        throw std::runtime_error("Error: No graphics pipelines in rendergraph! Use add_graphics_pipeline!");
    }
    check_for_cycles();
}

} // namespace inexor::vulkan_renderer::render_graph
