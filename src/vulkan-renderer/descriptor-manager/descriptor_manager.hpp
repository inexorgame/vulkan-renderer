#pragma once

#include "vulkan-renderer/class-templates/manager_template.hpp"
#include "vulkan-renderer/debug-marker/debug_marker_manager.hpp"
#include "vulkan-renderer/descriptor-pool/descriptor_pool.hpp"
#include "vulkan-renderer/descriptor-set/descriptor_bundle.hpp"
#include "vulkan-renderer/error-handling/error_handling.hpp"

#include <spdlog/spdlog.h>
#include <vulkan/vulkan.h>

#include <cassert>
#include <shared_mutex>
#include <string>
#include <vector>

namespace inexor {
namespace vulkan_renderer {

/// @class InexorDescriptorManager.
/// @brief A manager class for descriptor pools, descriptor set layouts and descriptor sets.
class InexorDescriptorManager : public ManagerClassTemplate<InexorDescriptorPool>, public ManagerClassTemplate<InexorDescriptorBundle> {
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
    InexorDescriptorManager() = default;

    ~InexorDescriptorManager() = default;

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
                                    std::shared_ptr<InexorDescriptorPool> &descriptor_pool);

    /// @brief Starts building a new descriptor.
    /// @param
    /// @param
    /// @param
    VkResult create_descriptor_bundle(const std::string &internal_descriptor_name, std::shared_ptr<InexorDescriptorPool> &descriptor_pool,
                                      std::shared_ptr<InexorDescriptorBundle> &descriptor_bundle);

    ///
    ///
    ///
    VkResult add_descriptor_set_layout_binding(std::shared_ptr<InexorDescriptorBundle> descriptor_bundle,
                                               const VkDescriptorSetLayoutBinding &descriptor_set_layout_binding);

    ///
    ///
    ///
    VkResult add_write_descriptor_set(std::shared_ptr<InexorDescriptorBundle> descriptor_bundle, const VkWriteDescriptorSet &write_descriptor_set);

    ///
    ///
    VkResult create_descriptor_set_layouts(std::shared_ptr<InexorDescriptorBundle> descriptor_bundle);

    ///
    ///
    VkResult create_descriptor_sets(std::shared_ptr<InexorDescriptorBundle> descriptor_bundle);

    ///
    ///
    ///
    std::optional<std::shared_ptr<InexorDescriptorBundle>> get_descriptor_bundle(const std::string &internal_descriptor_name);

    /// @brief Destroys all descriptor sets and descriptor pools.
    /// @param clear_descriptor_layout_bindings [in] True if descriptor set layout bindings should be cleared as well.
    /// This is only necessary when shutting down the entire application. This is not necessary when recreating swapchain!
    // Since we have to recreate swapchain more often, the default argument is set to false.
    VkResult shutdown_descriptors(bool clear_descriptor_layout_bindings = false);
};

}; // namespace vulkan_renderer
}; // namespace inexor
