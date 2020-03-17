#include "InexorMeshLoader.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


namespace inexor {
namespace vulkan_renderer {


	InexorMeshLoader::InexorMeshLoader()
	{
	}
			

	InexorMeshLoader::~InexorMeshLoader()
	{
	}
	
	
	VkResult InexorMeshLoader::load_model_from_obj_file(const std::string& OBJ_file_name, std::vector<InexorVertex>& vertices)
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn, err;

		// Load the OBJ file using tinyobj library.
		if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, OBJ_file_name.c_str(), NULL, true))
		{
			spdlog::error("Failed to load OBJ file {}!", OBJ_file_name);

			return VK_ERROR_INITIALIZATION_FAILED;
		}
		
		if(!err.empty())
		{
			spdlog::error(err);
		}


        //std::unordered_map<InexorVertex, uint32_t> uniqueVertices;

		for(const auto& shape : shapes)
		{
			for(const auto& index : shape.mesh.indices)
			{
				InexorVertex vertex = {};
				
				vertex.pos =
				{
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

				vertex.texture_coordinates =
				{
                    attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                vertex.color = {1.0f, 1.0f, 1.0f};
				
				/*
				if(0 == uniqueVertices.count(vertex))
				{
                    uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                }
				*/

                vertices.push_back(vertex);
                //indices.push_back(uniqueVertices[vertex]);
			}
		}

		return VK_SUCCESS;
	}


};
};
