#pragma once

#include <vector>
#include <string>
#include <optional>
#include <unordered_map>

#include <spdlog/spdlog.h>


namespace inexor
{
	namespace vulkan_renderer
	{
		namespace tools
		{


			/// @brief Defines the type of an accepted command line argument.
			enum INEXOR_COMMAND_LINE_ARGUMENT_TYPE
			{
				INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE,
				INEXOR_COMMAND_LINE_ARGUMENT_TYPE_STRING,
				INEXOR_COMMAND_LINE_ARGUMENT_TYPE_INT64,
				INEXOR_COMMAND_LINE_ARGUMENT_TYPE_UINT32,
				INEXOR_COMMAND_LINE_ARGUMENT_TYPE_BOOL
				// TODO: Add more
			};


			/// @class CommandLineArgumentTemplate
			/// @brief A command line argument template class.
			/// @noe This class does not contain any data yet!
			struct InexorCommandLineArgumentTemplate
			{
				InexorCommandLineArgumentTemplate(const INEXOR_COMMAND_LINE_ARGUMENT_TYPE param_type, std::string param_name)
				{
					argument_type = param_type;
					argument_name = param_name;
				}

				std::string argument_name;

				INEXOR_COMMAND_LINE_ARGUMENT_TYPE argument_type;
			};


			/// @class CommandLineArgumentValue
			/// @brief Contains the actual command line data.
			struct InexorCommandLineArgumentValue
			{
				INEXOR_COMMAND_LINE_ARGUMENT_TYPE type;

				std::string value_str;
				std::int64_t value_int64;
				std::uint32_t value_uint32;
				bool value_bool;
			};


			/// @class CommandLineArgumentParser
			/// @brief A simple command line argument parser.
			/// @todo What if an argumen gets specified twice?
			class InexorCommandLineArgumentParser
			{
				public:

				InexorCommandLineArgumentParser();

				~InexorCommandLineArgumentParser();

				private:


				/// @brief This defines the list of acceptable command line arguments with their corresponding types.
				const std::vector<InexorCommandLineArgumentTemplate> list_of_accepted_command_line_arguments
				{
					/// Defines which GPU to use (by array index).
					{INEXOR_COMMAND_LINE_ARGUMENT_TYPE_UINT32, "-gpu"},

					/// Defines if we will print stats about graphics cards.
					{INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE, "-nostats"},

					// TODO: Implement!
					// Use vertical synchronisation.
					//{INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE, "-vsync"},

					// Use RenderDoc layer.
					{INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE, "-renderdoc"},

					// Disable Khronos validation layer.
					{INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE, "-novalidation"},

					// Do not use distinct data transfer queue, use graphics queue.
					{INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE, "-no_separate_data_queue"},

					// Disable debug markers (even if -renderdoc is specified)
					{INEXOR_COMMAND_LINE_ARGUMENT_TYPE_NONE, "-no_vk_debug_markers"}

					/// TODO: Add more command line argumetns here!
				};


				/// The parsed arguments.
				std::unordered_map<std::string, InexorCommandLineArgumentValue> parsed_command_line_arguments;


				/// The number of command line arguments.
				std::int64_t number_of_parsed_command_line_arguments = 0;


				protected:

				/// @brief Checks if a command line argument 
				bool does_command_line_argument_template_exist(const std::string argument_name);


				// @brief Checks if the command line argument is specified.
				std::optional<bool> is_command_line_argument_specified(const std::string argument_name);

				public:

				/// @brief Parses the command line arguments
				void parse_command_line_arguments(std::size_t argument_count, char* arguments[]);


				/// @brief Returns the number of command line arguments.
				const std::int64_t get_number_of_parsed_command_line_arguments();


				/// @brief Returns the type of a command line argument.
				/// @param argument_name The name of the command line argument.
				const std::optional<INEXOR_COMMAND_LINE_ARGUMENT_TYPE> get_argument_template_type(const std::string& argument_name);


				/// @brief Returns the value of a boolean command line argument (if existent).
				/// @param argument_name The name of the command line argument.
				/// @return The value of the boolean command line argument
				/// (true or false, in case it even exists), std::nullopt otherwise.
				const std::optional<bool> get_command_line_argument_bool(const std::string& argument_name);


				/// @brief Returns the value of a std::string command line argument (if existent).
				/// @param argument_name The name of the command line argument.
				/// @return The std::string value of the command line argument.
				const std::optional<std::string> get_command_line_argument_string(const std::string& argument_name);


				/// @brief Returns the value of a std::int64_t command line argument (if existent).
				/// @parm argument_name The name of the command line argument.
				/// @return The std::int64_t value of the command line argument.
				const std::optional<std::int64_t> get_command_line_argument_int64(const std::string& argument_name);


				/// @brief Returns the value of a std::uint32_t command line argument (if existent).
				/// @parm argument_name The name of the command line argument.
				/// @return The std::uint32_t value of the command line argument.
				const std::optional<std::uint32_t> get_command_line_argument_uint32_t(const std::string& argument_name);


			};


		};
	};
};
