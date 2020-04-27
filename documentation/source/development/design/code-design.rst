Code Design
===========

- Split declarations and definitions, if possible.
- Use lowercase filenames, spaces should be written as underscore
- Organise the code in components. Try to keep dependencies between components at minimum because single components (e.g. classes) should be as recyclable as possible.
- Use C++17 and make extensive use of the standard library, also for multithreading.
- The code must be *thread-safe* since we will use multiple threads. Use ``std::mutex``, ``std::shared_mutex``, ``std::lock_guard`` and more.
- Follow Scott Meyer's book 'Effective Modern C++'
- Make sure the code is platform-independent. For now, we will support Windows and Linux but not Mac OS.
- Use Vulkan debug markers using ``VK_EXT_debug_marker``.
- Use ``assert`` to validate parameters or necessary resources during development (debug mode).
- Use `spdlog <https://github.com/gabime/spdlog>`__ instead of std::cout for console output.
- Document the code using `doxygen <http://doxygen.nl/>`__ comments. Code without documentation is almost useless.
- Avoid data redundancy in the engine. Do not keep memory copied unnecessarily.
- Do not allocate memory manually. Use modern ++ features like `smart pointers <https://en.cppreference.com/book/intro/smart_pointers>`__ or STL containers instead.
- Use `Vulkan memory allocator library <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__ for Vulkan-specific memory allocations like buffers.
- Do not use global variables. Everything data object should be bundled in classes.