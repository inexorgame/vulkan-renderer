#include "renderer.hpp"

#include "inexor/vulkan-renderer/wrapper/commands/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline_builder.hpp"
#include "standard_ubo.hpp"

#include <cstddef>

namespace inexor::example_app {

// Using declaration
using inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipelineBuilder;

ExampleAppBase::~ExampleAppBase() {
    spdlog::trace("Shutting down vulkan renderer");
    m_device->wait_idle();
}

} // namespace inexor::example_app
