Dependency management
=====================

- In general we try to keep the number of dependencies at a minimum.
- We avoid to add dependencies directly to the project repository because they increase the size of the repository and we have to update them manually.

- Instead, we prefer to use `conan package manager <https://conan.io/>`__ which allows us to get most dependencies from `conan center <https://conan.io/center/>`__.

Conan package manager
---------------------

- The list of currently used dependencies can be found in ``conanfile.py`` in the root folder of the repository.
- You must have installed `CMake <https://cmake.org/>`__ and `conan package manager <https://conan.io/>`__ in oder to download the dependencies automatically from conan center when running CMake.

Dependency folder
-----------------

- If we really need a dependency which is not yet available through conan center, we add it manually to the ``third_party`` folder.
- An example for such a dependency is `AMD's Vulkan Memory Allocator library (VMA) <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__, which is an essential library for out project but it's not yet available in conan center.

Criteria for library selection
------------------------------

If we really need a new library, it should be selected based on the following considerations:

- Are you sure we need this library? Can't we solve the problem with C++ standard library somehow?
- The library must have an open source license which is accepted by us (see `our list of accepted licenses <../../contributing/licenses.html>`_).
- It must be in active development.
- It must have a good documentation.
- A sufficient number of participants must have contributed so we can be sure it is reviewed.
