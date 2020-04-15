#pragma once

#include "vulkan-renderer/gltf-model-manager/gltf_model_node.hpp"

#include <memory>


namespace inexor {
namespace vulkan_renderer {


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
