#include "inexor/vulkan-renderer/wrapper/shader_loader.hpp"

#include "inexor/vulkan-renderer/vk_tools/fill_vk_struct.hpp"
#include "inexor/vulkan-renderer/vk_tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

#include <cassert>

namespace inexor::vulkan_renderer::wrapper {

ShaderLoader::ShaderLoader(const Device &device, const std::vector<ShaderLoaderJob> &jobs, const std::string job_name) {
    assert(!jobs.empty());

    const std::size_t shader_count = jobs.size();
    std::size_t current_shader = 1;

    m_shaders.reserve(shader_count);
    m_shader_stage_cis.reserve(shader_count);

    spdlog::trace("Loading {} {} shaders", shader_count, job_name);

    for (const auto &job : jobs) {
        spdlog::trace("    ({}/{}) Loading {}: {} ({})", current_shader, shader_count,
                      vk_tools::as_string(job.shader_type), job.file_name, job.debug_name);

        m_shaders.emplace_back(device, job.shader_type, job.file_name, job.debug_name);

        const auto shader_stage_ci = vk_tools::fill_pipeline_shader_stage_ci(m_shaders.back().module(), job.shader_type,
                                                                             m_shaders.back().entry_point().c_str());

        m_shader_stage_cis.push_back(std::move(shader_stage_ci));

        current_shader++;
    }

    spdlog::trace("Finished loading {} shaders", job_name);
}

} // namespace inexor::vulkan_renderer::wrapper
