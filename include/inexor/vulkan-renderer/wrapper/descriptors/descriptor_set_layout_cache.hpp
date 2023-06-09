#pragma once

#include "inexor/vulkan-renderer/wrapper/descriptors/descriptor_set_layout.hpp"

#include <volk.h>

#include <unordered_map>
#include <vector>

// Forward declaration
namespace inexor::vulkan_renderer::wrapper {
class Device;
}

namespace inexor::vulkan_renderer::wrapper::descriptors {

// Forward declaration
class DescriptorBuilder;

///
struct DescriptorSetLayoutInfo {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    [[nodiscard]] bool operator==(const DescriptorSetLayoutInfo &other) const;
    [[nodiscard]] std::size_t hash() const;
};

///
struct DescriptorSetLayoutHash {
    std::size_t operator()(const DescriptorSetLayoutInfo &k) const {
        return k.hash();
    }
};

/// A class for caching VkDescriptorSetLayouts with the help of std::unordered_map and a hashing function
/// For internal use inside of rendergraph only!
class DescriptorSetLayoutCache {
    friend DescriptorBuilder;

private:
    /// The device wrapper
    const Device &m_device;

    /// The actual descriptor set layout cache
    /// Note that std::unordered_map can accept a third template parameter which is the hash function
    std::unordered_map<DescriptorSetLayoutInfo, DescriptorSetLayout, DescriptorSetLayoutHash> m_cache;

public:
    /// Default constructor
    /// @param device The device wrapper
    DescriptorSetLayoutCache(const Device &device);

    /// Create a descriptor set layout with the help of the cache
    /// @param descriptor_set_layout_ci The descriptor set layout create info
    /// @return The descriptor set layout that was created
    [[nodiscard]] VkDescriptorSetLayout
    create_descriptor_set_layout(VkDescriptorSetLayoutCreateInfo descriptor_set_layout_ci);

    DescriptorSetLayoutCache(const DescriptorSetLayoutCache &) = delete;
    DescriptorSetLayoutCache(DescriptorSetLayoutCache &&) noexcept;
    ~DescriptorSetLayoutCache();

    DescriptorSetLayoutCache &operator=(const DescriptorSetLayoutCache &) = delete;
    DescriptorSetLayoutCache &operator=(DescriptorSetLayoutCache &&) = delete;
};

} // namespace inexor::vulkan_renderer::wrapper::descriptors
