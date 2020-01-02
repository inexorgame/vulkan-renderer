#include "VulkanShader.hpp"


namespace inexor {
namespace vulkan_renderer {


	void VulkanShader::load_file(const std::string& file_name)
	{
		// Open file stream to the shader file at the end of the file to read file size.
		std::ifstream shader_file(file_name.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

		if(shader_file.is_open())
		{
			// Read the size of the file in bytes.
			file_size = shader_file.tellg();

			// Preallocate memory for file buffer.
			file_data.resize(file_size);

			// Reset the file read position to the beginning of the file.
			shader_file.seekg(0, std::ios::beg);

			// Read the file data.
			shader_file.read(file_data.data(), file_size);

			// Close the file stream.
			shader_file.close();
		}
		else
		{
			cout << "Could not open shader!" << endl;
		}
	}


	VulkanShader::VulkanShader()
	{
		file_size = 0;
		file_data.clear();
	}

	
	VulkanShader::~VulkanShader()
	{
	}


};
};
