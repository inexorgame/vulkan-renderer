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
		public:
			
			VulkanShader();

			~VulkanShader();
			
			// The file data.
			std::vector<char> file_data;

			// The size of the file.
			std::size_t file_size;

			// Loads the entire file into memory.
			// @param file_name The name of the file.
			void load_file(const std::string& file_name);

	};

};
};
