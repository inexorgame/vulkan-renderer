# Overview

Inexor's vulkan-renderer is being developed with the following code-design characteristics:

* Try to keep master branch stable.
* Make sure the code is as easy to debug as possible.
* Always test your code, both with google tests and manually.
* Use C++17 and make extensive use of the standard library, also for multithreading.
* Document the code using [doxygen](http://doxygen.nl/) comments. Code without documentatin is almost useless.
* Organise the code in components. Try to keep dependencies between components at minimum because single components (e.g. classes) should be as recyclable as possible.
* Make sure the code is platform-independant. For now, we will support Windows and Linux but not Mac OS.
* Use `assert` to validate parameters or neccesary resources during development (debug mode).
* Use [spdlog](https://github.com/gabime/spdlog) instead of std::cout for console output.
* Use Vulkan debug markers using `VK_EXT_debug_marker`.
* Avoid data redundancy in the engine. Do not keep memory copied unnecessarily.
* Do not use global variables. Everything data object should be bundled in classes.
* Do not allocate memory manually. Use modern C++ features like [smart pointers](https://en.cppreference.com/book/intro/smart_pointers) or STL containers instead.
* If a third-party library is used, make sure it is up-to-date, well documented and battle-tested.
* The code must be *thread-safe* since we will use multiple threads. Use `std::mutex`, `std::shared_mutex`, `std::lock_guard` and more.
* Use [Vulkan memory allocator library](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) for Vulkan-specific memory allocations like buffers.
* Always write code in a way so it can't be used incorrectly. Assume worst-case scenarios and provide extensive error handling in case something goes wrong.
* Centralise methods for error handling. Do not use exceptions because they interrupt the logic order of the program (they don't free `std::mutex` for example). Use assert and return error codes instead.
* Do not add dependencies to the repository if possible. Use [conan package manager](https://conan.io/) instead. Sadly, not every dependency has a conan package (e.g. vma).
* If you must add a dependency, put it into `src/third_party/<name>`.
* Use [CMake](https://cmake.org/) as project map generator.
