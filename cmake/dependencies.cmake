# Download dependencies by using FetchContent_Declare
# Use FetchContent_MakeAvailable only in those code parts where the dependency is actually needed
# Please keep the dependencies in alphabetical order

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Benchmarking library for inexor-vulkan-renderer-benchmarks
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable Google Benchmark tests" FORCE)
FetchContent_Declare(benchmark SYSTEM
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG v1.8.5
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 1.8.5
)
    
# Command line argument parsing library
FetchContent_Declare(CLI11 SYSTEM
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.6.1
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 2.6.1
)

# Formatting library
FetchContent_Declare(fmt SYSTEM
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 11.0.1
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 11.0.1
)

# Window management for Linux and Microsoft Windows
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(glfw SYSTEM
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 3.4
)

# Mathematics library for computer graphics
FetchContent_Declare(glm SYSTEM
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW ON
    GIT_PROGRESS ON
)

# Unit testing library for inexor-vulkan-renderer-tests
FetchContent_Declare(gtest SYSTEM
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG v1.15.0
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 1.15.0
)

# Graphical user interface rendering library
FetchContent_Declare(imgui SYSTEM
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.90.9
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 1.90.9
)

# Console logging and logfile library
set(SPDLOG_USE_STD_FORMAT ON CACHE BOOL "" FORCE)
set(SPDLOG_FMT_EXTERNAL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(spdlog SYSTEM
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 1.15.3
)

# Required for loading textures in various formats
FetchContent_Declare(stb SYSTEM
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG f7f20f39fe4f206c6f19e26ebfef7b261ee59ee4
    # Do not enable shallow download on stb because it will cause a CMake error!
    GIT_PROGRESS ON
)

# Loader for models in glTF2 format
FetchContent_Declare(tinygltf SYSTEM
    GIT_REPOSITORY https://github.com/syoyo/tinygltf.git
    GIT_TAG v2.9.2
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 2.9.2
)

# Library for TOML configuration files
FetchContent_Declare(tomlplusplus SYSTEM
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 3.4.0
)

# Vulkan Memory Allocator library
FetchContent_Declare(vma SYSTEM
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
    GIT_TAG v3.1.0
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 3.1.0
)

# Meta-loader for the Vulkan API that dynamically loads function pointers at runtime, avoiding the need for static linking
FetchContent_Declare(volk SYSTEM
    GIT_REPOSITORY https://github.com/zeux/volk
    GIT_TAG 1.3.270
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    # Do not specify FIND_PACKAGE_ARGS here because this will fail Linux build!
    # What happens is that CMake will try to find volk::volk in Vulkan SDK instead of using the repository.
)

# The Vulkan API headers
FetchContent_Declare(Vulkan SYSTEM
    GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers
    GIT_TAG v1.3.283
    GIT_SHALLOW ON
    GIT_PROGRESS ON
    FIND_PACKAGE_ARGS 1.3.283
)
