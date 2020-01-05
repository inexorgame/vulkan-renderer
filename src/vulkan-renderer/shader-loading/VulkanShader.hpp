#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;


namespace inexor {
namespace vulkan_renderer {


	// A SPIR-V Vulkan shader.
	class VulkanShader
	{
		private:
			
			// The file data.
			std::vector<char> file_data;

			// The size of the file.
			std::size_t file_size;

		public:
			
			VulkanShader();

			~VulkanShader();


			// Returns the size of the file.
			const std::size_t get_file_size() const;

			// Returns the file's data.
			const std::vector<char>& get_file_data() const;

			// Loads the entire file into memory.
			// @param file_name The name of the file.
			void load_file(const std::string& file_name);

	};

};
};
