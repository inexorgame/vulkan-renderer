#include "inexor/vulkan-renderer/cubemap/cubemap_cpu_texture.hpp"

#include "inexor/vulkan-renderer/tools/file.hpp"

#include <spdlog/spdlog.h>

// Khronos texture format
#include <vulkan/vulkan.h>
#include <ktxvulkan.h>

namespace inexor::vulkan_renderer::cubemap {

CubemapCpuTexture::CubemapCpuTexture(const std::string &file_name) {

    const auto file_ext = tools::get_file_extension_lowercase(file_name);

    if (file_ext != "ktx") {
        throw std::runtime_error("Error: Currently only KTX format is supported for cubemaps!");
    }

    spdlog::trace("Loading cubemap texture {}", file_name);

    ktxTexture *texture = nullptr;

    const auto result =
        ktxTexture_CreateFromNamedFile(file_name.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);

    if (result != KTX_SUCCESS) {
        throw std::runtime_error("Error: ktxTexture_CreateFromNamedFile failed! Could not load ktx cubemap " +
                                 file_name + "!");
    }

    const auto texture_data = ktxTexture_GetData(texture);
    // const auto texture_size = ktxTexture_GetSize(texture);

    // TOOD: Once command buffer here!

    // Each cube has 6 faces
    constexpr std::uint32_t faces_on_cube = 6;

    ktxTexture_Destroy(texture);
}

} // namespace inexor::vulkan_renderer::cubemap
