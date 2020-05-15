#include "inexor/vulkan-renderer/tools/cla_parser.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace inexor::vulkan_renderer::tools {

template <>
int CommandLineArgumentValue::as() const {
    return std::stoi(value);
}

template <>
bool CommandLineArgumentValue::as() const {
    if (value == "false") {
        return false;
    }

    if (value == "true") {
        return true;
    }

    return static_cast<bool>(as<int>());
}

template <>
std::uint32_t CommandLineArgumentValue::as() const {
    return static_cast<std::uint32_t>(as<int>());
}

std::optional<CommandLineArgumentTemplate> CommandLineArgumentParser::get_arg_template(const std::string &argument_name) const {
    // clang-format off
    auto it = std::find_if(accepted_args.begin(), accepted_args.end(), [&](const auto &accepted_arg) {
        return argument_name == accepted_arg.get_argument();
    });
    // clang-format on

    if (it == accepted_args.end()) {
        return std::nullopt;
    }

    return *it;
}

void CommandLineArgumentParser::parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        // NOLINTNEXTLINE
        std::string arg_name = {argv[i]};
        auto arg_template = get_arg_template(arg_name);

        if (!arg_template) {
            spdlog::warn("Unknown command line argument {}!", arg_name);
            continue;
        }

        if (!arg_template->does_take_values()) {
            parsed_arguments.insert(std::make_pair(arg_name, ""));
            continue;
        }

        if (i + 1 >= argc) {
            throw std::runtime_error("No value specified for argument " + arg_name);
        }

        // NOLINTNEXTLINE
        std::string arg_value = {argv[++i]};
        parsed_arguments.insert(std::make_pair(arg_name, arg_value));
    }
}

} // namespace inexor::vulkan_renderer::tools
