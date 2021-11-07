#pragma once

#include <string>

namespace inexor::vulkan_renderer::cubemap {

class CubemapCpuTexture {
private:
    // TODO: Separate this out into another data structure?
    std::uint32_t m_width{0};
    std::uint32_t m_height{0};
    std::uint32_t m_mip_levels{0};

public:
    CubemapCpuTexture(const std::string &file_name);
};

} // namespace inexor::vulkan_renderer::cubemap
