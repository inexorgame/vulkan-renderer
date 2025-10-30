#include "application.hpp"

#include <stdexcept>

int main(int argc, char *argv[]) {
    try {
        auto renderer = std::make_unique<inexor::example_app::ExampleApp>(argc, argv);
        renderer->run();
    } catch (const std::exception &exception) {
        spdlog::critical(exception.what());
        return 1;
    }
    return 0;
}
