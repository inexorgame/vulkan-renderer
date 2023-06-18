#include "inexor/vulkan-renderer/render-graph/render_graph.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/render-graph/descriptor_set_update_frequency_category.hpp"
#include "inexor/vulkan-renderer/render-graph/push_constant_range_resource.hpp"
#include "inexor/vulkan-renderer/wrapper/buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <unordered_map>

namespace inexor::vulkan_renderer::render_graph {

RenderGraph::RenderGraph(const wrapper::Device &device) : m_device(device) {}

BufferResource *RenderGraph::add_buffer(std::string name, const BufferUsage usage,
                                        const DescriptorSetUpdateFrequencyCategory category,
                                        std::function<void()> on_update) {
    if (name.empty()) {
        throw std::invalid_argument("Error: buffer resource name must not be empty!");
    }
    // Add the buffer resource to the rendergraph
    // Note that while we require the programmer to specify the estimated descriptor set update frequency,
    // it is only used if this buffer resource is a uniform buffer, which requires a descriptor for it
    m_buffer_resources.push_back(
        std::make_unique<BufferResource>(std::move(name), usage, category, std::move(on_update)));

    // Return a raw pointer to the buffer resource that was just created
    return m_buffer_resources.back().get();
}

void RenderGraph::build_graphics_pipeline(const RenderStage *stage) {
    // TODO: Implement
}

void RenderGraph::build_render_pass(const RenderStage *stage) {
    // TODO: Implement
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
    create_graphics_stages();
}

void RenderGraph::create_buffers() {
    // Loop through all buffer resources and create them
    for (auto &buffer : m_buffer_resources) {
        // We call the update method of each buffer before we create it because we need to know the initial size
        buffer->m_on_update();

        // This maps from buffer usage to Vulkan buffer usage flags
        const std::unordered_map<BufferUsage, VkBufferUsageFlags> buffer_usage_flags{
            {BufferUsage::VERTEX_BUFFER, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT},
            {BufferUsage::INDEX_BUFFER, VK_BUFFER_USAGE_INDEX_BUFFER_BIT},
            {BufferUsage::UNIFORM_BUFFER, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT},
        };

        // This maps from buffer usage to VMA memory usage flags
        const std::unordered_map<BufferUsage, VmaMemoryUsage> memory_usage_flags{
            // TODO: Change to VMA_MEMORY_USAGE_GPU and support staging buffers correctly!
            {BufferUsage::VERTEX_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU},
            // TODO: Change to VMA_MEMORY_USAGE_GPU and support staging buffers correctly!
            {BufferUsage::INDEX_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU},
            {BufferUsage::UNIFORM_BUFFER, VMA_MEMORY_USAGE_CPU_TO_GPU},
        };

        // Create the physical buffer inside of the buffer resource wrapper
        // Keep in mind that this physical buffer can only be accessed from inside of the rendergraph
        buffer->m_buffer = std::make_unique<wrapper::Buffer>(m_device, buffer->m_data_size, // We must know the size
                                                             buffer_usage_flags.at(buffer->m_usage),
                                                             memory_usage_flags.at(buffer->m_usage), buffer->m_name);
    }
}

void RenderGraph::create_graphics_stages() {
    // TODO: Implement
}

void RenderGraph::create_textures() {
    // TODO: Implement
}

void RenderGraph::determine_stage_order() {
    // TODO: Implement dfs
}

void RenderGraph::record_command_buffer() {
    // TODO: Implement
}

void RenderGraph::render(const std::uint32_t swapchain_img_index) {
    // TODO: Implement
}

void RenderGraph::update_buffers() {
    // TODO: This can be done in parallel using taskflow library
    for (auto &buffer : m_buffer_resources) {
        // Call the update function of the buffer resource
        buffer->m_on_update();

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
