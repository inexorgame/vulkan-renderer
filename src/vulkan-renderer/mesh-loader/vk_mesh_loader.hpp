#pragma once

#include <vulkan/vulkan.h>

#include "../vertex/vk_vertex.hpp"

#include <vector>


namespace inexor {
namespace vulkan_renderer {

	
	/// 
	class InexorMeshLoader
	{
		public:

			InexorMeshLoader();
			
			~InexorMeshLoader();


			/// 
			VkResult load_model_from_glTF_file(const std::string& OBJ_file_name, std::vector<InexorVertex>& vertices/*, std::vector<uint32_t> indices*/);

	};


};
};
