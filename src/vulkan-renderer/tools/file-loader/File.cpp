#include "File.hpp"
using namespace std;


namespace inexor {
namespace vulkan_renderer {
namespace tools {

	
	InexorFile::InexorFile()
	{
	}
	

	InexorFile::~InexorFile()
	{
	}


	const std::size_t InexorFile::get_file_size() const
	{
		return file_size;
	}

	
	const std::vector<char>& InexorFile::get_file_data() const
	{
		return file_data;
	}


	bool InexorFile::load_file(const std::string& file_name)
	{
		assert(file_name.size() > 0);
		
		// Open stream at the end of the file to read it's size.
		std::ifstream file_to_load(file_name.c_str(), std::ios::in|std::ios::binary|std::ios::ate);

		if(file_to_load.is_open())
		{
			cout << "File " << file_name.c_str() << " has been opened." << endl;

			// Read the size of the file.
			file_size = file_to_load.tellg();

			// Preallocate memory for the file buffer.
			file_data.resize(file_size);

			// Reset the file read position to the beginning of the file.
			file_to_load.seekg(0, std::ios::beg);

			// Read the file data.
			file_to_load.read(file_data.data(), file_size);

			// Close the file stream.
			file_to_load.close();

			cout << "File " << file_name.c_str() << " has been closed." << endl;

			return true;
		}
		else
		{
			cout << "Could not open shader!" << endl;
			return false;
		}

		return true;
	}


};
};
};
