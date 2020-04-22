#pragma once

#include "inexor/vulkan-renderer/debug_marker_manager.hpp"
#include "inexor/vulkan-renderer/descriptor_bundle.hpp"
#include "inexor/vulkan-renderer/descriptor_pool.hpp"
#include "inexor/vulkan-renderer/error_handling.hpp"
#include "inexor/vulkan-renderer/manager_template.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <cassert>
#include <shared_mutex>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer {

/// @class DescriptorManager.
/// @brief A manager class for descriptor pools, descriptor set layouts and descriptor sets.
class DescriptorManager : public ManagerClassTemplate<DescriptorPool>, public ManagerClassTemplate<DescriptorBundle> {
private:
    VkDevice device;

    std::size_t number_of_images_in_swapchain = 0;

    std::shared_mutex descriptor_manager_mutex;

    bool descriptor_manager_initialised = false;

    std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager;

    /// @brief Destroys all descriptor pools.
    VkResult destroy_descriptor_pools();

    /// @brief Destroys all descriptor sets.
    VkResult destroy_descriptor_sets();

public:
    DescriptorManager() = default;

    ~DescriptorManager() = default;

    /// @brief Initialises descriptor manager.
    /// @param device [in] The Vulkan device which is being used.
    /// @param number_of_images_in_swapchain [in] The number of images in swapchain.
    /// @param debug_marker_manager [in] The debug marker manager.
    VkResult init(const VkDevice &device, const std::size_t number_of_images_in_swapchain,
                  const std::shared_ptr<VulkanDebugMarkerManager> debug_marker_manager);

    /// @brief Creates a new descriptor pool.
    /// @param internal_descriptor_pool_name [in] The internal name of the descriptor pool.
    /// @param pool_sizes [in] The size of the descriptor pool data.
    /// @param descriptor_pool [in] A reference of a shared pointer to the descriptor pool which will be created.
    VkResult create_descriptor_pool(const std::string &internal_descriptor_pool_name, const std::vector<VkDescriptorPoolSize> &pool_sizes,
                                    std::shared_ptr<DescriptorPool> &descriptor_pool);

    /// @brief Starts building a new descriptor.
    /// @param
    /// @param
    /// @param
    VkResult create_descriptor_bundle(const std::string &internal_descriptor_name, std::shared_ptr<DescriptorPool> &descriptor_pool,
                                      std::shared_ptr<DescriptorBundle> &descriptor_bundle);

    ///
    ///
    ///
    VkResult add_descriptor_set_layout_binding(std::shared_ptr<DescriptorBundle> descriptor_bundle,
                                               const VkDescriptorSetLayoutBinding &descriptor_set_layout_binding);

    ///
    ///
    ///
    VkResult add_write_descriptor_set(std::shared_ptr<DescriptorBundle> descriptor_bundle, const VkWriteDescriptorSet &write_descriptor_set);

    ///
    ///
    VkResult create_descriptor_set_layouts(std::shared_ptr<DescriptorBundle> descriptor_bundle);

    ///
    ///
    VkResult create_descriptor_sets(std::shared_ptr<DescriptorBundle> descriptor_bundle);

    ///
    ///
    ///
    std::optional<std::shared_ptr<DescriptorBundle>> get_descriptor_bundle(const std::string &internal_descriptor_name);

    /// @brief Destroys all descriptor sets and descriptor pools.
    /// @param clear_descriptor_layout_bindings [in] True if descriptor set layout bindings should be cleared as well.
    /// This is only necessary when shutting down the entire application. This is not necessary when recreating swapchain!
    // Since we have to recreate swapchain more often, the default argument is set to false.
    VkResult shutdown_descriptors(bool clear_descriptor_layout_bindings = false);
};

} // namespace inexor::vulkan_renderer
