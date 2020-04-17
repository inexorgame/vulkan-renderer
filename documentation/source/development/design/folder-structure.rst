Folder structures
=================

Source
------

.. code-block:: none

    connector/  «project root»
    ├── .gitignore  «git ignore»
    ├── .travis.yml  «Travis CI configuration»
    ├── CMakeLists.txt
    ├── CODE_OF_CONDUCT.md
    ├── conanfile.py  «Conan configuration»
    ├── CONTRIBUTING.md
    ├── LICENSE.md
    ├── README.md
    ├── .github/  «GitHub templates and action configurations»
    ├── assets/
    │   ├── models/
    │   └── textures/  «textures»
    ├── benchmarks/
    ├── cmake/  «CMake helpers»
    ├── configuration/
    ├── documentation/
    │   ├── CMakeLists.txt  «CMake file for the documentation»
    │   ├── cmake/  «documentation cmake helpers»
    │   └── source/  «documentation source code»
    ├── include/  «header files»
    ├── shaders/
    ├── src/  «source code»
    ├── tests/
    ├── third_party/  «third party dependencies»
    ├── vma-dumps/
    └── vma-replays/


Application
-----------

.. code-block:: none

    vulkan-renderer/  «application root»
    ├── inexor-vulkan-renderer.exe  «executable»
    ├── ...
    ├── assets/
    ├── shaders/
    └── ...
