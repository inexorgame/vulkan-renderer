#pragma once

#include <string>
#include <vector>
#include <cassert>

#include "spdlog/spdlog.h"
#include "vulkan/vulkan.h"

namespace inexor::vulkan_renderer {

/// @brief TODO
class Descriptor {
protected:
    std::string name = "";
    std::uint32_t number_of_images_in_swapchain = 0;

    std::vector<VkDescriptorSet> descriptor_sets = {};
    std::vector<VkWriteDescriptorSet> write_descriptor_sets = {};
    std::vector<VkDescriptorSetLayoutBinding> descriptor_set_layout_bindings = {};

    VkDescriptorSetLayout descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;
    VkDevice device = VK_NULL_HANDLE;

public:
    /// Delete the copy constructor so descriptors are move-only objects.
    Descriptor(const Descriptor &) = delete;
    Descriptor(Descriptor &&other) noexcept;

    /// Delete the copy assignment operator so descriptors are move-only objects.
    Descriptor &operator=(const Descriptor &) = delete;
    Descriptor &operator=(Descriptor &&) noexcept = default;

    /// @param device [in] The Vulkan device.
    /// @param number_of_images_in_swapchain [in] The number of images in the swapchain, usually 3 (triple buffering).
    Descriptor(const VkDevice device, const std::uint32_t number_of_images_in_swapchain, const std::string name);

    ~Descriptor();

    /// @brief Creates a descriptor pool.
    /// @param pool_types [in] An initializer list of types in the descriptor.
    void create_descriptor_pool(const std::initializer_list<VkDescriptorType> pool_types);

    /// @param descriptor_set_layout_bindings [in] 
    void create_descriptor_set_layouts(const std::vector<VkDescriptorSetLayoutBinding>& descriptor_set_layout_bindings);

    /// @brief
    /// @param 
    void add_descriptor_writes(const std::vector<VkWriteDescriptorSet>& descriptor_writes);
    
    /// 
    /// 
    void create_descriptor_sets();

    /// @brief Resets descriptor.
    /// @note This will be called when swapchain needs to be recreated.
    void reset(const bool clear_descriptor_layout_bindings = false);

    // TODO: Get methods here!
};

} // namespace inexor::vulkan_renderer
