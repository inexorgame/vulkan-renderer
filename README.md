# vulkan-renderer
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
![Issues](https://img.shields.io/github/issues/inexorgame/vulkan-renderer)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://github.com/inexorgame/vulkan-renderer)

A new rendering engine based on [Vulkan API](https://www.khronos.org/vulkan/)

## What is Vulkan?
![Vulkan API logo here..](https://upload.wikimedia.org/wikipedia/commons/thumb/3/30/Vulkan.svg/500px-Vulkan.svg.png)

Vulkan is a new, low level API ([application programming interface](https://en.wikipedia.org/wiki/Application_programming_interface)) for high-performance graphics programming and computing. It is seen by some as the successor to OpenGL, although it is important to state that is is very different from it. Vulkan is not just a new version of OpenGL or an extension of it. Both Vulkan and OpenGL are being developed by the [Khronos Group](https://www.khronos.org/). Like DirectX 12 or Apple's Metal, Vulkan is a low level API which allows for much deeper control over the graphics card and the driver. This offers better performance (higher FPS) due to reduction of overhead and driver guesswork during runtime. In general, Vulkan does a lot of work during the initialisation of the application but therefore reduces work during rendering. Since Vulkan is much more explicit, it is neccesary to write more code and to think about how to abstract it. Rendering a first triangle for example will take much more work than you are used to from OpenGL. The benefits of this low level design outweigh the additional effort though!

## Getting into Vulkan
You really should watch these expert talks on YouTube:

* [GDC 2018 - Getting explicit: How Hard is Vulkan really?](https://www.youtube.com/watch?v=0R23npUCCnw)
Dustin Land, Software engineer, id-Software.

* [DevU 2017: Getting Started with Vulkan](https://www.youtube.com/watch?v=yHZ3-AMJA6Y)
Developers from Imagination, Google and LunarG.

* [Porting your engine to Vulkan or DX12](https://www.youtube.com/watch?v=6NWfznwFnMs)
Adam Sawicki, Developer Software Engineer, AMD.

* [Vulkan Best Practices Roundtable discussion](https://www.youtube.com/watch?v=owuJRPKIUAg)
NVidia, Imagination, Qualcomm, id-Software, EPIC-games and Google.

* [Vulkan Memory Management](https://www.youtube.com/watch?v=rXSdDE7NWmA)
Jordan Logan, Developer technology engineer, AMD.

* [Vulkan Memory Managenent](https://www.youtube.com/watch?v=zSG6dPq57P8)
Steven Tovey, Developer technology engineer, AMD.

* [Vulkan: State of the Union 2019](https://www.youtube.com/watch?v=KLZsAJQBR5o)
Developers from ARM, LunarG, NVidia.


## Why use Vulkan?
* Unlike OpenGL, Vulkan fits the design of modern GPUs as it is not just one single [state machine](https://stackoverflow.com/questions/31282678/what-is-the-opengl-state-machine).
* Vulkan is a low-level API which gives much more control over GPU behaviour. This reduces driver guesswork und avoids undefined behaviour of graphics drivers.
* The API is asynchronous and encourages multithreaded rendering. This is a major advantage over OpenGL! Vulkan also wants you to use the GPU asynchronously.
* Lower and more predictable CPU load which results in [better performance](https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot) and a reduction of driver guesswork.
* The reduction of CPU workload and it's improved predictability can enforce the GPU to be the limiting factor (as it should be), instead of the CPU.
* Vulkan implies memory-management to be done by the application (by you) rather than the driver.
* Vulkan is a fresh start, whereas OpenGL contains a myriad of hacks to support very rare use cases.
* Available on a variety of platforms: Windows, Linux, mobile devices and much more!
* [Validation layers](https://github.com/KhronosGroup/Vulkan-ValidationLayers) and diagnostics can be independently activated during development, allowing better error handling and debugging compared with OpenGL or DirectX. Upon release builds, the validation layers can be deactivated easily.
* Vulkan pre-compiles shaders to a [standardised bytecode format](https://en.wikipedia.org/wiki/Standard_Portable_Intermediate_Representation). This again reduces driver guesswork during runtime.
* Vulkan API and most of the drivers are [open source](https://en.wikipedia.org/wiki/Open_source)! (unlike DirectX 12 for example)
* Vulkan has layers and extensions as part of its design. For example it's easy to put steam overlay into a game simply by enabling Valve's steam overlay layer.
* Vulkan is being developed through an [unprecedented collaboration](https://www.khronos.org/members/list) of major industry-leading companies. It is not being developed by one company only (like DirectX by Microsoft). As Vulkan's motto states, it really is `industry-forged`.
* The [ending of the OpenGL era](https://www.reddit.com/r/opengl/comments/b44tyu/apple_is_deprecating_opengl/) has begun.

## Roadmap

### Initialisation and glTF2 demo (0.1 alpha) (estimated April 12th, 2020)
* [X] Create a [CMake](https://cmake.org/) file with [conan package manager](https://conan.io/) setup.
* [X] Integrate [Vulkan Memory Allocator (VMA) library](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator).
* [X] Integrate [RenderDoc](https://renderdoc.org/) support.
* [X] Use [spdlog](https://github.com/gabime/spdlog) as logger library.
* [X] Integrate [tiny_gltf library](https://github.com/syoyo/tinygltf).
* [X] Mesh buffer manager for vertex and index buffers based on [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator).
* [X] Texture manager based on [stb_image](https://github.com/nothings/stb) and [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator).
* [X] Uniform buffer manager based on [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator).
* [X] Shader manager for loading [SPIR-V](https://en.wikipedia.org/wiki/Standard_Portable_Intermediate_Representation) shaders.
* [X] Load [TOML](https://en.wikipedia.org/wiki/TOML) configuration files using [toml11](https://github.com/ToruNiina/toml11). We deliberately [won't use JSON for this](https://www.lucidchart.com/techblog/2018/07/16/why-json-isnt-a-good-configuration-language/).
* [X] Vulkan [fence](https://vulkan.lunarg.com/doc/view/1.0.26.0/linux/vkspec.chunked/ch06s01.html) manager.
* [X] Vulkan [semaphore](https://www.khronos.org/blog/vulkan-timeline-semaphores) manager.
* [X] GPU info viewer functions.
* [X] [Vulkan debug callbacks](https://vulkan.lunarg.com/doc/view/1.0.37.0/linux/vkspec.chunked/ch32s02.html).
* [X] [Vulkan standard validation layers](https://github.com/KhronosGroup/Vulkan-ValidationLayers).
* [X] C++11 timestep class.
* [X] Use [glm](https://glm.g-truc.net/0.9.9/index.html).
* [X] Depth buffer.
* [X] Let [VMA](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) generate memory debug logs.
* [X] Associate internal resource names with memory regions to improve debugging.
* [X] Use separate data transfer queue for memory copies if available.
* [X] Availability checks for Vulkan features.
* [X] Settings decision maker for Vulkan initialisation.
* [X] Simple command line argument parser.
* [X] Automatic GPU selection mechanism and `-GPU <N>` command line argument for preferential  GPU.
* [X] Create windows using [glfw3](https://www.glfw.org/).
* [X] Keyboard input based using [glfw3](https://www.glfw.org/).
* [ ] Mouse input based using [glfw3](https://www.glfw.org/).
* [X] Load geometry of [glTF 2.0 files](https://www.khronos.org/gltf/) using [tiny_gltf library](https://github.com/syoyo/tinygltf).
* [X] Load animations of [glTF 2.0 files](https://www.khronos.org/gltf/) using [tiny_gltf library](https://github.com/syoyo/tinygltf).
* [ ] Load textures of [glTF 2.0 files](https://www.khronos.org/gltf/) using [tiny_gltf library](https://github.com/syoyo/tinygltf).
* [ ] Render [glTF 2.0 files](https://www.khronos.org/gltf/) with textures and animations
* [X] Basic camera class.
* [ ] [Bézier curves](https://en.wikipedia.org/wiki/B%C3%A9zier_curve).


### Threadpool demo (0.2 alpha) (date not set)
* [ ] Implement `-threads <N>` command line argument.
* [ ] Refactor `render_frame` method: Account for N buffering (prefer triple buffering).
* [X] Create a threadpool using C++17.
* [ ] Refactor the engine so it loads resources with worker threads. Use C++17 synchronisation techniques.
* [ ] Abstract command buffer recording into manager classes.
* [ ] Abstract pipeline creation into manager classes.
* [ ] Record command buffers on demand using separate thread.
* [ ] Update uniform buffers in separate thread.
* [ ] Poll window events in separate thread.
* [ ] Implement Vulkan pipeline statistics.
* [ ] Create new threads on demand.
* [ ] Give threadpool tasks a name.
* [ ] Use `std::chrono` to measure how long a task took to finish.

### imgui demo (0.3 alpha) (date not set)
* [ ] Add [imgui](https://github.com/ocornut/imgui) support.

### Octree demo (0.4 alpha) (date not set)
* [ ] Suggest implementation for inexor octree file format.
* [ ] Load octree data from a file.
* [ ] Render some world geometry which was generated from octree data.


## How to build
Feel free to open a ticket if you have problems generating project map files or building code.

[How to build vulkan-renderer](BUILDING.md)

## Who develops Vulkan?
The [Khronos Group](https://www.khronos.org/), which also made [OpenGL](https://www.opengl.org/).

## Who supports Vulkan ?
Just to give a selection of supporters:
* Google
* Intel
* Apple
* AMD
* NVidia
* Sony
* Samsung
* Huawei
* Qualcomm
* Valve Software

For a full list of contributors, see [this link](https://www.khronos.org/members/list).

## Which engines support Vulkan already?
* Unity engine
* Unreal engine (EPIC games)
* CryEngine (Crytek)
* id-Tech 7 (id-Software)
* Source engine (Valve)
* AnvilNext (Ubisoft)
* Godot 4
and many more..

## What's the plan?
First we will try to get some geometry rendered on screen. After this we will implement an octree renderer for Inexor based on this code.

## Links
### Vulkan API
https://www.khronos.org/vulkan/
https://vulkan-tutorial.com/

### Vulkan Examples
Sascha Willems' Vulkan examples:

https://github.com/SaschaWillems/Vulkan

Khronos Vulkan samples:

https://github.com/KhronosGroup/Vulkan-Samples

LunarG Vulkan samples:

https://github.com/LunarG/VulkanSamples

Intel Vulkan introduction:

https://github.com/GameTechDev/IntroductionToVulkan

Minimal Vulkan compute shader:

https://github.com/Erkaman/vulkan_minimal_compute

Vulkan Tutorial Github page:

https://github.com/Overv/VulkanTutorial

Niko Kauppi's Github page:

https://github.com/Niko40/Vulkan-API-Tutorials

Shabi's Vulkan Tutorials:

https://github.com/ShabbyX/vktut

### Vulkan Debuggers
https://renderdoc.org/

### Advantages of Vulkan
https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot

https://gamedev.stackexchange.com/questions/96014/what-is-vulkan-and-how-does-it-differ-from-opengl

https://www.imgtec.com/blog/stuck-on-opengl-es-time-to-move-on-why-vulkan-is-the-future-of-graphics/

https://www.toptal.com/api-developers/a-brief-overview-of-vulkan-api

https://developer.nvidia.com/Vulkan

https://www.quora.com/What-advantages-does-Vulkan-have-over-already-established-graphics-APIs

### Vulkan Tutorials
https://devblogs.nvidia.com/vulkan-dos-donts/

https://vulkan.lunarg.com/doc/sdk/1.0.26.0/linux/tutorial.html

https://www.toptal.com/api-developers/a-brief-overview-of-vulkan-api

https://vulkan-tutorial.com/

http://ogldev.atspace.co.uk/www/tutorial50/tutorial50.html

http://jhenriques.net/development.html

http://www.duskborn.com/posts/a-simple-vulkan-compute-example/


https://www.fasterthan.life/blog/2017/7/11/i-am-graphics-and-so-can-you-part-1

https://software.intel.com/en-us/articles/api-without-secrets-introduction-to-vulkan-part-1

### Multithreading in Vulkan
https://www.reddit.com/r/vulkan/comments/52aodq/multithreading_in_vulkan_where_should_i_start/

### OpenGL
https://www.opengl.org/

### MoltenVK
https://moltengl.com/moltenvk/
