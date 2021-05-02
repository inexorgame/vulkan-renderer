#include <gtest/gtest.h>

#include "inexor/vulkan-renderer/meta.hpp"

#include <iostream>

int main(int argc, char **argv) {
    using namespace inexor::vulkan_renderer;

    // Print engine and application metadata.
    std::cout << m_engine_name << ", version " << m_engine_version_str << std::endl;
    std::cout << m_application_name << ", version " << m_application_version_str << std::endl;
    std::cout << "Configuration: " << m_build_type << ", Git SHA " << m_build_git << std::endl;

    testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();

    std::cout << "Press Enter to close" << std::endl;
    std::cin.get();
}
