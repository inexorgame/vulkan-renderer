#pragma once

#include <volk.h>

#include <string>
#include <vector>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
} // namespace inexor::vulkan_renderer::wrapper

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Forward declaration
class DescriptorAllocator;

/// RAII wrapper for VkDescriptorPool
/// For internal use inside of rendergraph only!
class DescriptorPool {
    friend DescriptorAllocator;

private:
    const Device &m_device;
    std::string m_name;
    VkDescriptorPool m_descriptor_pool{VK_NULL_HANDLE};
    std::vector<VkDescriptorPoolSize> m_pool_sizes;

public:
    // TODO: Make constructor and descriptor_pool() get method private!

    /// Default constructor
    /// @param device The device wrapper
    /// @param pool_sizes The descriptor pool sizes (must not be empty!)
    /// @param max_sets The max descriptor set count
    /// @param name The internal debug name of this descriptor pool (must not be empty!)
    /// @exception std::invalid_argument Internal debug name for descriptor pool must not be empty
    /// @exception std::invalid_argument Descriptor pool sizes must not be empty
    /// @exception VulkanException vkCreateDescriptorPool call failed
    DescriptorPool(const Device &device, std::vector<VkDescriptorPoolSize> pool_sizes, std::uint32_t max_sets,
                   std::string name);

    DescriptorPool(const DescriptorPool &) = delete;
    DescriptorPool(DescriptorPool &&) noexcept;
    ~DescriptorPool();

    DescriptorPool &operator=(const DescriptorPool &) = delete;
    DescriptorPool &operator=(DescriptorPool &&) = delete;

    [[nodiscard]] VkDescriptorPool descriptor_pool() const noexcept {
        return m_descriptor_pool;
    }
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
