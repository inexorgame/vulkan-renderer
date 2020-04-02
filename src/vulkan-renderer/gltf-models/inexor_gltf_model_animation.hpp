#pragma once

#include "inexor_gltf_animation_channel.hpp"
#include "inexor_gltf_animation_sampler.hpp"

#include <string>
#include <vector>


namespace inexor {
namespace vulkan_renderer {
namespace gltf2 {


	/// 
	struct InexorModelAnimation
	{
		std::string name;
		
		std::vector<InexorModelAnimationSampler> samplers;
		
		std::vector<InexorModelAnimationChannel> channels;
		
		float start = std::numeric_limits<float>::max();
		
		float end = std::numeric_limits<float>::min();
	};


};
};
};
