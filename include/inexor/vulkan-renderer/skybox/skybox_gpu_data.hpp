#pragma once

#include "inexor/vulkan-renderer/render_graph.hpp"
#include "inexor/vulkan-renderer/skybox/skybox_cpu_data.hpp"

namespace inexor::vulkan_renderer::skybox {

class SkyboxGpuData {
private:
    // TODO: This stuff could be in an abstract base class!
    BufferResource *m_vertex_buffer{nullptr};
    BufferResource *m_index_buffer{nullptr};

    void setup_rendering_resources(RenderGraph *render_graph);

    // ----------------------

public:
    SkyboxGpuData(const wrapper::Device &device_wrapper, RenderGraph *render_graph, const SkyboxCpuData &model_data);

    // TODO: This stuff could be in an abstract base class!

    [[nodiscard]] const BufferResource *vertex_buffer() const {
        return m_vertex_buffer;
    }

    [[nodiscard]] const BufferResource *index_buffer() const {
        return m_index_buffer;
    }

    // ----------------------
};

} // namespace inexor::vulkan_renderer::skybox
