# vulkan-renderer
![License: CC0-1.0](https://img.shields.io/github/license/inexorgame/vulkan-renderer)
![Issues](https://img.shields.io/github/issues/inexorgame/vulkan-renderer)
[![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://github.com/inexorgame/vulkan-renderer)

A new rendering engine based on [Vulkan API 1.1](https://www.khronos.org/vulkan/)

## What is Vulkan?
![Vulkan API logo here..](https://upload.wikimedia.org/wikipedia/commons/thumb/3/30/Vulkan.svg/500px-Vulkan.svg.png)

Vulkan is a new API (application programming interface) for high-performance graphics programming and computing. It is seen by some as the successor to OpenGL, although it is important to state that is is very different from it. Both Vulkan and OpenGL are being developed by the [Khronos Group](https://www.khronos.org/). Like DirectX 12, Vulkan is a low level API which allows for much deeper control over the graphics card and the driver. This offers better performance (higher FPS) due to reduction of overhead and driver guesswork during runtime. In general, Vulkan does a lot of work during the initialisation of the application but therefore reduces work during rendering.

## Why use Vulkan?
* Unlike OpenGL, Vulkan fits the design of modern GPUs as it is not just one single [state machine](https://stackoverflow.com/questions/31282678/what-is-the-opengl-state-machine).
* Vulkan is a low-level API which gives much more control over GPU behaviour. This reduces driver guesswork und avoids undefined behaviour of graphics drivers.
* The API is asynchronous and encourages multithreaded rendering. This is a major advantage over OpenGL!
* Lower and more predictable CPU load which results in [better performance](https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot) and a reduction of driver guesswork.
* Vulkan implies memory-management to be done by the application rather than the driver.
* Available on a variety of platforms: Windows, Linux, mobile devices and much more!
* [Validation layers](https://github.com/KhronosGroup/Vulkan-ValidationLayers) and diagnostics can be independently activated during development, allowing better error handling and debugging compared with OpenGL or DirectX. Upon release builds, the validation layers can be deactivated easily.
* Vulkan is a fresh start, whereas OpenGL contains a myriad of hacks to support very rare use cases.
* Vulkan pre-compiles shaders to a [standardised bytecode format](https://en.wikipedia.org/wiki/Standard_Portable_Intermediate_Representation). This again reduces driver guesswork during runtime.
* Vulkan API and most of the drivers are [open source](https://en.wikipedia.org/wiki/Open_source)! (unlike DirectX 12 for example)
* Vulkan has layers and extensions. For example it's easy to put steam overlay into a game simply by enabling Valve's steam overlay layer.
* Vulkan is being developed through a collaboration of major industry-leading companies. It is not being developed by one company only (like DirectX by Microsoft). It's motto is therefore "`industry-forged`"

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
