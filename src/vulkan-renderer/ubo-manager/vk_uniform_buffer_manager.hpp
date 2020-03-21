#pragma once

#define GLM_FORCE_RADIANS

// This will force GLM to use a version of vec2 and mat4 that has the alignment requirements already specified for us.
/// @warning Unfortunately this method can break down if you start using nested structures.
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/glm.hpp>


namespace inexor {
namespace vulkan_renderer {

	
	/// @class UniformBufferObject
	/// @note We can exactly match the definition in the shader using data types in GLM.
	/// The data in the matrices is binary compatible with the way the shader expects
	/// it, so we can later just memcpy a UniformBufferObject to a VkBuffer.
	struct UniformBufferObject
	{
		glm::mat4 model;
		glm::mat4 view;
		glm::mat4 proj;
	};


};
};
