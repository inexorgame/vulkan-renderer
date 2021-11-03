#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_gpu_data.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

namespace inexor::vulkan_renderer::gltf {

class ModelRenderer {
private:
    void draw_node(const ModelGpuData &model, const ModelNode &node, const wrapper::CommandBuffer &cmd_buf,
                   VkPipelineLayout layout);

public:
    ModelRenderer() = default;
    ModelRenderer(const ModelRenderer &) = delete;
    ModelRenderer(ModelRenderer &&) = delete;
    ~ModelRenderer() = default;

    ModelRenderer &operator=(const ModelRenderer &) = delete;
    ModelRenderer &operator=(ModelRenderer &&) = delete;

    void setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer, const TextureResource *depth_buffer,
                     const std::vector<wrapper::Shader> &shaders, const ModelGpuData &model);
};

} // namespace inexor::vulkan_renderer::gltf
