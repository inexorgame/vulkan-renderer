#pragma once

#include <glm/glm.hpp>

// Changing this value here also requires changing it in the vertex shader.
#define MAX_NUM_JOINTS 128u


namespace inexor {
namespace vulkan_renderer{


	/// @class InexorModelStandardUniformBuffer
	/// @brief Inexor's standard uniform buffer block for glTF 2.0 models.
	struct InexorModelStandardUniformBufferBlock
	{
		glm::mat4 matrix;
		
		glm::mat4 joint_matrix[MAX_NUM_JOINTS]{};
		
		float joint_count{0};
	};


};
};
