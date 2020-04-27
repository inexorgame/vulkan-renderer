#pragma once

#include <spdlog/spdlog.h>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace inexor::vulkan_renderer::tools {

/// @brief Defines the type of an accepted command line argument.
enum class CommandLineArgumentType {
    NONE,
    STRING,
    INT64,
    UINT32,
    BOOL,
    // TODO: Add more
};

/// @brief A command line argument template class.
/// @noe This class does not contain any data yet!
struct CommandLineArgumentTemplate {
    CommandLineArgumentTemplate(const CommandLineArgumentType param_type, std::string param_name) {
        argument_type = param_type;
        argument_name = param_name;
    }

    std::string argument_name;

    CommandLineArgumentType argument_type;
};

/// @brief Contains the actual command line data.
struct CommandLineArgumentValue {
    CommandLineArgumentType type;

    std::string value_str;
    std::int64_t value_int64;
    std::uint32_t value_uint32;
    bool value_bool;
};

/// @brief A simple command line argument parser.
/// @todo What if an argumen gets specified twice?
class CommandLineArgumentParser {
public:
    CommandLineArgumentParser() = default;

    ~CommandLineArgumentParser() = default;

private:
    /// @brief This defines the list of acceptable command line arguments with their corresponding types.
    const std::vector<CommandLineArgumentTemplate> list_of_accepted_command_line_arguments{
        /// Defines which GPU to use (by array index).
        {CommandLineArgumentType::UINT32, "-gpu"},

        /// Defines if we will print stats about graphics cards.
        {CommandLineArgumentType::NONE, "-nostats"},

        // Use vertical synchronisation.
        {CommandLineArgumentType::NONE, "-vsync"},

        // Use RenderDoc layer.
        {CommandLineArgumentType::NONE, "-renderdoc"},

        // Disable Khronos validation layer.
        {CommandLineArgumentType::NONE, "-novalidation"},

        // Do not use distinct data transfer queue, use graphics queue.
        {CommandLineArgumentType::NONE, "-no_separate_data_queue"},

        // Disable debug markers (even if -renderdoc is specified)
        {CommandLineArgumentType::NONE, "-no_vk_debug_markers"}

        /// TODO: Add more command line argumetns here!
    };

    /// The parsed arguments.
    std::unordered_map<std::string, CommandLineArgumentValue> parsed_command_line_arguments;

    /// The number of command line arguments.
    std::int64_t number_of_parsed_command_line_arguments = 0;

protected:
    /// @brief Checks if a command line argument
    bool does_command_line_argument_template_exist(const std::string argument_name);

    // @brief Checks if the command line argument is specified.
    std::optional<bool> is_command_line_argument_specified(const std::string argument_name);

public:
    /// @brief Parses the command line arguments
    void parse_command_line_arguments(std::size_t argument_count, char *arguments[]);

    /// @brief Returns the number of command line arguments.
    const std::int64_t get_number_of_parsed_command_line_arguments();

    /// @brief Returns the type of a command line argument.
    /// @param argument_name The name of the command line argument.
    const std::optional<CommandLineArgumentType> get_argument_template_type(const std::string &argument_name);

    /// @brief Returns the value of a boolean command line argument (if existent).
    /// @param argument_name The name of the command line argument.
    /// @return The value of the boolean command line argument
    /// (true or false, in case it even exists), std::nullopt otherwise.
    const std::optional<bool> get_command_line_argument_bool(const std::string &argument_name);

    /// @brief Returns the value of a std::string command line argument (if existent).
    /// @param argument_name The name of the command line argument.
    /// @return The std::string value of the command line argument.
    const std::optional<std::string> get_command_line_argument_string(const std::string &argument_name);

    /// @brief Returns the value of a std::int64_t command line argument (if existent).
    /// @parm argument_name The name of the command line argument.
    /// @return The std::int64_t value of the command line argument.
    const std::optional<std::int64_t> get_command_line_argument_int64(const std::string &argument_name);

    /// @brief Returns the value of a std::uint32_t command line argument (if existent).
    /// @parm argument_name The name of the command line argument.
    /// @return The std::uint32_t value of the command line argument.
    const std::optional<std::uint32_t> get_command_line_argument_uint32(const std::string &argument_name);
};

} // namespace inexor::vulkan_renderer::tools
