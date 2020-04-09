#pragma once

#include <chrono>
#include <optional>


namespace inexor {
namespace vulkan_renderer {

	
	class InexorFPSCounter
	{
		private:

			std::size_t frames = 0;
			
			std::chrono::time_point<std::chrono::steady_clock> last_time;

			float fps_update_interval = 1.0f;


		public:

			InexorFPSCounter() = default;
			
			~InexorFPSCounter() = default;

			std::optional<uint32_t> update();


	};

};
};
