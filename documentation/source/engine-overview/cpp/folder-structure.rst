Folder Structure
================

- Use lowercase filenames and use underscores instead of spaces.

.. code-block:: none

    connector/                  «project root»
    ├── .clang-format           «clang-format configuration»
    ├── .clang-tidy             «clang-tidy configuration»
    ├── .git-blame-ignore-revs  «git ignore revisions»
    ├── .gitignore              «git ignore»
    ├── .readthedocs.yml        «Read The Docs configuration»
    ├── CMakeLists.txt
    ├── CODE_OF_CONDUCT.rst
    ├── CONTRIBUTING.rst
    ├── LICENSE
    ├── README.rst
    ├── .github/                «GitHub templates and GitHub action workflows»
    ├── assets/
    │   ├── fonts/              «ImGui font files»             
    │   ├── models/             «glTF2 models»
    │   └── textures/           «textures»
    ├── benchmarks/             «benchmarks»
    ├── configuration/          «TOML configuration files»
    ├── documentation/
    │   ├── CMakeLists.txt      «CMake file for the documentation»
    │   ├── cmake/              «documentation cmake helpers»
    │   └── source/             «documentation source code»
    ├── example/                «example application»
    ├── include/                «header files»
    ├── shaders/                «GLSL shaders»
    ├── src/                    «source code»
    └── tests/                  «unit tests»
