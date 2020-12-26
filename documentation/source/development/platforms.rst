Supported platforms
===================

- Vulkan API is completely platform-agnostic, which allows it to run on various operating systems.
- The required drivers for Vulkan are usually part of your graphic card's drivers.
- Update your graphics drivers ase often as possible since new drivers with Vulkan updates are released frequently.
- Driver updates contain new features, bug fixes, and performance improvements.
- Check out `Khronos website <https://www.khronos.org/vulkan/>`__ for more information.

Microsoft Windows
-----------------

- We support x64 Microsoft Windows 7, 8, 8.1 and 10.

Linux
------

- We support every x64 Linux distribution for which Vulkan drivers exist.
- We have specific build instructions for `Gentoo <https://www.gentoo.org/>`__ and `Ubuntu <https://ubuntu.com/download>`__.
- If you have found a way to set it up for other Linux distributions, please `open a pull request <https://github.com/inexorgame/vulkan-renderer/pulls>`__ and let us know!

macOS and iOS
-------------

- We do not support macOS or iOS because it would require us to use `MoltenVK <https://github.com/KhronosGroup/MoltenVK>`__ to get Vulkan running on Mac OS.
- Additionally, this would require some changes in the engines as not all of Inexor's dependencies are available on macOS or iOS.

Android
-------

- We also do not support Android because this would require some changes in the engines as not all of Inexor's dependencies are available on Android.
