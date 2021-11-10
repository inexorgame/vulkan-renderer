#pragma once

#include "inexor/vulkan-renderer/gltf/gltf_gpu_data.hpp"

namespace inexor::vulkan_renderer::skybox {

class SkyboxGpuData : public gltf::ModelGpuData {
private:
    // TODO: Rendering resources here

    void setup_rendering_resources(RenderGraph *render_graph);

public:
    SkyboxGpuData(const wrapper::Device &device_wrapper, RenderGraph *render_graph, const gltf::ModelFile &model_file,
                  glm::mat4 model_matrix, glm::mat4 proj_matrix);
};

} // namespace inexor::vulkan_renderer::skybox
