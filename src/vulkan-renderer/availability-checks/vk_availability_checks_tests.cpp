#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include "vk_availability_checks.hpp"


TEST(availability_checks, instance_layer_available)
{
	using namespace inexor::vulkan_renderer::availability_checks;

	// Check if standard validation layer is available.
	bool retval = is_instance_layer_available("VK_LAYER_LUNARG_standard_validation");

	EXPECT_EQ(retval, true);
}


TEST(availability_checks, instance_extension_available)
{
	using namespace inexor::vulkan_renderer::availability_checks;

	bool retval[3];

	retval[0] = is_instance_extension_available("VK_EXT_debug_utils");
	retval[1] = is_instance_extension_available("VK_EXT_debug_marker");
	retval[2] = is_instance_extension_available("VK_EXT_debug_report");

	// The test succeeded if one of these extensions could be found.
	// If none could be found, there might be something wrong with the method!
	bool result = retval[0] || retval[1] || retval[2];

	EXPECT_EQ(result, true);
}
