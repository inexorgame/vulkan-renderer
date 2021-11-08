#pragma once

#include "inexor/vulkan-renderer/wrapper/texture_base.hpp"

namespace inexor::vulkan_renderer::cubemap {

class GpuCubemap : public wrapper::TextureBase {
private:
    // Each cube has 6 faces
    static constexpr std::uint32_t FACE_COUNT{6};

public:
    ///
    ///
    ///
    GpuCubemap(const wrapper::Device &device, VkFormat format, std::uint32_t dim, std::uint32_t mip_levels,
               std::string name);

    ///
    ///
    ///
    void copy_from_image(VkCommandBuffer cmd_buf, VkImage source_image, std::uint32_t face, std::uint32_t mip_level,
                         std::uint32_t width, std::uint32_t height);
};

} // namespace inexor::vulkan_renderer::cubemap
