#include "inexor/vulkan-renderer/application.hpp"

#include <stdexcept>

int main(int argc, char *argv[]) {
    try {
        std::unique_ptr<inexor::vulkan_renderer::Application> renderer =
            std::make_unique<inexor::vulkan_renderer::Application>(argc, argv);

        renderer->run();
    } catch (const std::runtime_error &exception) {
        spdlog::critical(exception.what());
        return 1;
    } catch (const std::exception &exception) {
        spdlog::critical(exception.what());
        return 1;
    }

    spdlog::trace("Window closed");
}
