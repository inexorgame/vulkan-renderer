#pragma once

#include <array>

namespace inexor::vulkan_renderer::example_app {

/// The following data will be filled when CMake configures this file
constexpr const char *APP_NAME{"example-app"};
constexpr std::array<std::uint32_t, 3> APP_VERSION{0, 1, 0};
constexpr const char *APP_VERSION_STR{"0.1.0"};
constexpr const char* ENGINE_NAME{"Inexor Engine"};
constexpr std::array<std::uint32_t, 3> ENGINE_VERSION{0, 1, 0};
constexpr const char *ENGINE_VERSION_STR{"0.1.0"};
constexpr const char *BUILD_GIT = "";
constexpr const char *BUILD_TYPE = "Release";

} // namespace inexor::vulkan_renderer::example_app
