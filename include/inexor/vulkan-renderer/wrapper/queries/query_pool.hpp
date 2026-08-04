#pragma once

#include <volk.h>

#include <cstdint>
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

public:
    QueryPool(Device &device, std::uint32_t query_count = 2);

    ~QueryPool();

    [[nodiscard]] VkQueryPool query_pool() const {
        return m_query_pool;
    }

    [[nodiscard]] std::uint32_t query_count() const {
        return m_query_count;
    }

    [[nodiscard]] std::vector<std::uint64_t> get_results() const;
};

} // namespace inexor::vulkan_renderer::wrapper::queries
