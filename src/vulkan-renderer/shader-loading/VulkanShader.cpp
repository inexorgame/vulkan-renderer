#include "VulkanShader.hpp"


namespace inexor {
namespace vulkan_renderer {


	VulkanShader::VulkanShader()
	{
		file_size = 0;
		file_data.clear();
	}

	
	VulkanShader::~VulkanShader()
	{
	}


	const std::size_t VulkanShader::get_file_size() const
	{
		return file_size;
	}

	
	const std::vector<char>& VulkanShader::get_file_data() const
	{
		return file_data;
	}


	void VulkanShader::load_file(const std::string& file_name)
	{
		// Open stream at the end of the file to read it's size.
		std::ifstream shader_file(file_name.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

		if(shader_file.is_open())
		{
			cout << "File " << file_name.c_str() << " has been opened." << endl;

			// Read the size of the file.
			file_size = shader_file.tellg();

			// Preallocate memory for the file buffer.
			file_data.resize(file_size);

			// Reset the file read position to the beginning of the file.
			shader_file.seekg(0, std::ios::beg);

			// Read the file data.
			shader_file.read(file_data.data(), file_size);

			// Close the file stream.
			shader_file.close();

			cout << "File " << file_name.c_str() << " has been closed." << endl;
		}
		else
		{
			cout << "Could not open shader!" << endl;
		}
	}


};
};
