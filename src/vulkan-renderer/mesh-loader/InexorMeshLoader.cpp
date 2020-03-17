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
	
	
	VkResult InexorMeshLoader::load_model_from_glTF_file(const std::string& glTF_file_name, std::vector<InexorVertex>& vertices/*, std::vector<uint32_t> indices*/)
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

		for(auto& mesh: model.meshes)
		{
			for(auto& primitive : mesh.primitives)
			{
				const tinygltf::Accessor& accessor = model.accessors[primitive.attributes["POSITION"]];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];

				// cast to float type read only. Use accessor and bufview byte offsets to determine where position data 
				// is located in the buffer.
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				// bufferView byteoffset + accessor byteoffset tells you where the actual position data is within the buffer. From there
				// you should already know how the data needs to be interpreted.
				const float* positions = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
		
				// From here, you choose what you wish to do with this position data. In this case, we  will display it out.
				for(size_t i = 0; i < accessor.count; ++i)
				{
					InexorVertex vertex;

					vertex.pos.x = positions[i * 3 + 0];
					vertex.pos.y = positions[i * 3 + 1];
					vertex.pos.z = positions[i * 3 + 2];

					vertex.color.r = 255.0f;
					vertex.color.g = 0.0f;
					vertex.color.b = 0.0f;

					vertex.texture_coordinates.x = 0.5f;
					vertex.texture_coordinates.y = 0.5f;

					vertices.push_back(vertex);
				}
			}
		}

		return VK_SUCCESS;
	}


};
};
