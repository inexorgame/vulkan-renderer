#pragma once

#include "inexor/vulkan-renderer/gltf/model.hpp"
#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

namespace inexor::vulkan_renderer::gltf {

class ModelRenderer {
private:
    RenderGraph *m_render_graph;
    const TextureResource *m_back_buffer;
    const TextureResource *m_depth_buffer;
    const std::vector<wrapper::Shader> &m_shaders;
    wrapper::DescriptorBuilder &m_descriptor_builder;
    std::vector<wrapper::ResourceDescriptor> m_descriptors;

    // Rendergraph buffers for glTF2 model geometry
    BufferResource *m_gltf_vertex_buffer{nullptr};
    BufferResource *m_gltf_index_buffer{nullptr};

public:
    ModelRenderer() = delete;

    /// @brief Default constructor.
    /// @param render_graph The rendergraph which is used
    /// @param back_buffer The back buffer which is used
    /// @param depth_buffer The depth buffer which is used
    /// @param shaders The shaders which are used
    /// @param descriptor_builder A const reference to a descriptor builder
    ModelRenderer(RenderGraph *render_graph, const TextureResource *back_buffer, const TextureResource *depth_buffer,
                  const std::vector<wrapper::Shader> &shaders, wrapper::DescriptorBuilder &descriptor_builder);

    ModelRenderer(const ModelRenderer &) = delete;
    ModelRenderer(ModelRenderer &&) = delete;

    ModelRenderer &operator=(const ModelRenderer &) = delete;
    ModelRenderer &operator=(ModelRenderer &&) = delete;

    /// @brief Render a glTF2 model's nodes.
    /// @param model The glTF2 model
    /// @param scene_index The scene index of the glTF2 model
    void render_model(const Model &model, std::size_t scene_index);
};

} // namespace inexor::vulkan_renderer::gltf
