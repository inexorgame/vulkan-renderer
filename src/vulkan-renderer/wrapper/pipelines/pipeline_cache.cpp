#include "inexor/vulkan-renderer/wrapper/pipelines/pipeline_cache.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/pipelines/graphics_pipeline.hpp"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <utility>

namespace inexor::vulkan_renderer::wrapper::pipelines {

PipelineCache::PipelineCache(const Device &device) : m_device(device) {
    // Sanitize GPU name to only contain alphanumeric characters and underscores
    const auto &gpu_name = m_device.gpu_name();
    std::string sanitized_gpu_name;
    sanitized_gpu_name.reserve(gpu_name.size());
    for (const char c : gpu_name) {
        if (std::isalnum(static_cast<unsigned char>(c))) {
            sanitized_gpu_name.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else if (c == ' ' || c == '-') {
            // Replace spaces and hyphens with underscores
            sanitized_gpu_name.push_back('_');
        }
        // Skip any other special characters
    }

    // Avoid consecutive underscores
    sanitized_gpu_name.erase(std::unique(sanitized_gpu_name.begin(), sanitized_gpu_name.end(),
                                         [](char a, char b) { return a == '_' && b == '_'; }),
                             sanitized_gpu_name.end());

    // Remove leading/trailing underscores
    if (!sanitized_gpu_name.empty() && sanitized_gpu_name.front() == '_') {
        sanitized_gpu_name.erase(0, 1);
    }
    if (!sanitized_gpu_name.empty() && sanitized_gpu_name.back() == '_') {
        sanitized_gpu_name.pop_back();
    }

    // Fallback if sanitization resulted in empty string
    if (sanitized_gpu_name.empty()) {
        sanitized_gpu_name = "unknown_gpu";
    }

    // Generate a unique cache filename based on the sanitized GPU name and pipeline cache UUID
    const auto uuid = m_device.pipeline_cache_uuid();
    std::stringstream cache_name;
    cache_name << sanitized_gpu_name << "_";
    for (const auto byte : uuid) {
        cache_name << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(byte);
    }
    cache_name << ".cache";
    m_cache_file_name = cache_name.str();

    const auto pipeline_cache_data = read_cache_data_from_disk();

    // TODO: Do we need to set flag VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT?
    const auto pipeline_cache_ci = wrapper::make_info<VkPipelineCacheCreateInfo>({
        .initialDataSize = pipeline_cache_data.size(),
        .pInitialData = pipeline_cache_data.data(),
    });

    if (const auto result = vkCreatePipelineCache(m_device.device(), &pipeline_cache_ci, nullptr, &m_pipeline_cache);
        result != VK_SUCCESS) {
        throw VulkanException("vkCreatePipelineCache failed!", result, "m_pipeline_cache");
    }
    m_device.set_debug_name(m_pipeline_cache, "Pipeline Cache");
}

PipelineCache::~PipelineCache() {
    save_cache_data_to_disk();
    vkDestroyPipelineCache(m_device.device(), m_pipeline_cache, nullptr);
}

std::vector<uint8_t> PipelineCache::read_cache_data_from_disk() {
    // The data if the Vulkan pipeline cache (if any exists yet)
    std::vector<std::uint8_t> pipeline_cache_data;
    // Check if the vulkan pipeline cache file already exists
    if (std::filesystem::exists(m_cache_file_name)) {
        // Load the pipeline cache file
        std::ifstream cache_file_stream(m_cache_file_name, std::ios::binary | std::ios::ate);
        if (cache_file_stream) {
            // Store the size of the pipeline cache file
            const auto cache_file_size = static_cast<std::size_t>(cache_file_stream.tellg());
            if (cache_file_size == -1) {
                spdlog::error("Error: File stream tellg() returned -1!");
            } else {
                // Set the read position to start
                cache_file_stream.seekg(0, std::ios::beg);
                // Reserve memory for the file
                pipeline_cache_data.resize(cache_file_size);
                // Read the data from the file
                if (!cache_file_stream.read(reinterpret_cast<char *>(pipeline_cache_data.data()),
                                            pipeline_cache_data.size())) {
                    spdlog::error("Error: Could not load Vulkan pipeline cache '{}'!", m_cache_file_name);
                    pipeline_cache_data.clear();
                } else {
                    spdlog::trace("Loaded {} bytes from Vulkan pipeline cache '{}'.", cache_file_size,
                                  m_cache_file_name);
                }
            }
        } else {
            // This is an error, but not worth an exception
            // We simply create a new pipeline cache while running the application, and save it on exit
            spdlog::error("Error: Could not load Vulkan pipeline cache '{}'!", m_cache_file_name);
        }
    } else {
        // This is not an error at all, just likely the first the the user starts the application
        spdlog::trace("Vulkan pipeline cache file '{}' does not exist yet.", m_cache_file_name);
        spdlog::trace("A new Vulkan pipeline cache will written to disk at shutdown.");
    }
    return pipeline_cache_data;
}

void PipelineCache::save_cache_data_to_disk() {
    std::size_t cache_size = 0;
    if (m_pipeline_cache == VK_NULL_HANDLE) {
        spdlog::error("Vulkan pipeline cache cannot be saved to disk!");
        return;
    }
    auto result = vkGetPipelineCacheData(m_device.device(), m_pipeline_cache, &cache_size, nullptr);
    if (result == VK_SUCCESS) {
        if (cache_size > 0) {
            // We construct cache_data with the size, so it internally calls resize(cache_size), not resize(cache_size)!
            std::vector<uint8_t> cache_data(cache_size);
            // Get the cache data
            result = vkGetPipelineCacheData(m_device.device(), m_pipeline_cache, &cache_size, cache_data.data());
            if (result == VK_SUCCESS) {
                // We just overwrite existing Vulkan pipeline cache files by default!
                std::ofstream cache_file(m_cache_file_name, std::ios::binary);
                if (cache_file) {
                    // Write Vulkan pipeline cache to disk
                    cache_file.write(reinterpret_cast<char *>(cache_data.data()), cache_size);
                    spdlog::trace("Writing {} bytes to Vulkan pipeline cache file '{}'.", cache_size,
                                  m_cache_file_name);
                } else {
                    // Maybe the file path was set incorrectly?
                    spdlog::error("Error: Could not create file '{}' to write Vulkan pipeline cache to file!",
                                  m_cache_file_name);
                }
            } else {
                // This should be a rare error!
                spdlog::error("Error: Could not retrieve Vulkan pipeline cache data with vkGetPipelineCacheData!");
            }
        } else {
            // In this case, we probably forgot to pass the Vulkan pipeline cache handle during pipeline creation!
            spdlog::warn("Warning: Vulkan pipeline cache is empty at application shutdown!");
        }
    } else {
        // No exception thrown because we are in a destructor!
        spdlog::error("Error: vkGetPipelineCacheData returned {}!", tools::as_string(result));
    }
}

} // namespace inexor::vulkan_renderer::wrapper::pipelines
