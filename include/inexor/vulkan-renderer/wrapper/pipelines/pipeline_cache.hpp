#pragma once

#include <volk.h>

#include <string>

namespace inexor::vulkan_renderer::wrapper {
// Forward declaration.
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::pipelines {

/// RAII wrapper for VkPipelineCache.
class PipelineCache {
private:
    VkPipelineCache m_cache;
    std::string m_cache_file_name;

    void load_from_file(const std::string &file_name);

    void store_to_file();

public:
    PipelineCache(const Device &device, const std::string &file_name);
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
