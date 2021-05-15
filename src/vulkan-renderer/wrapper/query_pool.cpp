#include "inexor/vulkan-renderer/wrapper/query_pool.hpp"

#include "inexor/vulkan-renderer/exception.hpp"
#include "inexor/vulkan-renderer/wrapper/command_buffer.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

std::vector<VkQueryPipelineStatisticFlagBits> QueryPool::validate_pipeline_stats_flag_bits(
    const std::vector<VkQueryPipelineStatisticFlagBits> &pipeline_stats_flag_bits) {
    std::vector<VkQueryPipelineStatisticFlagBits> ret_val;

    ret_val.reserve(default_pipeline_stats_flag_bits.size());

    for (const auto flag_bit : pipeline_stats_flag_bits) {
        switch (flag_bit) {
        case VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT:
            if (m_device_features.tessellationShader == VK_FALSE) {
                spdlog::warn("Can't add VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT to "
                             "pipeline statistics flag bit!");
                spdlog::warn(
                    "Tesselation shaders are not available on this gpu (device_features.tessellationShader = false)");
                break;
            } else {
                // Tesselation shaders are available so it's safe to add this flag.
                ret_val.emplace_back(VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT);
            }
            break;
        case VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT:
            if (m_device_features.tessellationShader == VK_FALSE) {
                spdlog::warn("Can't add VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT to "
                             "pipeline statistics flag bit!");
                spdlog::warn(
                    "Tesselation shaders are not available on this gpu (device_features.tessellationShader = false)");
                break;
            } else {
                // Tesselation shaders are available so it's safe to add this flag.
                ret_val.emplace_back(VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT);
            }
            break;
        case VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT:
            if (m_device_features.geometryShader == VK_FALSE) {
                spdlog::warn("Can't add VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT to pipeline "
                             "statistics flag bit!");
                spdlog::warn("Geometry shaders are not available on this gpu (device_features.geometryShader = false)");
                break;
            } else {
                // Geometry shaders are available so it's safe to add this flag.
                ret_val.emplace_back(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT);
            }
            break;
        case VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT:
            if (m_device_features.geometryShader == VK_FALSE) {
                spdlog::warn("Can't add VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT to pipeline "
                             "statistics flag bit!");
                spdlog::warn("Geometry shaders are not available on this gpu (device_features.geometryShader = false)");
                break;
            } else {
                // Geometry shaders are available so it's safe to add this flag.
                ret_val.emplace_back(VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT);
            }
            break;
        default:
            // No special check required for this flag.
            ret_val.emplace_back(flag_bit);
            break;
        }
    }

    ret_val.shrink_to_fit();
    return ret_val;
}

// TODO: Make this a as_string method and return a std::string_view from representation.cpp!
std::string QueryPool::get_pipeline_stats_flag_bit_name(const VkQueryPipelineStatisticFlagBits bit) {
    switch (bit) {
    case VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT:
        return "Input assembly vertex count";
    case VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT:
        return "Input assembly primitives count";
    case VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT:
        return "Vertex shader invocations";
    case VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT:
        return "Clipping stage primitives processed";
    case VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT:
        return "Clipping stage primitives output";
    case VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT:
        return "Fragment shader invocations";
    case VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_INVOCATIONS_BIT:
        return "Geometry shader invocations";
    case VK_QUERY_PIPELINE_STATISTIC_GEOMETRY_SHADER_PRIMITIVES_BIT:
        return "Geometry assembly primitives count";
    case VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_CONTROL_SHADER_PATCHES_BIT:
        return "Tessellation control shader patch invocations";
    case VK_QUERY_PIPELINE_STATISTIC_TESSELLATION_EVALUATION_SHADER_INVOCATIONS_BIT:
        return "Tessellation evaluation shader invocations";
    case VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT:
        return "Compute shader invocations";
    default:
        break;
    }
    return "Unknown";
}

QueryPool::QueryPool(const Device &device, const std::string &name)
    : QueryPool(device, name, default_pipeline_stats_flag_bits) {}

QueryPool::QueryPool(const Device &device, const std::string &name,
                     const std::vector<VkQueryPipelineStatisticFlagBits> &pipeline_stats_flag_bits)
    : m_device(device) {
    assert(m_device.device());
    assert(!name.empty());

    // We must first check if pipeline query statistics are available.
    vkGetPhysicalDeviceFeatures(m_device.physical_device(), &m_device_features);

    if (m_device_features.pipelineStatisticsQuery == VK_FALSE) {
        throw InexorException("Error: vkGetPhysicalDeviceFeatures shows pipelineStatisticsQuery is not supported");
    }

    // Compose pipeline stats flags from pipeline_stats_flag_bits.
    const std::vector<VkQueryPipelineStatisticFlagBits> valid_pipeline_stats_flag_bits{
        validate_pipeline_stats_flag_bits(pipeline_stats_flag_bits)};

    m_pipeline_stat_names.reserve(valid_pipeline_stats_flag_bits.size());

    VkQueryPipelineStatisticFlags pipeline_stats_flags{};

    // No special check required for this flag.
    for (const auto valid_bit : valid_pipeline_stats_flag_bits) {
        pipeline_stats_flags |= valid_bit;
        m_pipeline_stat_names.emplace_back(get_pipeline_stats_flag_bit_name(valid_bit));
    }

    auto query_pool_ci = make_info<VkQueryPoolCreateInfo>();

    query_pool_ci.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
    query_pool_ci.pipelineStatistics = pipeline_stats_flags;
    query_pool_ci.queryCount = static_cast<std::uint32_t>(valid_pipeline_stats_flag_bits.size());

    if (const auto result = vkCreateQueryPool(m_device.device(), &query_pool_ci, nullptr, &m_query_pool);
        result != VK_SUCCESS) {
        throw VulkanException("Error: vkCreateQueryPool failed!", result);
    }
}

void QueryPool::reset(const wrapper::CommandBuffer &cmd_buffer) const {
    assert(!m_pipeline_stats.empty());
    vkCmdResetQueryPool(cmd_buffer.get(), m_query_pool, 0, static_cast<std::uint32_t>(m_pipeline_stats.size()));
}

void QueryPool::begin(const wrapper::CommandBuffer &cmd_buffer) const {
    vkCmdBeginQuery(cmd_buffer.get(), m_query_pool, 0, 0);
}

void QueryPool::end(const wrapper::CommandBuffer &cmd_buffer) const {
    vkCmdEndQuery(cmd_buffer.get(), m_query_pool, 0);
}

void QueryPool::get_results() {
    vkGetQueryPoolResults(m_device.device(), m_query_pool, 0, 1,
                          static_cast<std::uint32_t>(m_pipeline_stats.size()) * sizeof(std::uint64_t),
                          m_pipeline_stats.data(), sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);
}

void QueryPool::print_results() const {
    for (std::size_t i = 0; i < m_pipeline_stats.size(); i++) {
        spdlog::info("{}: {}", m_pipeline_stat_names[i], m_pipeline_stats[i]);
    }
}

QueryPool::~QueryPool() {
    vkDestroyQueryPool(m_device.device(), m_query_pool, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
