#include "inexor/vulkan-renderer/skybox/skybox_gpu_data.hpp"

namespace inexor::vulkan_renderer::skybox {

void SkyboxGpuData::setup_rendering_resources(RenderGraph *render_graph) {}

SkyboxGpuData::SkyboxGpuData(const wrapper::Device &device_wrapper, RenderGraph *render_graph,
                             const gltf::ModelFile &model_file, glm::mat4 model_matrix, glm::mat4 proj_matrix)
    : ModelGpuData(device_wrapper, render_graph, model_file, model_matrix, proj_matrix) {

    setup_rendering_resources(render_graph);
}

} // namespace inexor::vulkan_renderer::skybox
