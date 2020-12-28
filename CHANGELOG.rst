*********
Changelog
*********

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog <https://keepachangelog.com/en/1.0.0/>`__ and this project adheres to `Semantic Versioning <https://semver.org/spec/v2.0.0.html>`__.

You can find all releases in our `GitHub repository <https://github.com/inexorgame>`__.

v0.1-alpha.3
============

Changed
-------

- RAII in shader code
- RAII in shaders, gpu memory buffers, staging buffers, mesh buffers and textures
- RAII in descriptors
- RAII VkInstance
- RAII Swapchain
- Removed manager classes entirely.

v0.1-alpha.2
============

Added
-----

- Create a `threadpool <https://codereview.stackexchange.com/questions/221626/c17-thread-pool>`__ using C++17.
- Added a simple C++17 implementation of an octree.
- Added event system using `boost::signals2 <https://www.boost.org/doc/libs/1_61_0/doc/html/signals2.html>`__.
- Use `boost::bitstream <https://www.boost.org/doc/libs/master/boost/beast/zlib/detail/bitstream.hpp>`__ for data processing.
- Convert octree data structure to vertex geometry (a mesh buffer).
- Support arbitrary indentations of octree geometry.
- Added a descriptor set layout for simple octree geometry.
- Ported `Vulkan Memory Allocator library (VMA) <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__ to Linux.
- Added a simple camera movement class.
- Write `spdlog <https://github.com/gabime/spdlog>`__ console output to a logfile.

Changed
-------

- Improvements considering C++ code quality standards.
- Logging format and logger usage.

Fixed
-----

- Fixed a bug that would render every model twice.

v0.1-alpha.1
============

Added
-----

- Create a `CMake <https://cmake.org/>`__ file with `conan package manager <https://conan.io/center/>`__ setup.
- Integrate `Vulkan Memory Allocator library (VMA) <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__.
- Integrate `RenderDoc <https://renderdoc.org/>`__ support.
- Use `spdlog <https://github.com/gabime/spdlog>`__ as logger library.
- Integrate `tiny_gltf <https://github.com/syoyo/tinygltf>`__ library.
- Mesh buffer manager for vertex and index buffers based on VMA.
- Texture manager based on stb_image and VMA.
- Uniform buffer manager based on VMA.
- Shader manager for loading SPIR-V shaders.
- Load TOML configuration files using `toml11 <https://github.com/ToruNiina/toml11>`__. We deliberately won't use JSON for this.
- Vulkan fence manager.
- Vulkan semaphore manager.
- GPU info query functions.
- Vulkan debug callbacks.
- Vulkan standard validation layers.
- C++11 `std::chrono <https://en.cppreference.com/w/cpp/chrono>`__ class.
- Use `glm <https://github.com/g-truc/glm>`__.
- Depth buffer.
- Let VMA generate memory debug logs.
- Associate internal resource names with memory regions to improve debugging.
- Use separate data transfer queue for cpu to gpu memory copies if available.
- Availability checks for Vulkan features.
- Settings decision maker for Vulkan initialisation.
- Simple command line argument parser.
- Automatic GPU selection mechanism and -gpu <N> command line argument for preferential GPU.
- Create windows using `glfw3 <https://www.glfw.org/>`__.
- Keyboard input based on glfw3.
- Load geometry of glTF 2.0 files using tiny_gltf library.
- Basic camera class.
