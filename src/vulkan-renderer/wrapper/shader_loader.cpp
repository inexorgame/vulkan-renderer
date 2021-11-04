#include "inexor/vulkan-renderer/wrapper/shader_loader.hpp"

#include "inexor/vulkan-renderer/vk_tools/representation.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

ShaderLoader::ShaderLoader(const Device &device, const std::vector<ShaderLoaderJob> &jobs) {
    m_shaders.reserve(jobs.size());

    spdlog::trace("Loading {} shaders", jobs.size());

    for (const auto &job : jobs) {
        spdlog::trace("Loading {}: {} ({})", vk_tools::as_string(job.shader_type), job.file_name, job.debug_name);
        m_shaders.emplace_back(device, job.shader_type, job.file_name, job.debug_name);

        auto new_stage = wrapper::make_info<VkPipelineShaderStageCreateInfo>();
        new_stage.module = m_shaders.back().module();
        new_stage.stage = job.shader_type;
        new_stage.pName = m_shaders.back().entry_point().c_str();

        m_shader_stages.push_back(std::move(new_stage));
    }

    spdlog::trace("Loading shaders finished");
}

} // namespace inexor::vulkan_renderer::wrapper
