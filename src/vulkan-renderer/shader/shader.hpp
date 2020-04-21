#pragma once

#include "vulkan-renderer/tools/file-loader/File.hpp"

#include <vulkan/vulkan.h>

#include <string>

namespace inexor::vulkan_renderer {

class InexorShader : public tools::InexorFile {
public:
    InexorShader() = default;

    ~InexorShader() = default;

    std::string name;

    std::string entry_name;

    VkShaderStageFlagBits type;

    VkShaderModule module;
};

} // namespace inexor::vulkan_renderer
