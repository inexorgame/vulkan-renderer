*********
Changelog
*********

All notable changes to this project will be documented in this file.

The format is based on `Keep a Changelog <https://keepachangelog.com/en/1.0.0/>`_ and this project adheres to `Semantic Versioning <https://semver.org/spec/v2.0.0.html>`_.


v0.1-alpha.3
===================

Changed
-------

- RAII in shader code
- RAII in shaders, gpu memory buffers, staging buffers, mesh buffers and textures
- RAII in descriptors
- RAII VkInstance
- RAII Swapchain


v0.1-alpha.2
===================

Added
-----

- Create a threadpool using C++17.
- Added a simple C++17 implementation of an octree.
- Added event system using boost::signals2.
- Use boost::bitstream for data processing.
- Convert octree data structure to vertex geometry (a mesh buffer).
- Support arbitrary indentations of octree geometry.
- Added a descriptor set layout for simple octree geometry.
- Ported Vulkan Memory Allocator library (VMA) to Linux.
- Added a simple camera movement class.
- Write spdlog console output to a logfile.

Changed
-------

- Fixed a bug that would render every model twice.
- Improvements considering C++ code quality standards.
- Logging format and logger usage.

v0.1-alpha.1
=====

Added
-----

- Create a CMake file with conan package manager setup.
- Integrate Vulkan Memory Allocator (VMA) library.
- Integrate RenderDoc support.
- Use spdlog as logger library.
- Integrate tiny_gltf library.
- Mesh buffer manager for vertex and index buffers based on VMA.
- Texture manager based on stb_image and VMA.
- Uniform buffer manager based on VMA.
- Shader manager for loading SPIR-V shaders.
- Load TOML configuration files using toml11. We deliberately won't use JSON for this.
- Vulkan fence manager.
- Vulkan semaphore manager.
- GPU info query functions.
- Vulkan debug callbacks.
- Vulkan standard validation layers.
- C++11 std::chrono class.
- Use glm.
- Depth buffer.
- Let VMA generate memory debug logs.
- Associate internal resource names with memory regions to improve debugging.
- Use separate data transfer queue for cpu to gpu memory copies if available.
- Availability checks for Vulkan features.
- Settings decision maker for Vulkan initialisation.
- Simple command line argument parser.
- Automatic GPU selection mechanism and -gpu <N> command line argument for preferential GPU.
- Create windows using glfw3.
- Keyboard input based on glfw3.
- Load geometry of glTF 2.0 files using tiny_gltf library.
- Basic camera class.
