# vulkan-renderer
A new rendering engine based on Vulkan API 1.1

![Vulkan API logo here..](https://www.khronos.org/assets/uploads/apis/vulkan2.svg")

## What is Vulkan?
Vulkan is a new API (application programming interface) for graphics programming. It is seen as the successor to OpenGL. Both Vulkan and OpenGL are being developed by the Khronos Group. Like DirectX 12, Vulkan is a low level API which allows for much deeper control over the graphics card and the driver. This offers better performance (higher FPS) due to reduction of overhead and driver guesswork during runtime. In general, Vulkan does a lot of work during the initialisation of the application but therefore reduces work during rendering.

## Who develops Vulkan?
The Khronos Group, which also made [OpenGL](https://www.opengl.org/).

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

## Why use Vulkan?
* The API is asynchronous and encourages multithreaded rendering. This is a major advantage over OpenGL!
* Lower and more predictable CPU load which results in [better performance](https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot).
* Support of a variety of platforms: Windows, Linux, mobile devices and much more!
* [Validation layers](https://github.com/KhronosGroup/Vulkan-ValidationLayers) and diagnostics can be independently activated during development, allowing better error handling and debugging compared with OpenGL or DirectX. Upon release builds, the validation layers can be deactivated easily.
* Vulkan pre-compiles shaders to a standardised bytecode format.
* Vulkan API and most of the drivers are open source! (unlike DirectX 12 for example)

## Which engines support Vulkan already?
* Unity
* Unreal Engine
* CryEngine
* id-Tech engine
* Source engine
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

### Advantages of Vulkan
https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot
https://gamedev.stackexchange.com/questions/96014/what-is-vulkan-and-how-does-it-differ-from-opengl
https://www.imgtec.com/blog/stuck-on-opengl-es-time-to-move-on-why-vulkan-is-the-future-of-graphics/
https://www.toptal.com/api-developers/a-brief-overview-of-vulkan-api
https://developer.nvidia.com/Vulkan
https://www.quora.com/What-advantages-does-Vulkan-have-over-already-established-graphics-APIs

## Multithreading in Vulkan
https://www.reddit.com/r/vulkan/comments/52aodq/multithreading_in_vulkan_where_should_i_start/

### OpenGL
https://www.opengl.org/

### MoltenVK
https://moltengl.com/moltenvk/
