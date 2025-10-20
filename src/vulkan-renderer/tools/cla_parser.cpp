#include "inexor/vulkan-renderer/tools/cla_parser.hpp"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace inexor::vulkan_renderer::tools {

template <>
[[nodiscard]] int CommandLineArgumentValue::as() const {
    return std::stoi(m_value);
}

template <>
[[nodiscard]] bool CommandLineArgumentValue::as() const {
    if (m_value == "false") {
        return false;
    }
    if (m_value == "true") {
        return true;
    }
    return static_cast<bool>(as<int>());
}

template <>
[[nodiscard]] std::uint32_t CommandLineArgumentValue::as() const {
    return static_cast<std::uint32_t>(as<int>());
}

std::optional<CommandLineArgumentTemplate>
CommandLineArgumentParser::make_arg_template(const std::string &argument_name) const {
    auto it = std::find_if(m_accepted_args.begin(), m_accepted_args.end(),
                           [&](const auto &accepted_arg) { return argument_name == accepted_arg.argument(); });
    if (it == m_accepted_args.end()) {
        return std::nullopt;
    }
    return *it;
}

// TODO: Take in an std::vector<std::string> of arguments? This would avoid any hard to debug out of bounds errors (with
//       argc being bigger than the number of elements actually in argv).
void CommandLineArgumentParser::parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        // NOLINTNEXTLINE
        std::string arg_name(argv[i]);

        // If a user enters an unrecognized argument, just warn them about it for now.
        auto arg_template = make_arg_template(arg_name);
        if (!arg_template) {
            spdlog::warn("Unknown command line argument {}!", arg_name);
            continue;
        }

        // Insert a dummy value if the argument doesn't take any.
        if (!arg_template->takes_values()) {
            m_parsed_arguments.insert(std::make_pair(arg_name, ""));
            continue;
        }

        // We have to fail if the user doesn't specify a value for an argument expecting one (since it would mess up the
        // rest of parsing).
        if (i++ >= argc) {
            throw std::runtime_error("Error: No value specified for argument " + arg_name);
        }

        // NOLINTNEXTLINE
        std::string arg_value(argv[i]);
        m_parsed_arguments.insert(std::make_pair(arg_name, arg_value));
    }
}

} // namespace inexor::vulkan_renderer::tools
