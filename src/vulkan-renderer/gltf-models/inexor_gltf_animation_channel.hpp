#pragma once

#include "inexor_gltf_model_node.hpp"

#include <memory>


namespace inexor {
namespace vulkan_renderer {
namespace glTF2_models {


	/// 
	struct InexorModelAnimationChannel
	{
		enum PathType
		{
			TRANSLATION,
			ROTATION,
			SCALE
		};
		
		PathType path;
		
		std::shared_ptr<InexorModelNode> node;

		uint32_t samplerIndex;
	};


};
};
};
