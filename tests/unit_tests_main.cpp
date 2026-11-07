#include <gtest/gtest.h>

#include "inexor/vulkan-renderer/meta/meta.hpp"

#include <iostream>

int main(int argc, char **argv) {
    using namespace inexor::vulkan_renderer::meta;

    // Print engine and application metadata.
    std::cout << ENGINE_NAME << ", version " << ENGINE_VERSION_STR << std::endl;
    std::cout << APP_NAME << ", version " << APP_VERSION_STR << std::endl;
    std::cout << "Configuration: " << BUILD_TYPE << ", Git SHA " << BUILD_GIT << std::endl;

    const bool is_interactive = (argc <= 1);
    testing::InitGoogleTest(&argc, argv);
    const int result = RUN_ALL_TESTS();

    if (is_interactive) {
        std::cout << "Press Enter to close" << std::endl;
        std::cin.get();
    }
    return result;
}
