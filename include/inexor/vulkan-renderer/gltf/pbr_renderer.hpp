#pragma once

#include "inexor/vulkan-renderer/gltf/gpu_data.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/shader_loader.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <vector>

namespace inexor::vulkan_renderer::gltf {

class ModelPbrRenderer {
private:
    // TODO: Refactor so it's not using alpha_mode as parameter so it must no longer be called twice?
    void render_node(const ModelNode &node, VkDescriptorSet scene_descriptor_set, const wrapper::CommandBuffer &cmd_buf,
                     VkPipelineLayout pipeline_layout, const AlphaMode &alpha_mode);

    const std::vector<wrapper::ShaderLoaderJob> m_shader_files{
        {"shaders/gltf/pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "gltf vertex shader"},
        {"shaders/gltf/pbr_khr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "gltf fragment shader"}};

    wrapper::ShaderLoader m_shader_loader;

public:
    ModelPbrRenderer(const wrapper::Device &device);

    void setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer, const TextureResource *depth_buffer,
                     const ModelGpuPbrData &model);

    void render_model(const std::vector<std::shared_ptr<ModelNode>> &nodes, VkDescriptorSet scene_descriptor_set,
                      const wrapper::CommandBuffer &cmd_buf, VkPipelineLayout pipeline_layout);
};

} // namespace inexor::vulkan_renderer::gltf
