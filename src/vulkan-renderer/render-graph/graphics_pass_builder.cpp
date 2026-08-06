#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"

#include "inexor/vulkan-renderer/render-graph/buffer.hpp"
#include "inexor/vulkan-renderer/render-graph/graphics_pass.hpp"
#include "inexor/vulkan-renderer/render-graph/texture.hpp"
#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/swapchains/swapchain.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using tools::InexorException;

GraphicsPassBuilder::GraphicsPassBuilder() {
    reset();
}

GraphicsPassBuilder::GraphicsPassBuilder(GraphicsPassBuilder &&other) noexcept {
    m_on_record_cmd_buffer = std::move(other.m_on_record_cmd_buffer);
    m_swapchain_writes = std::move(other.m_swapchain_writes);
    m_texture_writes = std::move(other.m_texture_writes);
    m_buffer_reads = std::move(other.m_buffer_reads);
    m_buffer_writes = std::move(other.m_buffer_writes);
}

std::shared_ptr<GraphicsPass> GraphicsPassBuilder::build(std::string name, const DebugLabelColor pass_debug_color) {
    auto graphics_pass =
        std::make_shared<GraphicsPass>(std::move(name), std::move(m_on_record_cmd_buffer), std::move(m_buffer_reads),
                                       std::move(m_texture_writes), std::move(m_swapchain_writes), pass_debug_color);
    // NOTE: We could use RAII here to bind the call of reset() to some destructor call like a scope guard pattern does.
    reset();
    return graphics_pass;
}

GraphicsPassBuilder &GraphicsPassBuilder::conditionally_reads_from(std::weak_ptr<Buffer> buffer, bool condition) {
    // If the condition is true, we add the buffer to the list of buffer reads
    return condition ? reads_from(buffer) : *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::conditionally_writes_to(const TextureOrSwapchain write_attachment,
                                                                  bool condition,
                                                                  const std::optional<VkClearValue> clear_value) {
    // If the condition is true, we add the write attachment to the list of attachments to write to
    return condition ? writes_to(write_attachment, clear_value) : *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::reads_from(std::weak_ptr<Buffer> buffer) {
    if (buffer.expired()) {
        throw InexorException("Error: Parameter 'buffer' is an invalid pointer!");
    }
    m_buffer_reads.push_back(std::move(buffer));
    return *this;
}

void GraphicsPassBuilder::reset() {
    m_on_record_cmd_buffer = {};
    m_swapchain_writes.clear();
    m_texture_writes.clear();
    m_buffer_reads.clear();
    m_buffer_writes.clear();
}

GraphicsPassBuilder &
GraphicsPassBuilder::set_on_record(std::function<void(CommandBufferBuilder &)> on_record_cmd_buffer) {
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    return *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::writes_to(std::weak_ptr<Buffer> buffer) {
    if (buffer.expired()) {
        throw InexorException("Error: Parameter 'buffer' is an invalid pointer!");
    }
    m_buffer_writes.emplace_back(std::move(buffer));
    return *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::writes_to(TextureOrSwapchain write_attachment,
                                                    std::optional<VkClearValue> clear_value) {
    // Check if this is a std::weak_ptr<Texture>
    if (std::holds_alternative<std::weak_ptr<Texture>>(write_attachment)) {
        // This is a std::weak_ptr<Texture>, but we need to check if it's a valid pointer
        auto &texture = std::get<std::weak_ptr<Texture>>(write_attachment);
        // Check if the std::weak_ptr<Texture> is still a valid pointer
        if (texture.expired()) {
            throw InexorException("Error: Parameter 'write_attachment' is an invalid pointer!");
        }
        // It's a std::weak_ptr<Texture> and the memory is valid
        m_texture_writes.emplace_back(std::move(texture), std::move(clear_value));
    } else {
        // Otherwise, this must be a std::weak_ptr<Swapchain>! No need to check with std::holds_alternative explicitely.
        auto &swapchain = std::get<std::weak_ptr<Swapchain>>(write_attachment);
        // Check if the std::weak_ptr<Swapchain> is still a valid pointer
        if (swapchain.expired()) {
            throw InexorException("Error: Parameter 'write_attachment' is an invalid pointer!");
        }
        // It's a std::weak_ptr<Swapchain> and the memory is valid
        m_swapchain_writes.emplace_back(std::move(swapchain), std::move(clear_value));
    }
    return *this;
}

} // namespace inexor::vulkan_renderer::render_graph
