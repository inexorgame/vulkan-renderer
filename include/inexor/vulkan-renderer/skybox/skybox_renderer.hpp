#pragma once

#include "inexor/vulkan-renderer/gltf/gpu_data.hpp"
#include "inexor/vulkan-renderer/gltf/node.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/skybox/skybox_gpu_data.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/shader_loader.hpp"

namespace inexor::vulkan_renderer::skybox {

/// A wrapper class for rendering skyboxes
/// In Inexor engine, a skybox is really just a glTF2 model
/// We didn't want to hardcode the skybox vertices into the engine code
/// This allows for more exotic sky geometries to be rendered in the future
class SkyboxRenderer {
private:
    const std::vector<wrapper::ShaderLoaderJob> m_shader_files{
        {"shaders/skybox/skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "skybox vertex shader"},
        {"shaders/skybox/skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "skybox fragment shader"}};

    wrapper::ShaderLoader m_shader_loader;

    void draw_node(VkCommandBuffer cmd_buf, const gltf::ModelNode *node);

public:
    /// Initialize skybox renderer by loading the skybox shaders
    SkyboxRenderer(const wrapper::Device &device, RenderGraph *render_graph);

    ///
    ///
    ///
    ///
    ///
    void setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer, const TextureResource *depth_buffer,
                     const SkyboxGpuData &skybox);
};

} // namespace inexor::vulkan_renderer::skybox
