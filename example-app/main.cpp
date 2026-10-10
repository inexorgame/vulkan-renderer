#include "application.hpp"

#include <cstdint>
#include <optional>
#include <stdexcept>

int main(int argc, char *argv[]) {
    try {
        std::optional<std::uint32_t> gpu_override;
        do {
            auto renderer = std::make_unique<inexor::example_app::ExampleApp>(argc, argv, gpu_override);
            gpu_override = renderer->run();
        } while (gpu_override);
    } catch (const std::exception &exception) {
        spdlog::critical(exception.what());
        return 1;
    }
    return 0;
}
