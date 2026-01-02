#pragma once

#include "inexor/vulkan-renderer/wrapper/device.hpp"

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::pipelines {

// Forward declaration
class GraphicsPipeline;

/// RAII wrapper class for VkPipelineCache
/// We use one pipeline cache for all pipelines (no matter what type: graphics or compute)
class PipelineCache {
private:
    // We prefer friend declarations over public get methods
    friend class GraphicsPipeline;

    // The device wrapper
    const Device &m_device;

    // We need to store the file name of the pipeline cache because we will overwrite it on save
    std::string m_cache_file_name;

    /// NOTE: It could be that the pipeline cache is missing (at first start) or invalid for some reason
    /// (e.g. driver update), in which case this Vulkan handle remains as VK_NULL_HANLDE.
    VkPipelineCache m_pipeline_cache{VK_NULL_HANDLE};

    /// Attempt to read the existing Vulkan pipeline cache file from disk
    std::vector<std::uint8_t> read_cache_data_from_disk();

    /// Save the Vulkan pipeline cache to disk
    void save_cache_data_to_disk();

public:
    // TODO: Define the file name of the pipeline cache by hashing the gpu! Caches are gpu specific!

    /// Default constructor
    /// @param cache_file_name The name of the pipeline cache file to load
    PipelineCache(const Device &device);

    /// Write the Vulkan pipeline cache to file and destroy it with vkDestroyPipelineCache
    ~PipelineCache();
};

} // namespace inexor::vulkan_renderer::wrapper::pipelines
