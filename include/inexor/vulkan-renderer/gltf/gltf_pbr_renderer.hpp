#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_gpu_data.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/shader_loader.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

#include <vector>

namespace inexor::vulkan_renderer::gltf {

class ModelRenderer {
private:
    void render_node(const ModelNode &node, const VkDescriptorSet scene, const wrapper::CommandBuffer &cmd_buf,
                     const VkPipelineLayout &pipeline_layout, const AlphaMode &alpha_mode);

    const std::vector<wrapper::ShaderLoaderJob> m_shader_files{
        {"shaders/gltf/pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, "gltf vertex shader"},
        {"shaders/gltf/pbr_khr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, "gltf fragment shader"}};

    wrapper::ShaderLoader m_shader_loader;

public:
    ModelRenderer(const wrapper::Device &device);

    void setup_stage(RenderGraph *render_graph, const VkDescriptorSet scene, const TextureResource *back_buffer,
                     const TextureResource *depth_buffer, const ModelGpuData &model);
};

} // namespace inexor::vulkan_renderer::gltf
