set(APP_MANUFACTURER_NAME "Inexor collective")
set(APP_DESCRIPTION "Inexor vulkan-renderer")
set(APP_NAME "Inexor engine")
set(APP_LICENSE "MIT License")
set(APP_ORIGINAL_NAME "Inexor vulkan renderer example")
set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 0)
set(APP_VERSION_PATCH 0)

# Replace variables in resource file with CMake values
configure_file(
   win_resource_file.rc.in ${CMAKE_BINARY_DIR}/include/inexor/vulkan-renderer/win_resource_file.rc
)

configure_file(
   resource.hpp.in ${CMAKE_BINARY_DIR}/include/inexor/vulkan-renderer/resource.hpp
)
