Automatic Code Formatting
=========================

.. note::
    A pull request will only be accepted if the C++ code is formatted correctly!

- To have one unified C++ code formatting style, we use `clang-format <https://clang.llvm.org/docs/ClangFormat.html>`__.
- Clang-format automatically formats source code according to a set of pre-defined rules.
- Our current style can be found in the `.clang-format file <https://github.com/inexorgame/vulkan-renderer/blob/main/.clang-format>`__ file in the root folder of the `repository <https://github.com/inexorgame/vulkan-renderer>`__:

.. literalinclude:: ../../../../.clang-format
    :language: yaml
    :linenos:

- We recommend to install plugins which auto format the code when the file is being saved.
- Instructions for how to enable clang-format in `Microsoft Visual Studio <https://visualstudio.microsoft.com/>`__ can be found `here <https://learn.microsoft.com/en-us/visualstudio/ide/reference/options-text-editor-c-cpp-formatting?view=vs-2022>`__.
- Other editors like `Visual Studio Code <https://code.visualstudio.com/>`__, `Notepad++ <https://notepad-plus-plus.org/downloads/>`__ and `Sublime Text <https://www.sublimetext.com/>`__ support this as well.
- Part of our `continuous integration (CI) <https://en.wikipedia.org/wiki/Continuous_integration>`__ is an automated clang-format check using `GitHub actions <https://github.com/features/actions>`__.
- Our setup of clang-format with GitHub actions can be `here <https://github.com/inexorgame/vulkan-renderer/blob/main/.github/workflows/code_style.yml>`__.
