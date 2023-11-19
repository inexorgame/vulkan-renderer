#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_stage.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_stage_builder.hpp"
#include "inexor/vulkan-renderer/render-graph/push_constant_range_resource.hpp"
#include "inexor/vulkan-renderer/wrapper/buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <unordered_map>

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(wrapper::Device &device) : m_device(device) {}

std::weak_ptr<BufferResource> RenderGraph::add_buffer(std::string name, const BufferType type,
                                                      const DescriptorSetUpdateFrequency category,
                                                      std::optional<std::function<void()>> on_update) {
    if (name.empty()) {
        throw std::invalid_argument("Error: Buffer resource name must not be empty!");
    }

    // TODO: If on_update is nullopt, we can move it into m_const_buffer_resources

    // Add the buffer resource to the rendergraph
    // Note that while we require the programmer to specify the estimated descriptor set update frequency,
    // it is only used if this buffer resource is a uniform buffer, which requires a descriptor for it
    m_buffer_resources.emplace_back(
        std::make_shared<BufferResource>(std::move(name), type, category, std::move(on_update)));

    // Return a weak pointer to the buffer resource that was just created
    return m_buffer_resources.back();
}

void RenderGraph::add_graphics_stage(std::shared_ptr<GraphicsStage> graphics_stage) {
    // This way, the constructor arguments of GraphicsStage class can be passed into add_graphics_stage directly
    m_graphics_stages.push_back(std::move(graphics_stage));
}

std::weak_ptr<TextureResource> RenderGraph::add_texture(std::string name, const TextureUsage usage,
                                                        const VkFormat format) {
    if (name.empty()) {
        throw std::invalid_argument("Error: Texture resource name must not be empty!");
    }
    // Add the texture resource to the rendergraph
    m_texture_resources.emplace_back(std::make_shared<TextureResource>(name, usage, format));
    // Return a weak pointer to the texture resource that was just created
    return m_texture_resources.back();
}

void RenderGraph::check_for_cycles() {
    // TODO: Implement
    // TODO: throw std::logic_error in case the rendergraph contains cycles!
}

void RenderGraph::compile() {
    check_for_cycles();
    determine_stage_order();
    create_buffers();
    create_textures();
    record_command_buffers();
}

void RenderGraph::create_buffers() {
    // Loop through all buffer resources and create them
    for (auto &buffer : m_buffer_resources) {
        // We call the update method of each buffer before we create it because we need to know the initial size
        if (buffer->m_on_update) {
            // Only call update callback if it exists
            std::invoke(buffer->m_on_update.value());
        }

        // This maps from buffer usage to Vulkan buffer usage flags
        const std::unordered_map<BufferType, VkBufferUsageFlags> buffer_usage_flags{
            {BufferType::VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
            {BufferType::INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
            {BufferType::UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
        };

        // This maps from buffer usage to VMA memory usage flags
        const std::unordered_map<BufferType, VmaMemoryUsage> memory_usage_flags{
            // TODO: Change to VMA_MEMORY_USAGE_GPU and support staging buffers correctly!
            {BufferType::VERTEX_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU},
            // TODO: Change to VMA_MEMORY_USAGE_GPU and support staging buffers correctly!
            {BufferType::INDEX_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU},
            {BufferType::UNIFORM_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU},
        };

        // Create the physical buffer inside of the buffer resource wrapper
        // Keep in mind that this physical buffer can only be accessed from inside of the rendergraph
        buffer->m_buffer = std::make_unique<wrapper::Buffer>(m_device, buffer->m_data_size, // We must know the size
                                                             buffer_usage_flags.at(buffer->m_type),
                                                             memory_usage_flags.at(buffer->m_type), buffer->m_name);
    }
}

void RenderGraph::create_textures() {
    // Loop through all texture resources and create them
    for (auto &texture : m_texture_resources) {
        // TODO: Update texture here?
    }
}

void RenderGraph::determine_stage_order() {
    // TODO: Implement dfs
}

void RenderGraph::record_command_buffer(const std::shared_ptr<GraphicsStage> graphics_stage,
                                        const wrapper::CommandBuffer &cmd_buf, const bool is_first_stage,
                                        const bool is_last_stage) {
    // TODO: Implement
}

void RenderGraph::record_command_buffers() {
    // TODO: Implement
    /*for () {
        record_command_buffer(cmd_buf, stage);
    }
    */
}

void RenderGraph::render(const std::uint32_t swapchain_img_index, const VkSemaphore *img_available) {
    const auto &cmd_buf = m_device.request_command_buffer("RenderGraph::render");
    const std::array<VkPipelineStageFlags, 1> stage_mask{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    cmd_buf.submit_and_wait(wrapper::make_info<VkSubmitInfo>({
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = img_available,
        .pWaitDstStageMask = stage_mask.data(),
        .commandBufferCount = 1,
        .pCommandBuffers = cmd_buf.ptr(),
    }));
}

void RenderGraph::update_buffers() {
    // TODO: This can be done in parallel using taskflow library
    for (auto &buffer : m_buffer_resources) {
        if (!buffer->m_on_update) {
            // Not all buffers need an update
            // TODO: Sort buffers which do not need an update into separate vector! This could make iteration faster?
            continue;
        }
        // Call the update function of the buffer resource
        std::invoke(buffer->m_on_update.value());

        // Now does the physical buffer require an update?
        if (buffer->m_update_required) {
            // TODO: Recreate buffer if the size is bigger
            // Note that a recreate method would only accept the size!
            // TODO: Also recreate if the new size is smaller?
            if (buffer->m_requires_staging_buffer_update) {
                // TOOD: Batch staging buffer updates!
                // It should be okay to mark buffers as updated before all batched buffer updates?
            } else {
                // This buffer update does not require a staging buffer
                std::memcpy(buffer->m_buffer->memory(), buffer->m_data, buffer->m_data_size);
            }
            // The update has finished
            buffer->m_update_required = false;
        }
        // TODO: Execute batched copy operation(s) and state exactly one pipeline barrier!
    }
}

void RenderGraph::update_descriptor_sets() {
    // TOOD: Implement
}

void RenderGraph::update_push_constant_ranges() {
    // TODO: This can be done in parallel using taskflow library
    for (const auto &push_constant : m_push_constant_ranges) {
        push_constant->m_on_update();
    }
}

} // namespace inexor::vulkan_renderer::render_graph
