#include "inexor/vulkan-renderer/application.hpp"

#include <stdexcept>

int main(int argc, char *argv[]) {
    try {
        using inexor::vulkan_renderer::Application;
        std::unique_ptr<Application> renderer = std::make_unique<Application>(argc, argv);
        renderer->run();
    } catch (const std::runtime_error &exception) {
        spdlog::critical(exception.what());
        return 1;
    } catch (const std::exception &exception) {
        spdlog::critical(exception.what());
        return 1;
    }
    return 0;
}
