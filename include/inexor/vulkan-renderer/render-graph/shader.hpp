#pragma once

#include <volk.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::pipelines {
// Forward declaration
class GraphicsPipelineBuilder;
} // namespace inexor::vulkan_renderer::wrapper::pipelines

namespace inexor::vulkan_renderer::render_graph {

// Forward declaration
class RenderGraph;

// TODO: Support shader specialization constants

// We don't want to type that out all the time
using inexor::vulkan_renderer::wrapper::Device;
using inexor::vulkan_renderer::wrapper::pipelines::GraphicsPipelineBuilder;

// TODO: This should not be in rendergraph!

/// RAII wrapper class for VkShaderModule
class Shader {
private:
    friend class RenderGraph;
    friend class GraphicsPipelineBuilder;

    const Device &m_device;
    std::string m_name;
    std::string m_file_name;
    VkShaderStageFlagBits m_shader_stage;
    VkShaderModule m_shader_module{VK_NULL_HANDLE};

    // TODO: Use a SPIR-V library like spirv-cross to deduce shader type automatically using shader reflection!

public:
    /// Load the shader file and call vkCreateShaderModule
    /// @param device The device wrapper
    /// @param name The internal debug name of the shader (not necessarily the file name)
    /// @param type The shader type
    /// @param file_name The shader file name
    Shader(const Device &device, std::string name, VkShaderStageFlagBits type, std::string file_name);

    Shader(const Shader &) = delete;
    Shader(Shader &&) noexcept;

    /// Call vkDestroyShaderModule
    ~Shader();

    Shader &operator=(const Shader &) = delete;
    Shader &operator=(Shader &&) = delete;
};

} // namespace inexor::vulkan_renderer::render_graph
