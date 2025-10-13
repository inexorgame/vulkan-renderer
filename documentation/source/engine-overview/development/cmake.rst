CMake Build System
==================

* See also: :ref:`BUILD_INSTRUCTIONS`

* We use `CMake <https://cmake.org/>`__ as a `build system <https://en.wikipedia.org/wiki/Build_system_(software_development)>`__ for this project.
* CMake generates project files (e.g. for Visual Studio) to build the renderer (library, sample app, tests, and benchmarks), the shaders for the renderer, and the documentation.
* The CMake setup takes care of downloading all required dependencies and finding libraries on the system.
* Without CMake, it would be impossible to keep an up-to-date project solution file.

.. note::
    In early versions of this project (until ``v0.1.0-alpha-3``), we used `conan package manager <https://conan.io/>`__ package manager for downloading and managing C++ dependencies. However, we removed conan package manager from the project in `pull request 528 <https://github.com/inexorgame/vulkan-renderer/pull/528>`__ and instead download all dependencies directly through `CMake <https://cmake.org/>`__ using the `FetchContent <https://cmake.org/cmake/help/latest/module/FetchContent.html>`__ commands.
