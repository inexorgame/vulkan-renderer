#include "inexor/vulkan-renderer/tools/cla_parser.hpp"

namespace inexor::vulkan_renderer::tools {

bool InexorCommandLineArgumentParser::does_command_line_argument_template_exist(const std::string argument_name) {
    for (const auto &accepted_argument : list_of_accepted_command_line_arguments) {
        if (0 == accepted_argument.argument_name.compare(argument_name)) {
            // Yes, this argument is known to the application.
            return true;
        }
    }

    return false;
}

std::optional<bool> InexorCommandLineArgumentParser::is_command_line_argument_specified(const std::string argument_name) {
    if (!does_command_line_argument_template_exist(argument_name)) {
        return std::nullopt;
    }

    std::unordered_map<std::string, InexorCommandLineArgumentValue>::const_iterator argument_specified = parsed_command_line_arguments.find(argument_name);

    if (argument_specified == parsed_command_line_arguments.end()) {
        return false;
    } else {
        return true;
    }

    return std::nullopt;
}

const std::optional<INEXOR_COMMAND_LINE_ARGUMENT_TYPE> InexorCommandLineArgumentParser::get_argument_template_type(const std::string &argument_name) {
    if (does_command_line_argument_template_exist(argument_name)) {
        for (const auto &argument_template : list_of_accepted_command_line_arguments) {
            if (0 == argument_template.argument_name.compare(argument_name)) {
                return argument_template.argument_type;
            }
        }
    }

    return std::nullopt;
}

void InexorCommandLineArgumentParser::parse_command_line_arguments(std::size_t argument_count, char *arguments[]) {
    // Start the loop with 1, since index 0 is the path to the application itself + the applications name.
    for (std::size_t i = 0; i < argument_count; i++) {
        std::string argument_name = arguments[i];

        // Check if the argument specified is even wated by the application.
        if (does_command_line_argument_template_exist(argument_name)) {
            InexorCommandLineArgumentValue new_parsed_value;

            auto command_line_type = get_argument_template_type(argument_name).value();

            if (INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE == command_line_type) {
                // No follow-up argument required.
                new_parsed_value.type = INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE;
            } else {
                // Check if the next argument can be accepted as index for the value.
                if ((i + 1) < argument_count) {
                    std::string argument_value = arguments[1 + i];

                    // Yes, this is an argument that the application supports.
                    // Now let's try to parse the argument.
                    switch (command_line_type) {
                    case INEXOR_COMMAND_LINE_ARGUMENT_TYPE_STRING: {
                        new_parsed_value.type = INEXOR_COMMAND_LINE_ARGUMENT_TYPE_STRING;
                        new_parsed_value.value_str = std::string(argument_value);
                        break;
                    }
                    case INEXOR_COMMAND_LINE_ARGUMENT_TYPE_UINT32: {
                        new_parsed_value.type = INEXOR_COMMAND_LINE_ARGUMENT_TYPE_UINT32;
                        new_parsed_value.value_uint32 = static_cast<uint32_t>(std::stoi(argument_value));
                        break;
                    }
                    case INEXOR_COMMAND_LINE_ARGUMENT_TYPE_INT64: {
                        new_parsed_value.type = INEXOR_COMMAND_LINE_ARGUMENT_TYPE_INT64;
                        new_parsed_value.value_int64 = static_cast<std::int64_t>(std::stoi(argument_value));
                        break;
                    }
                    case INEXOR_COMMAND_LINE_ARGUMENT_TYPE_BOOL: {
                        new_parsed_value.type = INEXOR_COMMAND_LINE_ARGUMENT_TYPE_BOOL;

                        if (std::stoi(argument_value) > 0) {
                            new_parsed_value.value_bool = true;
                        } else {
                            new_parsed_value.value_bool = false;
                        }

                        break;
                    }
                    }
                } else {
                    spdlog::error("Argument {} is accepted but no value specified!", arguments[i]);
                }
            }

            // Add the new parsed command line argument to the buffer.
            parsed_command_line_arguments[argument_name] = new_parsed_value;

            number_of_parsed_command_line_arguments++;

            // This was the parameter value for the argument, therefore move on!
            if (INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE != new_parsed_value.type) {
                i++;
            }
        } else {
            if (i > 0) {
                // The user specified a command line argument which is not known to the application!
                spdlog::warn("Warning: Unknown command line argument {}", arguments[i]);
            }
        }
    }
}

const std::int64_t InexorCommandLineArgumentParser::get_number_of_parsed_command_line_arguments() { return number_of_parsed_command_line_arguments; }

const std::optional<bool> InexorCommandLineArgumentParser::get_command_line_argument_bool(const std::string &argument_name) {
    if (does_command_line_argument_template_exist(argument_name)) {
        if (is_command_line_argument_specified(argument_name)) {
            auto return_value = parsed_command_line_arguments[argument_name];
            if (INEXOR_COMMAND_LINE_ARGUMENT_TYPE_BOOL == return_value.type) {
                return return_value.value_bool;
            }
        }
    }

    return std::nullopt;
}

const std::optional<std::string> InexorCommandLineArgumentParser::get_command_line_argument_string(const std::string &argument_name) {
    if (does_command_line_argument_template_exist(argument_name)) {
        if (is_command_line_argument_specified(argument_name)) {
            auto return_value = parsed_command_line_arguments[argument_name];
            if (INEXOR_COMMAND_LINE_ARGUMENT_TYPE_STRING == return_value.type) {
                return return_value.value_str;
            }
        }
    }

    return std::nullopt;
}

const std::optional<std::int64_t> InexorCommandLineArgumentParser::get_command_line_argument_int64(const std::string &argument_name) {
    if (does_command_line_argument_template_exist(argument_name)) {
        if (is_command_line_argument_specified(argument_name)) {
            auto return_value = parsed_command_line_arguments[argument_name];
            if (INEXOR_COMMAND_LINE_ARGUMENT_TYPE_INT64 == return_value.type) {
                return return_value.value_int64;
            }
        }
    }

    return std::nullopt;
}

const std::optional<std::uint32_t> InexorCommandLineArgumentParser::get_command_line_argument_uint32_t(const std::string &argument_name) {
    if (does_command_line_argument_template_exist(argument_name)) {
        if (is_command_line_argument_specified(argument_name)) {
            auto return_value = parsed_command_line_arguments[argument_name];
            if (INEXOR_COMMAND_LINE_ARGUMENT_TYPE_UINT32 == return_value.type) {
                return return_value.value_uint32;
            }
        }
    }

    return std::nullopt;
}

} // namespace inexor::vulkan_renderer::tools
