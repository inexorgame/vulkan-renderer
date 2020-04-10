#pragma once

#include <vector>
#include <string>
#include <fstream>

#include <spdlog/spdlog.h>

#include <cassert>


namespace inexor {
namespace vulkan_renderer {
namespace tools {


	/// @class InexorFile
	/// @brief A class for loading files into memory.
	class InexorFile
	{
		private:
			
			/// The file data.
			std::vector<char> file_data;

			/// The size of the file.
			std::size_t file_size;

		public:
			
			InexorFile() = default;

			~InexorFile() = default;


			/// @brief Returns the size of the file.
			const std::size_t get_file_size() const;

			/// @brief Returns the file's data.
			const std::vector<char>& get_file_data() const;

			/// @brief Loads the entire file into memory.
			/// @param file_name The name of the file.
			/// @return True if file was loaded successfully, false otherwise.
			bool load_file(const std::string& file_name);

	};



};
};
};
