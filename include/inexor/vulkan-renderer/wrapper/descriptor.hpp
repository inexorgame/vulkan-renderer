#pragma once

#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <string>
#include <vector>

namespace inexor::vulkan_renderer::wrapper {

/// @brief TODO
class Descriptor {
protected:
    std::string m_name = "";
    std::uint32_t m_number_of_images_in_swapchain = 0;

    std::vector<VkDescriptorSet> m_descriptor_sets{};
    std::vector<VkWriteDescriptorSet> m_write_descriptor_sets{};
    std::vector<VkDescriptorSetLayoutBinding> m_descriptor_set_layout_bindings{};

    VkDescriptorSetLayout m_descriptor_set_layout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptor_pool = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;

public:
    /// Delete the copy constructor so descriptors are move-only objects.
    Descriptor(const Descriptor &) = delete;
    Descriptor(Descriptor &&other) noexcept;

    /// Delete the copy assignment operator so descriptors are move-only objects.
    Descriptor &operator=(const Descriptor &) = delete;
    Descriptor &operator=(Descriptor &&) noexcept = default;

    /// @param device [in] The Vulkan device.
    /// @param number_of_images_in_swapchain [in] The number of images in the swapchain, usually 3 (triple buffering).
    Descriptor(VkDevice device, std::uint32_t number_of_images_in_swapchain, const std::string &name);

    ~Descriptor();

    void create_descriptor_pool(const std::initializer_list<VkDescriptorType> pool_types);

    void create_descriptor_set_layouts(const std::vector<VkDescriptorSetLayoutBinding> &descriptor_set_layout_bindings);

    void add_descriptor_writes(const std::vector<VkWriteDescriptorSet> &descriptor_writes);

    void create_descriptor_sets();

    /// @brief Resets descriptor.
    /// @note This will be called when swapchain needs to be recreated.
    void reset(const bool clear_descriptor_layout_bindings = false);

    const auto descriptor_sets_data() const {
        return m_descriptor_sets.data();
    }

    const auto descriptor_set_layout() const {
        return m_descriptor_set_layout;
    }
};

} // namespace inexor::vulkan_renderer::wrapper
