#pragma once

#include <glm/glm.hpp>


namespace inexor {
namespace vulkan_renderer {
namespace glTF2_models {


	/// 
	struct InexorModelVertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv0;
		glm::vec2 uv1;
		glm::vec4 joint0;
		glm::vec4 weight0;
	};


};
};
};
