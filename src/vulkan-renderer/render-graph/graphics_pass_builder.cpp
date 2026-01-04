#include "inexor/vulkan-renderer/render-graph/graphics_pass_builder.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"

#include <utility>

namespace inexor::vulkan_renderer::render_graph {

// Using declaration
using wrapper::InexorException;

GraphicsPassBuilder::GraphicsPassBuilder() {
    reset();
}

GraphicsPassBuilder::GraphicsPassBuilder(GraphicsPassBuilder &&other) noexcept {
    m_on_record_cmd_buffer = std::move(other.m_on_record_cmd_buffer);
    m_write_attachments = std::move(other.m_write_attachments);
    m_write_swapchains = std::move(other.m_write_swapchains);
    m_graphics_pass_reads = std::move(other.m_graphics_pass_reads);
}

std::shared_ptr<GraphicsPass> GraphicsPassBuilder::build(std::string name, const DebugLabelColor pass_debug_color) {
    auto graphics_pass = std::make_shared<GraphicsPass>(
        std::move(name), std::move(m_on_record_cmd_buffer), std::move(m_graphics_pass_reads),
        std::move(m_write_attachments), std::move(m_write_swapchains), pass_debug_color);
    // NOTE: We could use RAII here to bind the call of reset() to some destructor call like a scope_guard does.
    reset();
    return graphics_pass;
}

GraphicsPassBuilder &GraphicsPassBuilder::conditionally_reads_from(std::weak_ptr<GraphicsPass> graphics_pass,
                                                                   const bool condition) {
    if (!graphics_pass.expired() && condition) {
        m_graphics_pass_reads.push_back(std::move(graphics_pass));
    }
    // NOTE: No exception is thrown if this graphics pass is expired because it's an optional pass!
    return *this;
}

GraphicsPassBuilder &GraphicsPassBuilder::reads_from(std::weak_ptr<GraphicsPass> graphics_pass) {
    if (graphics_pass.expired()) {
        throw InexorException("Error: Parameter 'graphics_pass' is an invalid pointer!");
    }
    m_graphics_pass_reads.push_back(std::move(graphics_pass));
    return *this;
}

void GraphicsPassBuilder::reset() {
    m_on_record_cmd_buffer = {};
    m_graphics_pass_reads.clear();
    m_write_attachments.clear();
}

GraphicsPassBuilder &
GraphicsPassBuilder::set_on_record(std::function<void(const CommandBuffer &)> on_record_cmd_buffer) {
    m_on_record_cmd_buffer = std::move(on_record_cmd_buffer);
    return *this;
}

GraphicsPassBuilder &
GraphicsPassBuilder::writes_to(std::variant<std::weak_ptr<Texture>, std::weak_ptr<Swapchain>> write_attachment,
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
        m_write_attachments.emplace_back(std::move(texture), std::move(clear_value));
    } else {
        // Otherwise, this must be a std::weak_ptr<Swapchain>! No need to check with std::holds_alternative explicitely.
        auto &swapchain = std::get<std::weak_ptr<Swapchain>>(write_attachment);
        // Check if the std::weak_ptr<Swapchain> is still a valid pointer
        if (swapchain.expired()) {
            throw InexorException("Error: Parameter 'write_attachment' is an invalid pointer!");
        }
        m_write_swapchains.emplace_back(std::move(swapchain), std::move(clear_value));
    }
    return *this;
}

} // namespace inexor::vulkan_renderer::render_graph
