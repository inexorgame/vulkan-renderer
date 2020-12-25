Clang format
============

- In order to have one unified code formatting style, we use `clang-format <https://clang.llvm.org/docs/ClangFormat.html>`__.
- Clang-format automatically formats source code according to a set of rules which a project needs to agree on.
- Our current style can be found in the `clang-format file <https://github.com/inexorgame/vulkan-renderer/blob/master/.clang-format>`__ file in the root folder of the `repository <https://github.com/inexorgame/vulkan-renderer>`__.
- We recommend to install plugins which auto format the code when the file is being saved.
- Instructions for how to enable clang-format in `Microsoft Visual Studio <https://visualstudio.microsoft.com/>`__ can be found `here <https://devblogs.microsoft.com/cppblog/clangformat-support-in-visual-studio-2017-15-7-preview-1/>`__.
- Other editors like `Visual Studio Code <https://code.visualstudio.com/>`__, `Atom.io <https://atom.io/>`__, `Notepad++ <https://notepad-plus-plus.org/downloads/>`__ and `Sublime Text <https://www.sublimetext.com/>`__ support this as well.
- Part of our `Continuous Integration (CI) <https://en.wikipedia.org/wiki/Continuous_integration>`__ are automated clang-format checks using `GitHub actions <https://github.com/features/actions>`__.
- Our setup of clang-format with GitHub actions can be `here <https://github.com/inexorgame/vulkan-renderer/blob/master/.github/workflows/code_style.yml>`__.
- A pull request will only be accepted if it follows those code formatting rules.

**Example of clang-format checking a pull request along with gcc/clang/msvc build**:

.. image:: clang-format-and-ci-example.jpg
    :alt: Example of Continuous Integration (CI) at work.
