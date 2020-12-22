Dependencies
============

In general we avoid to add dependencies directly to the project repository because they increase the size of the repository and we have to update them manually.
Instead, we prefer to use `conan package manager <https://conan.io/>`__ which allows us to get most dependencies from `conan center <https://conan.io/center/>`__.

Conan package manager
---------------------

The list of currently used dependencies can be found in ``conanfile.py``.
You must have installed `CMake <https://cmake.org/>`__ and `conan package manager <https://conan.io/>`__ in oder to download the dependencies automatically from conan center.

Dependency folder
---------------------
If we really need a dependency which is not yet available through conan center, we add it manually to the ``third_party`` folder.
An example for such a dependency is `AMD's Vulkan Memory Allocator library (VMA) <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__, which is an essential library for out project but it's not yet available in conan center.
