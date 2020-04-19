Building
========

CMake
-----

This project uses out of source builds. Except the documentation, which will create two folders in ``doc``, they are ignored by git.
There are three CMake targets.

- inexor-vulkan-renderer
    The main program and executable.
- inexor-vulkan-renderer-benchmark
    Benchmark the renderer.
- inexor-vulkan-renderer-tests
    Tests the renderer.
- inexor-vulkan-renderer-documentation
    Builds the documentation with Sphinx. Enable target creation with ``-DINEXOR_BUILD_DOC=ON``.

Available CMake options

- INEXOR_CONAN_PROFILE
    To adjust the conan profile, use ``-DCONNECTOR_CONAN_PROFILE=<name>``. The building type will be overriden by CMake.
    Default: ``default``
- INEXOR_BUILD_BENCHMARKS
    Builds inexor-renderer-benchmarks.
    Default: ``OFF``
- INEXOR_BUILD_DOC
    To build the documentation enable it.
    Default: ``OFF``
- INEXOR_BUILD_EXAMPLE
    Builds inexor-renderer-example.
    Default: ``ON``
- INEXOR_BUILD_TESTS
    Builds inexor-renderer tests.
    Default: ``OFF``
- INEXOR_USE_VMA_RECORDING
    Enables or disables VulkanMemoryAllocator's recording feature.
    Default: ``ON``

.. note::
    When building a VS solution add ``--config Debug/Release`` to define the build type.

.. code-block:: shell

    # executing from project root assumed
    # Ninja generator and Debug type
    cmake -DINEXOR_CONAN_PROFILE=default -G Ninja -B./cmake-build-debug/ -DCMAKE_BUILD_TYPE=Debug ./
    # Ninja generator and Release type
    cmake -G Ninja -B./cmake-build-release/ -DCMAKE_BUILD_TYPE=Release ./
    # Create Visual Studio Solution
    cmake -G "Visual Studio 16 2019" -A x64 -B./cmake-build-debug-vs/ -DCMAKE_BUILD_TYPE=Debug ./
    # Build all targets
    cmake --build ./cmake-build-debug/
