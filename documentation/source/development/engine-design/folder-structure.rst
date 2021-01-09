Folder structure
================

- Use lowercase filenames, spaces should be written as underscore.

Source code
-----------

.. code-block:: none

    connector/  «project root»
    ├── .clang-format  «Clang Format configuration»
    ├── .clang-tidy  «Clang Tidy configuration»
    ├── .git-blame-ignore-revs  «git ignore revisions»
    ├── .gitignore  «git ignore»
    ├── .readthedocs.yml  «Read The Docs configuration»
    ├── CHANGELOG.rst
    ├── CMakeLists.txt
    ├── CODE_OF_CONDUCT.md
    ├── conanfile.py  «Conan configuration»
    ├── CONTRIBUTING.md
    ├── LICENSE.md
    ├── README.rst
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
    ├── example/  «example application»
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
