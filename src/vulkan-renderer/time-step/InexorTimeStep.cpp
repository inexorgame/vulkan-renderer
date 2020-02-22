#include "InexorTimeStep.hpp"


namespace inexor {
namespace vulkan_renderer {

	
	InexorTimeStep::InexorTimeStep()
	{
		program_start_time = std::chrono::high_resolution_clock::now();
		last_time = std::chrono::high_resolution_clock::now();
	}


	InexorTimeStep::~InexorTimeStep()
	{
	}


	float InexorTimeStep::get_time_step()
	{
		auto current_time = std::chrono::high_resolution_clock::now();
		auto time_duration = std::chrono::duration<float, std::chrono::seconds::period>(current_time - last_time).count();
		last_time = current_time;
		return time_duration;
	}


	float InexorTimeStep::get_program_start_time_step()
	{
		auto current_time = std::chrono::high_resolution_clock::now();
		auto time_duration = std::chrono::duration<float, std::chrono::seconds::period>(current_time - program_start_time).count();
		return time_duration;
	}

};
};
