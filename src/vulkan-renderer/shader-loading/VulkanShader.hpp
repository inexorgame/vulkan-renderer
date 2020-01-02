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
			std::vector<char> file_data;

			// 
			std::size_t file_size;

			// 
			void load_file(const std::string& file_name);

	};

};
};
