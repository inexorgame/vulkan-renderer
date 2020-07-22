***************
vulkan-renderer
***************

|programming language| |license|

|github actions| |readthedocs|

|issues| |last commit| |code size| |contributors| |downloads|

----

A new octree-based game+engine using `Vulkan API <https://www.khronos.org/vulkan/>`__ and `C++17 <https://stackoverflow.com/questions/38060436/what-are-the-new-features-in-c17>`__.

Our current octree demo ``v0.1-alpha.2`` can be downloaded `here <https://github.com/inexorgame/vulkan-renderer/releases>`__. Please send your logfiles to info@inexor.org.

Please visit `inexor.org <https://inexor.org>`__ and join our `Discord <https://discord.gg/acUW8k7>`__ server.

----

.. image:: https://raw.githubusercontent.com/inexorgame/artwork/2c479edcf7a1782d082a9d807b0f1e860ddd398c/vulkan/readme/front_banner_2.jpg

What is Vulkan?
###############

.. image:: https://upload.wikimedia.org/wikipedia/commons/thumb/3/30/Vulkan.svg/300px-Vulkan.svg.png

The Inexor project is using Vulkan API for the main rendering engine. `Vulkan <https://www.khronos.org/vulkan/>`__ is a new, low level API (`application programming interface <https://en.wikipedia.org/wiki/Application_programming_interface>`__) for high-performance graphics programming and computing. It is the successor to `OpenGL <https://en.wikipedia.org/wiki/OpenGL>`__, and it is important to state that is is very different from it. Vulkan is not just a new version of OpenGL or an extension of it. Like `DirectX 12 <https://en.wikipedia.org/wiki/DirectX>`__ or Apple's `Metal <https://en.wikipedia.org/wiki/Metal_(API)>`__, Vulkan is a very low level API which allows for much deeper control over the graphics card and the driver. Unlike OpenGL, Vulkan API is build in a way it fits the architecture of modern graphics cards. This offers `better performance <https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot>`__ (higher FPS) due to reduction of overhead and driver guesswork during runtime. The most important benefit of Vulkan is the fact that it allows for `multithreaded rendering <https://stackoverflow.com/questions/11097170/multithreaded-rendering-on-opengl>`__, which is not possible in OpenGL at all. In general, Vulkan does a lot of work during the initialization of the application but therefore reduces work during rendering. Since Vulkan is much more explicit in terms of code, it foces you to think about the structure and architecture of your code. Both Vulkan and OpenGL are being developed by the `Khronos Group <https://www.khronos.org/>`__.


Getting into Vulkan
###################

You really should watch these expert talks on YouTube:

`GDC 2018 - Getting explicit: How Hard is Vulkan really? <https://www.youtube.com/watch?v=0R23npUCCnw>`__
    Dustin Land, Software engineer, id-Software.
`DevU 2017: Getting Started with Vulkan <https://www.youtube.com/watch?v=yHZ3-AMJA6Y>`__
    Developers from Imagination, Google and LunarG.
`Porting your engine to Vulkan or DX12 <https://www.youtube.com/watch?v=6NWfznwFnMs>`__
    Adam Sawicki, Developer Software Engineer, AMD.
`Vulkan Best Practices Roundtable discussion <https://www.youtube.com/watch?v=owuJRPKIUAg>`__
    NVidia, Imagination, Qualcomm, id-Software, EPIC-games and Google.
`Vulkan Memory Management <https://www.youtube.com/watch?v=rXSdDE7NWmA>`__
    Jordan Logan, Developer technology engineer, AMD.
`Vulkan Memory Managenent <https://www.youtube.com/watch?v=zSG6dPq57P8>`__
    Steven Tovey, Developer technology engineer, AMD.
`Vulkan: State of the Union 2019 <https://www.youtube.com/watch?v=KLZsAJQBR5o>`__
    Developers from ARM, LunarG, NVidia.


Why use Vulkan?
###############

- Unlike OpenGL, Vulkan fits the design of modern GPUs as it is not just one single `state machine <https://stackoverflow.com/questions/31282678/what-is-the-opengl-state-machine>`__.
- Vulkan is a low-level API which gives much more control over GPU behaviour. This reduces driver guesswork und avoids undefined behaviour of graphics drivers.
- The API is asynchronous and encourages multithreaded rendering. This is a major advantage over OpenGL! Vulkan also wants you to use the GPU asynchronously.
- Lower and more predictable CPU load which results in `better performance <https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot>`__ and a reduction of driver guesswork.
- The reduction of CPU workload and it's improved predictability can enforce the GPU to be the limiting factor (as it should be), instead of the CPU.
- Vulkan implies memory-management to be done by the application (by you) rather than the driver.
- Vulkan is a fresh start, whereas OpenGL contains a myriad of hacks to support very rare use cases.
- Available on a variety of platforms: Windows, Linux, mobile devices and much more!
- `Validation layers <https://github.com/KhronosGroup/Vulkan-ValidationLayers>`__ and diagnostics can be independently activated during development, allowing better error handling and debugging compared with OpenGL or DirectX. Upon release builds, the validation layers can be deactivated easily.
- Vulkan pre-compiles shaders to a `standardised bytecode format <https://en.wikipedia.org/wiki/Standard_Portable_Intermediate_Representation>`__. This again reduces driver guesswork during runtime.
- Vulkan API and most of the drivers are `open source <https://en.wikipedia.org/wiki/Open_source>`__! (unlike DirectX 12 for example)
- Vulkan has layers and extensions as part of its design. For example it's easy to put steam overlay into a game simply by enabling Valve's steam overlay layer.
- Vulkan is being developed through an `unprecedented collaboration <https://www.khronos.org/members/list>`__ of major industry-leading companies. It is not being developed by one company only (like DirectX by Microsoft). As Vulkan's motto states, it really is `industry-forged`.
- The `ending of the OpenGL era <https://www.reddit.com/r/opengl/comments/b44tyu/apple_is_deprecating_opengl/>`__ has begun.

Releases
########

`Initialisation and glTF2 demo (v0.1-alpha.1), April 12th, 2020 <https://github.com/inexorgame/vulkan-renderer/releases/tag/v0.1-alpha.1>`__

`Octree demo (v0.1-alpha.2), April 26th, 2020 <https://github.com/inexorgame/vulkan-renderer/releases/tag/v0.1-alpha.2>`__

How to build
############

Feel free to open a ticket if you have problems generating project map files or building code.

`How to build vulkan-renderer <https://inexor-vulkan-renderer.readthedocs.io/en/latest/development/building.html>`__

Who develops Vulkan?
####################

The `Khronos Group <https://www.khronos.org/>`__, which also made `OpenGL <https://www.opengl.org/>`__.

Who supports Vulkan?
####################
- Google
- Intel
- Apple
- AMD
- NVidia
- Sony
- Samsung
- Huawei
- Qualcomm
- Valve Software

For a full list of contributors, see `this link <https://www.khronos.org/members/list>`__.

Which engines support Vulkan already?
#####################################
- Unity engine
- Unreal engine (EPIC games)
- CryEngine (Crytek)
- id-Tech 7 (id-Software)
- Source engine (Valve)
- AnvilNext (Ubisoft)
- Godot 4

and many more..

Links
#####

Vulkan API
----------

- https://www.khronos.org/vulkan/
- https://vulkan-tutorial.com/

Vulkan Examples
---------------

- https://github.com/SaschaWillems/Vulkan
    Sascha Willems' Vulkan examples
- https://github.com/KhronosGroup/Vulkan-Samples
    Khronos Vulkan samples
- https://github.com/LunarG/VulkanSamples
    LunarG Vulkan samples
- https://github.com/GameTechDev/IntroductionToVulkan
    Intel Vulkan introduction
- https://github.com/Erkaman/vulkan_minimal_compute
    Minimal Vulkan compute shader
-  https://github.com/Overv/VulkanTutorial
    Vulkan Tutorial Github page
- https://github.com/Niko40/Vulkan-API-Tutorials
    Niko Kauppi's Github page
- https://github.com/ShabbyX/vktut
    Shabi's Vulkan Tutorials

Vulkan Debuggers
----------------

- https://renderdoc.org/

Advantages of Vulkan
--------------------

- https://stackoverflow.com/questions/56766983/what-can-vulkan-do-specifically-that-opengl-4-6-cannot
- https://gamedev.stackexchange.com/questions/96014/what-is-vulkan-and-how-does-it-differ-from-opengl
- https://www.imgtec.com/blog/stuck-on-opengl-es-time-to-move-on-why-vulkan-is-the-future-of-graphics/
- https://www.toptal.com/api-developers/a-brief-overview-of-vulkan-api
- https://developer.nvidia.com/Vulkan
- https://www.quora.com/What-advantages-does-Vulkan-have-over-already-established-graphics-APIs

Vulkan Tutorials
----------------

- https://devblogs.nvidia.com/vulkan-dos-donts/
- https://vulkan.lunarg.com/doc/sdk/1.0.26.0/linux/tutorial.html
- https://www.toptal.com/api-developers/a-brief-overview-of-vulkan-api
- https://vulkan-tutorial.com/
- http://ogldev.atspace.co.uk/www/tutorial50/tutorial50.html
- http://jhenriques.net/development.html
- http://www.duskborn.com/posts/a-simple-vulkan-compute-example/
- https://www.fasterthan.life/blog/2017/7/11/i-am-graphics-and-so-can-you-part-1
- https://software.intel.com/en-us/articles/api-without-secrets-introduction-to-vulkan-part-1

Multithreading in Vulkan
------------------------

- https://www.reddit.com/r/vulkan/comments/52aodq/multithreading_in_vulkan_where_should_i_start/

OpenGL
------

- https://www.opengl.org/

MoltenVK
--------
- https://moltengl.com/moltenvk/

.. Badges.

.. |github actions| image:: https://github.com/inexorgame/vulkan-renderer/workflows/Build/badge.svg
   :target: https://github.com/inexorgame/vulkan-renderer/actions?query=workflow%3A%22Build%22

.. |license| image:: https://img.shields.io/badge/License-MIT-brightgreen.svg
   :target: https://github.com/inexorgame/vulkan-renderer/blob/master/LICENSE.rst

.. |programming language| image:: https://img.shields.io/badge/Language-C++17-orange.svg
   :target: https://inexor-vulkan-renderer.readthedocs.io/en/latest/development/design/coding-style.html

.. |contributors| image:: https://img.shields.io/github/contributors/inexorgame/vulkan-renderer
   :target: https://inexor-vulkan-renderer.readthedocs.io/en/latest/contributors/main.html

.. |downloads| image:: https://img.shields.io/github/downloads/inexorgame/vulkan-renderer/total

.. |readthedocs| image:: https://readthedocs.org/projects/inexor-vulkan-renderer/badge/?version=latest
   :target: https://inexor-vulkan-renderer.readthedocs.io

.. |last commit| image:: https://img.shields.io/github/last-commit/inexorgame/vulkan-renderer

.. |issues| image:: https://img.shields.io/github/issues/inexorgame/vulkan-renderer
   :target: https://github.com/inexorgame/vulkan-renderer/issues

.. |code size| image:: https://img.shields.io/github/languages/code-size/inexorgame/vulkan-renderer
