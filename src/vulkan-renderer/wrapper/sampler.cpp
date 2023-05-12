#include "inexor/vulkan-renderer/wrapper/sampler.hpp"

#include "inexor/vulkan-renderer/wrapper/device.hpp"
#include "inexor/vulkan-renderer/wrapper/make_info.hpp"

#include <utility>

namespace inexor::vulkan_renderer::wrapper {

Sampler::Sampler(const Device &device, const VkSamplerCreateInfo &sampler_ci, std::string name)
    : m_device(device), m_name(std::move(name)) {
    m_device.create_sampler(sampler_ci, &m_sampler, m_name);
}

Sampler::Sampler(const Device &device, std::string name)
    : Sampler(device,
              // Default sampler settings
              make_info<VkSamplerCreateInfo>({
                  .magFilter = VK_FILTER_LINEAR,
                  .minFilter = VK_FILTER_LINEAR,
                  .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
                  .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                  .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                  .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
                  .mipLodBias = 0.0f,
                  .anisotropyEnable = VK_FALSE,
                  .maxAnisotropy = 1.0f,
                  .compareEnable = VK_FALSE,
                  .compareOp = VK_COMPARE_OP_ALWAYS,
                  .minLod = 0.0f,
                  .maxLod = 0.0f,
                  .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                  .unnormalizedCoordinates = VK_FALSE,
              }),
              std::move(name)) {}

Sampler::~Sampler() {
    vkDestroySampler(m_device.device(), m_sampler, nullptr);
}

} // namespace inexor::vulkan_renderer::wrapper
