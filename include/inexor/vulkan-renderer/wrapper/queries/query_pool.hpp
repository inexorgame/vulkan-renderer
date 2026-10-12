#pragma once

#include <volk.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace inexor::vulkan_renderer::wrapper::core {
// Forward declaration
class Device;
} // namespace inexor::vulkan_renderer::wrapper::core

namespace inexor::vulkan_renderer::wrapper::queries {

// Using declaration
using vulkan_renderer::wrapper::core::Device;

class QueryPool {
private:
    const Device &m_device;
    VkQueryPool m_query_pool{VK_NULL_HANDLE};
    std::uint32_t m_query_count{0};
    VkQueryType m_query_type{VK_QUERY_TYPE_TIMESTAMP};
    VkQueryPipelineStatisticFlags m_pipeline_statistics{0};
    std::uint32_t m_values_per_query{1};

public:
    QueryPool(Device &device, std::uint32_t query_count = 2);

    QueryPool(Device &device, VkQueryType query_type, std::uint32_t query_count,
              VkQueryPipelineStatisticFlags pipeline_statistics = 0);

    ~QueryPool();

    [[nodiscard]] VkQueryPool query_pool() const {
        return m_query_pool;
    }

    [[nodiscard]] std::uint32_t query_count() const {
        return m_query_count;
    }

    [[nodiscard]] VkQueryType query_type() const {
        return m_query_type;
    }

    [[nodiscard]] VkQueryPipelineStatisticFlags pipeline_statistics() const {
        return m_pipeline_statistics;
    }

    /// Number of result values produced by one query, excluding its availability value.
    [[nodiscard]] std::uint32_t values_per_query() const {
        return m_values_per_query;
    }

    /// Results are laid out query-by-query. Pipeline-statistics values follow the bit order
    /// in pipeline_statistics().
    [[nodiscard]] std::vector<std::uint64_t> get_results(std::uint32_t first_query = 0,
                                                         std::uint32_t query_count = 0) const;

    /// Uses the same layout as get_results(), followed by one availability value per query.
    [[nodiscard]] std::optional<std::vector<std::uint64_t>> try_get_results(std::uint32_t first_query = 0,
                                                                            std::uint32_t query_count = 0) const;
};

} // namespace inexor::vulkan_renderer::wrapper::queries
