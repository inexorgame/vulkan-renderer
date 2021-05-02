Frequently asked questions
==========================

Please visit `inexor.org <https://inexor.org>`__ and join our `Discord server <https://discord.com/invite/acUW8k7>`__.

----

- `What is Inexor?`_
- `Which platforms are supported?`_
- `What is the current state of the project?`_
- `How is Inexor organized?`_
- `How to contact us?`_
- `How to build?`_
- `Where to find Inexor's documentation?`_
- `What is Vulkan API?`_
- `Why is Vulkan API the future?`_
- `Can you explain Vulkan API in simple terms?`_
- `How difficult is development with Vulkan API?`_
- `Does my graphics card support Vulkan API?`_
- `Will you support other rendering APIs?`_
- `Which topics are currently not in focus of development?`_

----

What is Inexor?
---------------

.. image:: /images/inexor2.png


Inexor is a MIT-licensed open-source project which develops a new 3D octree game engine by combining `modern C++ <https://awesomecpp.com/>`__ with Vulkan `Vulkan API <https://www.khronos.org/vulkan/>`__.

**We have the following goals for the Inexor engine:**

- Combine `modern C++ <https://www.youtube.com/watch?v=TC9zhufV_Z8>`__ with `Vulkan API <https://www.khronos.org/vulkan/>`__.
- `Task-based parallelization <https://youtu.be/JpmK0zu4Mts?t=500>`__ using a `threadpool <https://community.khronos.org/t/opinions-on-using-threadpools-for-designing-a-vulkan-game-engine/105519>`__ and a `work stealing queue <https://stackoverflow.com/questions/2101789/implementation-of-a-work-stealing-queue-in-c-c>`__.
- `Generic rendering architecture <https://youtu.be/6NWfznwFnMs?t=1845>`__ using a `rendergraph <https://de.slideshare.net/DICEStudio/framegraph-extensible-rendering-architecture-in-frostbite>`__.
- Create a Vulkan API codebase which can be used in production.

**We are using good software engineering practices:**

- `Resource acquisition is initialization (RAII) <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-raii>`__.
- `Software design patterns <https://refactoring.guru/>`__.
- `Continuous integration (CI) <https://en.wikipedia.org/wiki/Continuous_integration>`__ using `GitHub actions <https://github.com/features/actions>`__.
- Code design by strict compliance with the `C++ core guidelines <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines>`__ and `Scott Meyers books <https://www.oreilly.com/library/view/effective-modern-c/9781491908419/>`__.
- Use of the new `C++ standard library <https://en.cppreference.com/w/cpp/header>`__ (C++11, C++14, and C++17).
- Code documentation using `doxygen <https://www.doxygen.nl/index.html>`__.
- Automatic `unit testing <https://github.com/google/googletest>`__ and `benchmarking <https://github.com/google/benchmark>`__.
- `Static code analysis <https://en.wikipedia.org/wiki/Static_program_analysis>`__ with `clang-tidy <https://clang.llvm.org/extra/clang-tidy/>`__.
- `Automatic code formatting <https://clang.llvm.org/docs/ClangFormat.html>`__ using `clang-format <https://clang.llvm.org/docs/ClangFormat.html>`__.
- `CMake <https://cmake.org/>`__ project setup with `conan package manager <https://conan.io/center/>`__ integration. 

You can find Vulkan example code online which follows the mantra "don't use this in production - it's tutorial code". Inexor disagrees with this as we believe that defeats its own purpose. If example code is not meant to be used in some other projects then there's something wrong with that example code. Many projects don't use a proper memory management library like `VMA <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__ or they do not abstract their code using `RAII <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-raii>`__, but use a lot of code duplication instead. Inexor is designed to be used in production. Bear in mind however that Inexor is also still far away from being a finished game engine.

----

Which platforms are supported?
------------------------------

- We support x64 Microsoft Windows 8, 8.1, and 10.
- We support every x64 Linux distribution for which Vulkan drivers exist.
- We have specific build instructions for `Gentoo <https://www.gentoo.org/>`__ and `Ubuntu <https://ubuntu.com/download>`__. If you have found a way to set it up for other Linux distributions, please `open a pull request <https://github.com/inexorgame/vulkan-renderer/pulls>`__ and let us know!
- We do not support macOS or iOS because it would require us to use `MoltenVK <https://github.com/KhronosGroup/MoltenVK>`__ to get Vulkan running on Mac OS. Additionally, this would require some changes in the engines as not all of Inexor's dependencies are available on macOS or iOS.
- We also do not support Android because this would require some changes in the engines as not all of Inexor's dependencies are available on Android.

----

What is the current state of the project?
-----------------------------------------

We are still in very early development, but this project can already offer:

- A modern C++17 codebase with a setup for CMake and conan package manager.
- Stable builds for Windows and Linux using `Continuous Integration (CI) <https://en.wikipedia.org/wiki/Continuous_integration>`__.
- A `rendergraph <https://de.slideshare.net/DICEStudio/framegraph-extensible-rendering-architecture-in-frostbite>`__ in early development.
- `ImGui <https://github.com/ocornut/imgui>`__ integration using separate renderpasses.
- `RAII <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rr-raii>`__ wrappers for various Vulkan resources.
- Extensive logging with `spdlog <https://github.com/gabime/spdlog>`_.
- `Vulkan Memory Allocator <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__ for graphics memory management.
- `VMA memory replays <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator#binaries>`__ for debugging are already working.
- Full `RenderDoc <https://renderdoc.org/>`__ integration with `internal resource naming <https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/>`__.

----

How is Inexor organized?
------------------------

- Inexor has no central authority.
- It's a headless collective which makes decisions through creative discussions.
- We are welcoming new contributors to our team.

----

How to contact us?
------------------

Please visit `inexor.org <https://inexor.org>`__ and join our `Discord <https://discord.com/invite/acUW8k7>`__ server.

----

How to build?
-------------

If you have any trouble building please `open a ticket <https://github.com/inexorgame/vulkan-renderer/issues>`__ or `join our Discord <https://discord.com/invite/acUW8k7>`__.

`How to build vulkan-renderer? <https://inexor-vulkan-renderer.readthedocs.io/en/latest/development/building.html>`__

Where to find Inexor's documentation?
-------------------------------------

- Read our docs `here <https://inexor-vulkan-renderer.readthedocs.io/en/latest/>`__.

----

What is Vulkan API?
-------------------

.. image:: /images/vulkan.png

Inexor uses `Vulkan API <https://www.khronos.org/vulkan/>`__ as rendering backend. Vulkan is a new, multi platform low level API (`application programming interface <https://en.wikipedia.org/wiki/Application_programming_interface>`__) for high-performance graphics programming and computing. It is the successor to `OpenGL <https://en.wikipedia.org/wiki/OpenGL>`__, and it is important to state that is is very different from it. Vulkan is not just a new version of OpenGL or an extension of it. Instead, Vulkan is a very low level API which allows for much deeper control over the graphics card and the driver, like `DirectX 12 <https://en.wikipedia.org/wiki/DirectX>`__ or Apple's `Metal <https://en.wikipedia.org/wiki/Metal_(API)>`__. Unlike OpenGL, Vulkan API is build in a way it fits the architecture of modern graphics cards. This offers `better performance <https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot>`__ due to reduction of overhead and driver guesswork during runtime. This results in higher frame rate, more predictable CPU workload and a lower memory usage. The most important benefit of Vulkan is the fact that it allows for `multithreaded rendering <https://stackoverflow.com/questions/11097170/multithreaded-rendering-on-opengl>`__, which is not possible in OpenGL at all. In general, Vulkan does a lot of work during the initialization of the application but therefore reduces work during rendering. Since Vulkan is much more explicit in terms of code, it foces you to think about the structure and architecture of your code. Both Vulkan and OpenGL are being developed by the `Khronos Group <https://www.khronos.org/>`__. Vulkan is being developed through an `unprecedented collaboration <https://www.khronos.org/members/list>`__ of major industry-leading companies (Google, Intel, AMD, NVidia, Sony, Samsung, Huawei, Qualcomm, Valve Software and many more). Vulkan is the only multi platform low level graphics API.

----

Why is Vulkan API the future?
-----------------------------

**Performance**

- Lower and more predictable CPU load which results in `better performance <https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot>`__ and a reduction of driver guesswork.
- Vulkan API is asynchronous and encourages `multithreaded rendering <https://www.reddit.com/r/vulkan/comments/52aodq/multithreading_in_vulkan_where_should_i_start/>`__. This is not possible with OpenGL!
- The low level API design of Vulkan allows for advanced optimizations such as `rendergraphs <https://de.slideshare.net/DICEStudio/framegraph-extensible-rendering-architecture-in-frostbite>`__ for generic rendering architectures.
- It also wants you to use the GPU asynchronously, sometimes referred to as GPU multithreading.
- Vulkan allows the use of multiple GPUs, even if they are not physically linked via crossfire bridge.
- The reduction of CPU workload and it's improved predictability can enforce the GPU to be the limiting factor of performance, as it should be.

**Memory efficiency**

- Vulkan gives much deeper control and better interfaces over graphics and system memory.
- Vulkan API enforces memory management to be done by the application rather than the driver.
- Since the application knows best about the importance of every resource it uses, Vulkan API allows for a better memory usage.

**Architecture**

- Unlike OpenGL, Vulkan fits the design of modern GPUs as it is not just one single `state machine <https://stackoverflow.com/questions/31282678/what-is-the-opengl-state-machine>`__. This means Vulkan API was designed from the beginning to match the architecture of modern graphics cards. OpenGL however still matches the design of graphics cards from the time it was invented in the 1990s.
- Vulkan is a fresh start, whereas OpenGL contains a myriad of hacks to support very rare use cases.
- Vulkan has `layers <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#extendingvulkan-layers>`__ and `extensions <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#extendingvulkan-extensions>`__ as part of its design. You can check for supported GPU plugins on the target machine and enable them as needed.
- Vulkan API itself is completely platform agnostic.
- Available on a variety of platforms: Windows, Linux, mobile devices and much more!
- The `ending of the OpenGL era <https://www.reddit.com/r/opengl/comments/b44tyu/apple_is_deprecating_opengl/>`__ has begun.
- Vulkan is being developed through an `unprecedented collaboration <https://www.khronos.org/members/list>`__ of major industry-leading companies. It is not being developed by one company only (like Microsoft's DirectX for example).
- As Vulkan's motto states, it really is `industry-forged`.

**Consistency and standardization**

- Vulkan precompiles shaders to a `standardized bytecode format <https://en.wikipedia.org/wiki/Standard_Portable_Intermediate_Representation>`__ called `SPIR-V <https://www.khronos.org/spir/>`__. This also reduces driver guesswork during runtime.
- The explicit design of Vulkan gives much deeper control and avoids driver guesswork and undefined behavior of graphics drivers.

**Debugging tools**

- `Validation layers <https://github.com/KhronosGroup/Vulkan-ValidationLayers>`__ and diagnostics can be independently activated during development, allowing better error handling and debugging compared with OpenGL or DirectX.
- Upon release builds, the validation layers can be turned off easily.
- Vulkan API applications can be debugged with `RenderDoc <https://renderdoc.org/>`__.
- The `Vulkan specification <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html>`__ is very easy to read and it is the central guideline for how to use the API.

**Open Source**

- Vulkan API and some Vulkan graphics card drivers are `open source <https://en.wikipedia.org/wiki/Open_source>`__.

----

Can you explain Vulkan API in simple terms?
-------------------------------------------

- Vulkan API gives programmers much deeper control over the gamer's hardware.
- If applied correctly, Vulkan can result in a significant performance boost.
- The API encourages the programmers to think in detail about graphics cards and their game engine.
- It offers advanced optimization techniques which can result in a lower RAM and video memory usage.
- Using Vulkan can yield in lower and more predictable CPU usage.
- Vulkan allows programmers to make more effective use of multiple CPU cores.

----

How difficult is development with Vulkan API?
---------------------------------------------

- This API does a lot of initialization during the loading phase of the application.
- The key to success is a good abstraction of Vulkan API based on the needs of the application/game.
- Vulkan is a C-style API. In simplified terms you fill out structures which start with ``Vk..`` and submit them together with other parameters to ``vk...`` functions. That's it. No complex interfaces.
- Vulkan API has a `very good documentation <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html>`__.
- The challenges of Vulkan game/engine development boil down to basic programming challenges: abstraction, resource management and parallelization.
- You may want to read `Vulkan in 30 minutes <https://renderdoc.org/vulkan-in-30-minutes.html>`__ by `Baldur Karlsson <https://github.com/baldurk/renderdoc>`__.

----

Does my graphics card support Vulkan API?
-----------------------------------------

- You can look up your graphics card in the `Vulkan hardware database <https://vulkan.gpuinfo.org/>`__ by `Sascha Willems <https://www.saschawillems.de/>`__.
- Every new graphics card which is coming out these days supports Vulkan API.
- Vulkan is also supported on older graphics cards going back to `Radeon HD 7000 series <https://en.wikipedia.org/wiki/Radeon_HD_7000_series>`__ and `Nvidia Geforce 6 series <https://en.wikipedia.org/wiki/GeForce_6_series>`__.

----

Will you support other rendering APIs?
--------------------------------------
- No, because testing for Vulkan already takes a lot of time and there is no sense in supporting deprecated technology.
- Some studios like id-software also `dropped OpenGL entirely <https://youtu.be/0R23npUCCnw?t=252>`__.
- Vulkan API is the only low level multi platform graphics and compute API.

----

Which topics are currently not in focus of development?
-------------------------------------------------------
- We are currently focusing on the renderer and Vulkan API. When the time has come, we will take parallelization into account.
- A game engine needs other components besides rendering of course. However, we are currently not focusing on the following topics: networking, sound, physics, packaging of game engine resources and everything else which is not related to rendering.
- We will not begin to support additional platforms besides Linux and Windows in the near future.

----
