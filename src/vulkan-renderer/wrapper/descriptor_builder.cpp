#include "inexor/vulkan-renderer/wrapper/descriptor_builder.hpp"

#include "inexor/vulkan-renderer/wrapper/descriptor.hpp"
#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <spdlog/spdlog.h>

namespace inexor::vulkan_renderer::wrapper {

DescriptorBuilder::DescriptorBuilder(const Device &device, std::uint32_t swapchain_image_count)
    : m_device(device), m_swapchain_image_count(swapchain_image_count) {
    assert(m_device.device());
    assert(m_swapchain_image_count > 0);
}

ResourceDescriptor DescriptorBuilder::build(std::string name) {
    assert(!m_layout_bindings.empty());
    assert(!m_write_sets.empty());
    assert(m_write_sets.size() == m_layout_bindings.size());

    // Generate a new resource descriptor.
    ResourceDescriptor generated_descriptor(m_device, m_swapchain_image_count, m_layout_bindings, m_write_sets,
                                            std::move(name));

    m_layout_bindings.clear();
    m_write_sets.clear();
    m_descriptor_buffer_infos.clear();
    m_descriptor_image_infos.clear();

    return generated_descriptor;
};

}; // namespace inexor::vulkan_renderer::wrapper
