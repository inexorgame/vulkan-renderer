#pragma once

#include "inexor/vulkan-renderer/gltf/model_gpu_data.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/uniform_buffer.hpp"

namespace inexor::vulkan_renderer::gltf {

class ModelRenderer {
private:
    /// @brief Render a glTF model node.
    /// @param model The glTF2 model
    /// @param cmd_buf The command buffer
    /// @param layout The pipeline layout
    /// @param node The glTF model node
    void render_model_node(const ModelGpuData &model, const wrapper::CommandBuffer &cmd_buf, VkPipelineLayout layout,
                           const ModelNode &node);

    /// @brief Render the model.
    /// @param model The glTF2 model
    /// @param cmd_buf
    /// @param layout The pipeline layout
    void render_model_nodes(const ModelGpuData &model, const wrapper::CommandBuffer &cmd_buf, VkPipelineLayout layout);

public:
    /// @brief Default constructor.
    /// @param render_graph The rendergraph which is used
    /// @param back_buffer The back buffer which is used
    /// @param depth_buffer The depth buffer which is used
    /// @param shaders
    ModelRenderer(RenderGraph *render_graph, const TextureResource *back_buffer, const TextureResource *depth_buffer,
                  const std::vector<wrapper::Shader> &shaders);

    ModelRenderer() = delete;
    ModelRenderer(const ModelRenderer &) = delete;
    ModelRenderer(ModelRenderer &&) = delete;
    ~ModelRenderer() = default;

    ModelRenderer &operator=(const ModelRenderer &) = delete;
    ModelRenderer &operator=(ModelRenderer &&) = delete;

    /// @brief Render a glTF2 model's nodes.
    ///
    ///
    ///
    void setup_stage(RenderGraph *render_graph, const TextureResource *back_buffer, const TextureResource *depth_buffer,
                     const std::vector<wrapper::Shader> &shaders, const ModelGpuData &model);
};

} // namespace inexor::vulkan_renderer::gltf
