#pragma once

#include <vulkan/vulkan_core.h>

#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

// Forward declaration of device wrapper
class Device;

/// @brief A wrapper for Vulkan query pools
class QueryPool {
private:
    const Device &m_device;
    VkPhysicalDeviceFeatures m_device_features{};
    VkQueryPool m_query_pool{};
    std::vector<std::uint64_t> m_pipeline_stats;
    std::vector<std::string> m_pipeline_stat_names;

    /// @brief These pipeline statistics are enabled by default if the default constructor is used.
    /// @note We are not storing these as VkQueryPipelineStatisticFlags, because we need to perform additional checks
    /// for some of these flags in order to use them. For example we need to check if tessellation is enabled in order
    /// to query its performance. Please note computer shaders do not require special checks.
    /// <a href="https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#queries-pipestats">
    /// Vulkan specification: Pipeline Statistics Queries</a>
    const std::vector<VkQueryPipelineStatisticFlagBits> default_pipeline_stats_flag_bits = {
        VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT,
        VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT,
        VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT,
        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT,
        VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT,
        VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT,
        VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT,                // requires geometry shaders
        VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT,                 // requires geometry shaders
        VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT,        // requires tesselation shaders
        VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT, // requires tesselation shaders
        VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT};

    std::string get_pipeline_stats_flag_bit_name(VkQueryPipelineStatisticFlagBits bit) const;

    /// @brief Validates every specified VkQueryPipelineStatisticFlagBits into one VkQueryPipelineStatisticFlags.
    /// Some VkQueryPipelineStatisticFlagBits values require special checks (tesselation shaders for example).
    /// @return A vector which contains the valid Vulkan pipeline query statistics flag bits.
    std::vector<VkQueryPipelineStatisticFlagBits>
    validate_pipeline_stats_flag_bits(const std::vector<VkQueryPipelineStatisticFlagBits> &pipeline_stats_flag_bits);

public:
    /// @brief Construct a Vulkan Query Pool using the default pipeline statistics.
    /// @param device The const reference to a device RAII wrapper instance.
    /// @param name The internal name.
    QueryPool(const Device &device, const std::string &name);

    /// Call <a href="https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCreateQueryPool">
    /// vkCreateQueryPool</a>
    /// @param device The device wrapper.
    /// @param name The internal name of this performance query.
    /// @param pipeline_stats_flag_bits The enabled <a
    /// href="https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#VkQueryPipelineStatisticFlagBits">
    /// Vulkan pipeline statistics flags</a>.
    QueryPool(const Device &device, const std::string &name,
              const std::vector<VkQueryPipelineStatisticFlagBits> &pipeline_stats_flag_bits);

    QueryPool(const QueryPool &) = delete;
    QueryPool(QueryPool &&) = delete;
    ~QueryPool();

    QueryPool operator=(const QueryPool &) = delete;
    QueryPool operator=(QueryPool &&) = delete;

    [[nodiscard]] const wrapper::Device &device() const noexcept {
        return m_device;
    }

    /// Call <a
    /// href="https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdResetQueryPool">
    /// vkCmdResetQueryPool</a>
    void reset(VkCommandBuffer cmd_buffer) const;

    /// Call <a href="https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdBeginQuery">
    /// vkCmdBeginQuery</a>
    void begin(VkCommandBuffer cmd_buffer) const;

    /// Call <a href="https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkCmdEndQuery">
    /// vkCmdEndQuery</a>
    void end(VkCommandBuffer cmd_buffer) const;

    /// Call <a
    /// href="https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#vkGetQueryPoolResults">
    /// vkGetQueryPoolResults</a>
    void get_results();

    /// TODO: Implement a get method for the results which returns a tuple of query name and result?

    /// Print all the captured pipeline statistics.
    void print_results() const;
};

} // namespace inexor::vulkan_renderer::wrapper
