#include "inexor/vulkan-renderer/application.hpp"

#include <stdexcept>

int main(int argc, char *argv[]) {
    try {
        using inexor::vulkan_renderer::Application;
        auto renderer = std::make_unique<Application>(argc, argv);
        renderer->run();
    } catch (const std::exception &exception) {
        spdlog::critical(exception.what());
        return 1;
    }
    return 0;
}
