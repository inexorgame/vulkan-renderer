#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "availability_checks.hpp"


TEST(availability_checks, instance_layer_available)
{
	using namespace inexor::vulkan_renderer::availability_checks;

	// Check if standard validation layer is available.
	bool retval = is_instance_layer_available("VK_LAYER_LUNARG_standard_validation");

	EXPECT_EQ(retval, true);
}
