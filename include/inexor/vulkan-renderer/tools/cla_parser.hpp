#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace inexor::vulkan_renderer::tools {

/// @brief Template class for creating arguments
class CommandLineArgumentTemplate {
    const std::string m_argument;
    const bool m_takes_values;

public:
    /// @param argument The argument to be passed on the command line (e.g --vsync)
    /// @param takes_args Whether this argument can take values (e.g --gpu 1)
    /// @note Only arguments that take zero or one values are supported
    CommandLineArgumentTemplate(std::string argument, bool takes_args)
        : m_argument(std::move(argument)), m_takes_values(takes_args) {}

    [[nodiscard]] const std::string &argument() const {
        return m_argument;
    }

    [[nodiscard]] bool takes_values() const {
        return m_takes_values;
    }
};

class CommandLineArgumentValue {
    const std::string m_value;

public:
    explicit CommandLineArgumentValue(std::string value) : m_value(std::move(value)) {}

    template <typename T>
    T as() const;
};

/// @brief A simple command line argument parser.
/// @note Only supports arguments with zero or one values (e.g --vsync or --gpu 1)
/// @note Only supports long arguments (e.g --<arg>)
/// @todo Support equals arguments and arguments with multiple values (e.g --gpus=1,2,3)
/// @todo Support short arguments with stacking (e.g -abc)
class CommandLineArgumentParser {
    /// @todo Allow runtime addition of accepted parameters
    const std::vector<CommandLineArgumentTemplate> m_accepted_args = {
        // Defines which GPU to use (by array index).
        {"--gpu", true},

        // Defines if we will print stats about graphics cards.
        {"--no-stats", false},

        // Use vertical synchronisation.
        {"--vsync", false},

        // Use RenderDoc layer.
        {"--renderdoc", false},

        // Disable Khronos validation layer.
        {"--no-validation", false},

        // Do not use distinct data transfer queue, use graphics queue.
        {"--no-separate-data-queue", false},

        // Disable debug markers (even if -renderdoc is specified)
        {"--no-vk-debug-markers", false}};

    std::unordered_map<std::string, CommandLineArgumentValue> m_parsed_arguments;

    std::optional<CommandLineArgumentTemplate> make_arg_template(const std::string &argument_name) const;

public:
    /// @brief Parses the command line arguments
    void parse_args(int argc, char **argv);

    template <typename T>
    std::optional<T> arg(const std::string &name) const {
        auto arg_template = make_arg_template(name);
        if (!arg_template) {
            return std::nullopt;
        }

        auto it = m_parsed_arguments.find(name);
        if (it == m_parsed_arguments.end()) {
            return std::nullopt;
        }

        if (!arg_template->takes_values()) {
            return true;
        }

        return it->second.as<T>();
    }

    /// @brief Returns the number of command line arguments.
    [[nodiscard]] std::size_t parsed_arg_count() const {
        return m_parsed_arguments.size();
    }
};

} // namespace inexor::vulkan_renderer::tools
