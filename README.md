# vulkan-renderer
[![Build Status](https://travis-ci.org/inexorgame/vulkan-renderer.svg?branch=master)](https://travis-ci.org/inexorgame/vulkan-renderer)
![Discord](https://img.shields.io/discord/698219248954376256)
[![License: MIT](https://img.shields.io/badge/License-MIT-brightgreen.svg)](https://opensource.org/licenses/MIT)
![Issues](https://img.shields.io/github/issues/inexorgame/vulkan-renderer)
![Contributors](https://img.shields.io/github/contributors/inexorgame/vulkan-renderer)
![Downloads](https://img.shields.io/github/downloads/inexorgame/vulkan-renderer/total)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://github.com/inexorgame/vulkan-renderer)
![Last Commit](https://img.shields.io/github/last-commit/inexorgame/vulkan-renderer)
![Code Size](https://img.shields.io/github/languages/code-size/inexorgame/vulkan-renderer)

A new octree-based game+engine using [Vulkan API](https://www.khronos.org/vulkan/) and [C++17](https://stackoverflow.com/questions/38060436/what-are-the-new-features-in-c17).

Our very first tech demo `v0.1-alpha.1` can be downloaded [here](https://github.com/inexorgame/vulkan-renderer/releases). Please send your logfiles to info@inexor.org.

Please visit [inexor.org](https://inexor.org) and join our [discord](https://discord.gg/acUW8k7). 

![Add frontbanner here](https://raw.githubusercontent.com/inexorgame/artwork/179e891a10f8ee1cd4c0f777aff40485f0212c76/vulkan/readme/front_banner_1.jpg)

## What is Vulkan?
![Vulkan API logo here..](https://upload.wikimedia.org/wikipedia/commons/thumb/3/30/Vulkan.svg/300px-Vulkan.svg.png)

Vulkan is a new, low level API ([application programming interface](https://en.wikipedia.org/wiki/Application_programming_interface)) for high-performance graphics programming and computing. It is seen by some as the successor to OpenGL, although it is important to state that is is very different from it. Vulkan is not just a new version of OpenGL or an extension of it. Both Vulkan and OpenGL are being developed by the [Khronos Group](https://www.khronos.org/). Like DirectX 12 or Apple's Metal, Vulkan is a low level API which allows for much deeper control over the graphics card and the driver. This offers better performance (higher FPS) due to reduction of overhead and driver guesswork during runtime. In general, Vulkan does a lot of work during the initialisation of the application but therefore reduces work during rendering. Since Vulkan is much more explicit in terms of code, it foces you to think about the structure and architecture of your code.

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

[Initialisation and glTF2 demo (0.1 alpha), April 12th, 2020](https://github.com/inexorgame/vulkan-renderer/releases/tag/v0.1-alpha.1)

Next planned releases:

### [Threadpool](https://en.wikipedia.org/wiki/Thread_pool) demo (0.2 alpha) (est. April 26th, 2020)
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

### [imgui](https://github.com/ocornut/imgui) demo (0.3 alpha) (date not set)
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