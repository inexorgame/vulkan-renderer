#pragma once

#include "inexor/vulkan-renderer/debug_marker_manager.hpp"
#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/manager_template.hpp"

#include <vulkan/vulkan.h>

#include <cassert>
#include <memory>
#include <mutex>

namespace inexor::vulkan_renderer {

///
class VulkanFenceManager : public ManagerClassTemplate<VkFence> {
private:
    bool fence_manager_initialised = false;

    std::mutex fence_manager_mutex;

    VkDevice device = VK_NULL_HANDLE;

    std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

public:
    VulkanFenceManager() = default;

    ~VulkanFenceManager() = default;

    /// @brief Initialises Vulkan fence manager.
    /// @param device [in] The Vulkan device.
    /// @param debug_marker_manager [in] A pointer to the debug marker manager.
    VkResult init(const VkDevice &device, std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager);

    /// @brief Checks if a fence with this name already exists.
    /// @param fence_name [in] The name of the fence.
    /// @return True if a fence with this name already exists, false otherwise.
    bool does_fence_exist(const std::string &fence_name);

    /// @brief Creates a new Vulkan fence.
    /// @param device [in] The Vulkan device.
    /// @param fence_name [in] The unique name of the fence.
    /// @param create_as_signaled [in] Describes if the fence will be created as signaled (turned on).
    std::optional<std::shared_ptr<VkFence>> create_fence(const std::string &fence_name, bool create_as_signaled = true);

    /// @brief Gets a certain fence by name.
    /// @param fence_name [in] The name of the fence.
    /// @return The acquired fence (if existent), std::nullopt otherwise.
    std::optional<std::shared_ptr<VkFence>> get_fence(const std::string &fence_name);

    /// @brief Destroys all existing fences.
    /// @param device [in] The Vulkan device.
    void shutdown_fences();
};

} // namespace inexor::vulkan_renderer
