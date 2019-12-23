#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
using namespace std;


namespace inexor {
namespace vulkan_renderer {

	// 
	class VulkanShader
	{

		public:
			
			// 
			VulkanShader();

			// 
			~VulkanShader();

			// 
			void load_file(const std::string& file_name);
			
			// 
			std::vector<char> file_data;

			// 
			std::size_t file_size;

	};

};
};
