#include "inexor/vulkan-renderer/wrapper/queries/query_pool.hpp"

#include "inexor/vulkan-renderer/tools/exception.hpp"
#include "inexor/vulkan-renderer/tools/make_info.hpp"
#include "inexor/vulkan-renderer/wrapper/core/device.hpp"

#include <stdexcept>

namespace inexor::vulkan_renderer::wrapper::queries {

QueryPool::QueryPool(Device &device, const std::uint32_t query_count) : m_device(device), m_query_count(query_count) {
    if (m_query_count == 0) {
        throw std::invalid_argument("Error: Parameter 'query_count' must be greater than zero!");
    }

    const auto query_ci = tools::make_info<VkQueryPoolCreateInfo>({
        .queryType = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount = m_query_count,
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

std::vector<std::uint64_t> QueryPool::get_results() const {
    std::vector<std::uint64_t> results(m_query_count, 0);

    const auto result =
        vkGetQueryPoolResults(m_device.device(), m_query_pool, 0, m_query_count, sizeof(std::uint64_t) * results.size(),
                              results.data(), sizeof(std::uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    if (result != VK_SUCCESS) {
        throw tools::VulkanException("Error: vkGetQueryPoolResults failed!", result);
    }

    return results;
}

} // namespace inexor::vulkan_renderer::wrapper::queries
