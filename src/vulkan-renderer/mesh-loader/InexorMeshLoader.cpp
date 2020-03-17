#include "InexorMeshLoader.hpp"

#include <spdlog/spdlog.h>

// JSON for modern C++11 library.
// https://github.com/nlohmann/json
#include "nlohmann/json.hpp"

// tinygltf: header only C++ tiny glTF library(loader/saver).
// https://github.com/syoyo/tinygltf
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

// We include JSON library manually.
#define TINYGLTF_NO_INCLUDE_JSON

#include "../../third_party/tiny_gltf/tiny_gltf.h"


namespace inexor {
namespace vulkan_renderer {


	InexorMeshLoader::InexorMeshLoader()
	{
	}
			

	InexorMeshLoader::~InexorMeshLoader()
	{
	}
	
	
	VkResult InexorMeshLoader::load_model_from_glTF_file(const std::string& glTF_file_name, std::vector<InexorVertex>& vertices, std::vector<uint32_t> indices)
	{
		using namespace tinygltf;

		Model model;
		TinyGLTF loader;
		std::string errors;
		std::string warnings;

		bool result = loader.LoadASCIIFromFile(&model, &errors, &warnings, glTF_file_name);

		if(!warnings.empty())
		{
			spdlog::warn(warnings);
		}

		if(!errors.empty())
		{
			spdlog::error(errors);
		}

		if(!result)
		{
			spdlog::error("Failed to load glTF file: {}", glTF_file_name);
			return VK_ERROR_INITIALIZATION_FAILED;
		}

		return VK_SUCCESS;
	}


};
};
