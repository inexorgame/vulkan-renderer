#pragma once

#include <vulkan/vulkan.h>

#include "../vertex-structure/InexorVertex.hpp"

#include <spdlog/spdlog.h>


namespace inexor {
namespace vulkan_renderer {

	
	/// 
	class InexorMeshLoader
	{
		public:

			InexorMeshLoader();
			
			~InexorMeshLoader();


			/// 
			VkResult load_model_from_obj_file(const std::string& OBJ_file_name, std::vector<InexorVertex>& vertices);

	};


};
};
