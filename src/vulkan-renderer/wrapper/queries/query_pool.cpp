#include "inexor/vulkan-renderer/wrapper/queries/query_pool.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"

#include <bit>
#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::queries {

QueryPool::QueryPool(Device &device, const std::uint32_t query_count)
    : QueryPool(device, VK_QUERY_TYPE_TIMESTAMP, query_count) {}

QueryPool::QueryPool(Device &device, const VkQueryType query_type, const std::uint32_t query_count,
                     const VkQueryPipelineStatisticFlags pipeline_statistics)
    : m_device(device), m_query_count(query_count), m_query_type(query_type),
      m_pipeline_statistics(pipeline_statistics) {
    if (m_query_count == 0) {
        throw std::invalid_argument("Error: Parameter 'query_count' must be greater than zero!");
    }
    if (m_query_type != VK_QUERY_TYPE_TIMESTAMP && m_query_type != VK_QUERY_TYPE_PIPELINE_STATISTICS) {
        throw std::invalid_argument("Error: Unsupported query type!");
    }
    if (m_query_type == VK_QUERY_TYPE_TIMESTAMP && m_pipeline_statistics != 0) {
        throw std::invalid_argument("Error: Timestamp query pools cannot specify pipeline statistics!");
    }
    if (m_query_type == VK_QUERY_TYPE_PIPELINE_STATISTICS) {
        if (m_pipeline_statistics == 0) {
            throw std::invalid_argument("Error: A pipeline statistics query pool requires at least one statistic!");
        }
        if (m_device.enabled_features().pipelineStatisticsQuery != VK_TRUE) {
            throw std::invalid_argument("Error: Device feature 'pipelineStatisticsQuery' is not enabled!");
        }
        m_values_per_query = static_cast<std::uint32_t>(std::popcount(m_pipeline_statistics));
    }

    const auto query_ci = tools::make_info<VkQueryPoolCreateInfo>({
        .queryType = m_query_type,
        .queryCount = m_query_count,
        .pipelineStatistics = m_pipeline_statistics,
    });
    if (const auto result = vkCreateQueryPool(m_device.device(), &query_ci, nullptr, &m_query_pool);
        result != VK_SUCCESS) {
        throw tools::InexorException("Error: vkCreateQueryPool failed!");
    }
}

QueryPool::~QueryPool() {
    if (m_query_pool != VK_NULL_HANDLE) {
        vkDestroyQueryPool(m_device.device(), m_query_pool, nullptr);
    }
}

std::vector<std::uint64_t> QueryPool::get_results(const std::uint32_t first_query,
                                                  const std::uint32_t query_count) const {
    const auto actual_query_count = query_count == 0 ? m_query_count - first_query : query_count;
    if (first_query >= m_query_count || actual_query_count == 0 || first_query + actual_query_count > m_query_count) {
        throw std::invalid_argument("Error: Query range is outside the query pool!");
    }
    const auto result_stride = static_cast<std::size_t>(m_values_per_query);
    std::vector<std::uint64_t> results(static_cast<std::size_t>(actual_query_count) * result_stride, 0);

    const auto result = vkGetQueryPoolResults(
        m_device.device(), m_query_pool, first_query, actual_query_count, sizeof(std::uint64_t) * results.size(),
        results.data(), sizeof(std::uint64_t) * result_stride, VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    if (result != VK_SUCCESS) {
        throw tools::VulkanException("Error: vkGetQueryPoolResults failed!", result);
    }

    return results;
}

std::optional<std::vector<std::uint64_t>> QueryPool::try_get_results(const std::uint32_t first_query,
                                                                     const std::uint32_t query_count) const {
    const auto actual_query_count = query_count == 0 ? m_query_count - first_query : query_count;
    if (first_query >= m_query_count || actual_query_count == 0 || first_query + actual_query_count > m_query_count) {
        throw std::invalid_argument("Error: Query range is outside the query pool!");
    }
    const auto result_stride = static_cast<std::size_t>(m_values_per_query) + 1;
    std::vector<std::uint64_t> results(static_cast<std::size_t>(actual_query_count) * result_stride, 0);

    const auto result = vkGetQueryPoolResults(m_device.device(), m_query_pool, first_query, actual_query_count,
                                              sizeof(std::uint64_t) * results.size(), results.data(),
                                              sizeof(std::uint64_t) * result_stride,
                                              VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
    if (result == VK_NOT_READY) {
        return std::nullopt;
    }
    if (result != VK_SUCCESS && result != VK_INCOMPLETE) {
        throw tools::VulkanException("Error: vkGetQueryPoolResults failed!", result);
    }

    return results;
}

} // namespace inexor::vulkan_renderer::wrapper::queries
