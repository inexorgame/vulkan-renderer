#pragma once

#include "inexor/vulkan-renderer/exceptions/exception.hpp"

#include <vulkan/vulkan_core.h>

namespace inexor::vulkan_renderer::exceptions {

/// @brief
class VulkanException final : public Exception {
private:
    ///@brief Return a VkResult's description text.
    /// @note This function can be used for both VkResult error and success values.
    /// @param result The VkResult return value which will be turned into a string.
    [[nodiscard]] static std::string get_vkresult_description(const VkResult result);

    /// @brief Turn a VkResult into a string.
    /// @note This function can be used for both VkResult error and success values.
    /// @param result The VkResult return value which will be turned into a string.
    [[nodiscard]] static std::string get_vkresult_string(const VkResult result);

public:
    /// @brief Default constructor.
    /// @param message The exception message.
    /// @param result The VkResult value of the Vulkan API call which failed.
    VulkanException(const std::string message, const VkResult result);
};

} // namespace inexor::vulkan_renderer::exceptions
