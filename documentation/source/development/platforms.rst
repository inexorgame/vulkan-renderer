Platforms
=========

Inexor currently supports Linux and Microsoft Windows (7/8/8.1/10).

Vulkan API itself is completely platform agnostic. This means the core of Vulkan itself does not know about operating systems. This allows it to run on various operating systems. Check out `Khronos website <https://www.khronos.org/vulkan/>`__ for more information.

We do not support macOS because it would require us to use `MoltenVK <https://github.com/KhronosGroup/MoltenVK>`__ to get Vulkan running on Mac OS.

We also do not support Android because this would require some changes in the engines as not all of Inexor's dependencies are available on Android.
