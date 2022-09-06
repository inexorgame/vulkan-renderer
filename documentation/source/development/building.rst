.. _BUILDING:

Building vulkan-renderer
========================

 * :ref:`BUILDING Windows`
 * :ref:`BUILDING Linux`

Also see :ref:`GETTING_STARTED`.

If you have any trouble please `open a ticket <https://github.com/inexorgame/vulkan-renderer/issues>`__ or join our `Discord server <https://discord.com/invite/acUW8k7>`__.

This project uses out of source builds using either `gcc <https://gcc.gnu.org/>`__, `clang <https://clang.llvm.org/>`__ or `Microsoft Visual Studio <https://visualstudio.microsoft.com/en/downloads/>`__ compiler.

Generating the documentation will create two subfolders in ``doc`` which will be ignored by git.

The following CMake targets and options are available:

.. list-table:: List of CMake build targets.
   :header-rows: 1

   * - build target
     - description
     - comment
   * - inexor-vulkan-renderer
     - The main executable.
     -
   * - inexor-vulkan-renderer-tests
     - Tests the renderer using `Google Test <https://github.com/google/googletest>`__.
     - There are no tests available yet.
   * - inexor-vulkan-renderer-benchmark
     - Benchmarking of the renderer using `Google Benchmark <https://github.com/google/benchmark>`__.
     - There are no benchmarks available yet.
   * - inexor-vulkan-renderer-documentation
     - Builds the documentation with `Sphinx <https://www.sphinx-doc.org/en/master/>`__. Enable target creation with ``-DINEXOR_BUILD_DOC=ON``.
     -
   * - inexor-vulkan-renderer-documentation-linkcheck
     - Use sphinx's linkcheck feature to search for broken links.
     -

.. list-table:: List of CMake options.
   :header-rows: 1

   * - option
     - description
     - default value
   * - INEXOR_BUILD_EXAMPLE
     - Builds inexor-renderer-example.
     - ``ON``
   * - INEXOR_BUILD_TESTS
     - Builds inexor-renderer-tests.
     - ``OFF``
   * - INEXOR_BUILD_BENCHMARKS
     - Builds inexor-renderer-benchmarks.
     - ``OFF``
   * - INEXOR_CONAN_PROFILE
     - To adjust the conan profile, use ``-DINEXOR_CONAN_PROFILE=<name>``.
     - ``default``
   * - INEXOR_BUILD_DOC
     - Builds the documentation with `Sphinx <https://www.sphinx-doc.org/en/master/>`__.
     - ``OFF``
   * - INEXOR_BUILD_DOCUMENTATION_USE_VENV
     - Generate and use a Python virtual environment for the documentation dependencies.
     - ``ON``

.. _BUILDING windows:

Windows
^^^^^^^

Example: Create Visual Studio 2019 project map for Debug mode including docs, tests, and benchmarks:

.. code-block:: shell

    cmake -G "Visual Studio 16 2019" -A x64 -B./cmake-build-debug-vs/ -DCMAKE_BUILD_TYPE=Debug -DINEXOR_BUILD_DOC=ON -DINEXOR_BUILD_TESTS=ON -DINEXOR_BUILD_BENCHMARKS=ON ./

Example: Create Visual Studio 2019 project map for Release mode but without docs, tests, and benchmarks:

.. code-block:: shell

    cmake -G "Visual Studio 16 2019" -A x64 -B./cmake-build-release-vs/ -DCMAKE_BUILD_TYPE=Release ./

If you have `Ninja build system <https://ninja-build.org/>`__ installed, you can use it like this:

.. code-block:: doscon

    # executing from project root assumed
    # Ninja generator and Debug type
    \> cmake -DINEXOR_CONAN_PROFILE=default -G Ninja -B./cmake-build-debug/ -DCMAKE_BUILD_TYPE=Debug ./
    # Ninja generator and Release type
    \> cmake -G Ninja -B./cmake-build-release/ -DCMAKE_BUILD_TYPE=Release ./
    # Create Visual Studio Solution
    \> cmake -G "Visual Studio 16 2019" -A x64 -B./cmake-build-debug-vs/ -DCMAKE_BUILD_TYPE=Debug ./
    # Build all targets
    \> cmake --build ./cmake-build-debug/

.. note::
    If you use CMake GUI add `CMAKE_BUILD_TYPE` with value `Debug` or `Release`. `#228 <https://github.com/inexorgame/vulkan-renderer/issues/228>`__.

- Choose any IDE that CMake can generate a project map for. If in doubt use `Visual Studio 2019 <https://visualstudio.microsoft.com/>`__.
- Clone the source code. Free and good tools are `GitHub Desktop <https://desktop.github.com/>`__ or `GitKraken Git GUI <https://www.gitkraken.com/git-client>`__.
- Open CMake and select the root folder which contains ``CMakeLists.txt`` (not just ``src`` folder!).
- You can choose any location for the ``build`` folder.
- Click "Configure" and select your IDE (in doubt ``Visual Studio 16 2019``). Click "Finish".
- CMake will now set up dependencies automatically for you using conan package manager. This might take a while. If this fails, you really should open a ticket!
- Click "Generate". You can now open the Visual Studio project file in your ``build`` folder.
- For debugging, please check that the root directory of the repository is set as working directory in Visual Studio. Usually, CMake should take care of this already.
- You are now ready to start debugging! Our main branch must be stable at all cost.

.. _BUILDING linux:

Linux
^^^^^

Install dependencies and tools:

+--------+--------------------------------------+
| Ubuntu | Follow the                           |
|        | `Install the SDK`_-instructions on   |
|        | the vulkan-sdk page.                 |
|        |                                      |
|        | Install the required packages:[#f1]_ |
|        |                                      |
|        | .. code-block:: shell-session        |
|        |                                      |
|        |     # apt install -y \               |
|        |         pkg-config \                 |
|        |         libglm-dev \                 |
|        |         libxcb-dri3-0 \              |
|        |         libxcb-present0 \            |
|        |         libpciaccess0 \              |
|        |         libpng-dev \                 |
|        |         libxcb-keysyms1-dev \        |
|        |         libxcb-dri3-dev \            |
|        |         libx11-dev  \                |
|        |         libmirclient-dev \           |
|        |         libwayland-dev \             |
|        |         libxrandr-dev \              |
|        |         libxcb-ewmh-dev              |
|        |     # apt install -y \               |
|        |         cmake \                      |
|        |         ninja-build \                |
|        |         clang-tidy \                 |
|        |         vulkan-sdk \                 |
|        |         python3 \                    |
|        |         python3-pip                  |
|        |     $ pip3 install \                 |
|        |         wheel \                      |
|        |         setuptools \                 |
|        |         conan                        |
|        |                                      |
+--------+--------------------------------------+
| Gentoo | .. code-block:: shell-session        |
|        |                                      |
|        |     # emerge \                       |
|        |        dev-util/cmake \              |
|        |        dev-util/conan \              |
|        |        dev-util/vulkan-headers \     |
|        |        dev-util/vulkan-tools \       |
|        |        dev-vcs/git \                 |
|        |        media-libs/vulkan-layers \    |
|        |        media-libs/vulkan-loader      |
|        |                                      |
|        |                                      |
|        | Install ninja build tool (optional): |
|        |                                      |
|        |                                      |
|        | .. code-block:: shell-session        |
|        |                                      |
|        |     # emerge dev-util/ninja          |
+--------+--------------------------------------+
| Debian | Follow the                           |
|        | `Install the SDK`_-instructions on   |
|        | the vulkan-sdk page.                 |
|        |                                      |
|        | Install the required packages:[#f1]_ |
|        |                                      |
|        | .. code-block:: shell-session        |
|        |                                      |
|        |     # apt install -y \               |
|        |         libvulkan-dev                |
|        |         glslang-dev                  |
|        |         glslang-tools                |
|        |         vulkan-tools                 |
|        |         vulkan-validationlayers-dev  |
|        |         spirv-tools                  |
|        |         pkg-config \                 |
|        |         libglm-dev \                 |
|        |         libxcb-dri3-0 \              |
|        |         libxcb-present0 \            |
|        |         libpciaccess0 \              |
|        |         libpng-dev \                 |
|        |         libxcb-keysyms1-dev \        |
|        |         libxcb-dri3-dev \            |
|        |         libx11-dev  \                |
|        |         libmirclient-dev \           |
|        |         libwayland-dev \             |
|        |         libxrandr-dev \              |
|        |         libxcb-ewmh-dev              |
|        |     # apt install -y \               |
|        |         cmake \                      |
|        |         ninja-build \                |
|        |         clang-tidy \                 |
|        |         vulkan-sdk \                 |
|        |         python3 \                    |
|        |         python3-pip                  |
|        |     $ pip3 install \                 |
|        |         wheel \                      |
|        |         setuptools \                 |
|        |         conan                        |
|        |                                      |
+--------+--------------------------------------+
| Arch   | Follow the                           |
|        | `Install the SDK`_-instructions on   |
|        | the vulkan-sdk page.                 |
|        |                                      |
|        | Install the required packages:[#f1]_ |
|        |                                      |
|        | .. code-block:: shell-session        |
|        |                                      |
|        |     # pacman -S \                    |
|        |         pkg-config \                 |
|        |         glslang \                    |
|        |         spirv-tools \                |
|        |         glm \                        |
|        |         libice \                     |
|        |         libpciaccess \               |
|        |         libpng \                     |
|        |         libx11 \                     |
|        |         libxres \                    |
|        |         xkeyboard-config \           |
|        |         libxrandr \                  |
|        |         libxcb \                     |
|        |         libxaw \                     |
|        |         xcb-util \                   |
|        |         xtrans \                     |
|        |         libxvmc                      |
|        |     # pacman -S \                    |
|        |         cmake \                      |
|        |         ninja \                      |
|        |         vulkan-headers \             |
|        |         vulkan-tools \               |
|        |         vulkan-validation-layers \   |
|        |         python3 \                    |
|        |         python-pip                   |
|        |     $ pip3 install \                 |
|        |         wheel \                      |
|        |         setuptools \                 |
|        |         conan                        |
|        |                                      |
+--------+--------------------------------------+
| Other  | Planned. `We would love to see a     |
|        | pull request on this file if you get |
|        | it running on other                  |
|        | distributions.`__                    |
+--------+--------------------------------------+

__ https://github.com/inexorgame/vulkan-renderer/blob/main/documentation/source/development/building.rst

.. _Install the SDK: https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html#user-content-install-the-sdk


Clone the repository:

.. code-block:: shell-session

    $ git clone https://github.com/inexorgame/vulkan-renderer
    $ cd vulkan-renderer

Configure cmake:

.. note::

    Only pass ``-GNinja`` if the ninja build tool is installed.

.. code-block:: shell-session

    $ cmake . \
       -Bbuild \
       -DCMAKE_BUILD_TYPE=Debug \
       -GNinja

Build and run:

If you have any trouble please `open a ticket <https://github.com/inexorgame/vulkan-renderer/issues>`__ or join our `Discord server <https://discord.com/invite/acUW8k7>`__.

.. code-block:: shell-session

    $ cmake --build build --target inexor-vulkan-renderer-example
    $ ./build/bin/inexor-vulkan-renderer-example

.. rubric:: Footnotes

.. [#f1] Make sure that ``$PATH`` includes the directory which contains ``conan`` (normally ``$HOME/.local/bin``). Bash includes this directory by default, zsh does **not**.
