#include "fps_counter.hpp"


namespace inexor
{
	namespace vulkan_renderer
	{


		std::optional<uint32_t> InexorFPSCounter::update()
		{
			auto current_time = std::chrono::high_resolution_clock::now();

			auto time_duration = std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count();

			if(time_duration >= fps_update_interval)
			{
				uint32_t fps_value = static_cast<uint32_t>(frames / time_duration);

				last_time = current_time;
				frames = 0;

				return fps_value;
			}

			frames++;

			return std::nullopt;
		}


	};
};
