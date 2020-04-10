#pragma once

#include <chrono>


namespace inexor {
namespace vulkan_renderer {


	/// @class InexorTimeStep
	/// @brief Responsible for calculating the amount of time which has passed between rendering two frames.
	/// Since every machine has slightly different speed, it is neccesary to the timestep when animating something.
	/// @todo Implement time step for every thread?
	class InexorTimeStep
	{
		private:

			// The time point of the last render call.
			std::chrono::time_point<std::chrono::steady_clock> last_time;

			// The time point of program start.
			std::chrono::time_point<std::chrono::steady_clock> program_start_time;

		public:

			InexorTimeStep();

			~InexorTimeStep() = default;


			/// @brief Returns a scaling factor which corresponds to the
			/// time which has passed since last render call and now.
			float get_time_step();


			/// @brief Returns a scaling factor which corresponds to the
			/// time which has passed since program start and now.
			float get_program_start_time_step();


	};


};
};
