#include "inexor/vulkan-renderer/skybox/skybox_gpu_data.hpp"

namespace inexor::vulkan_renderer::skybox {

void SkyboxGpuData::setup_rendering_resources(RenderGraph *render_graph) {
    // TODO: Implement
}

SkyboxGpuData::SkyboxGpuData(const wrapper::Device &device_wrapper, RenderGraph *render_graph,
                             const SkyboxCpuData &model_data) {

    setup_rendering_resources(render_graph);
}

} // namespace inexor::vulkan_renderer::skybox
