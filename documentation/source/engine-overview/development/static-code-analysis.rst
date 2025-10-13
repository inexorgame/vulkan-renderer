Static Code Analysis (SCA)
==========================

.. note::
   Fixing all code warnings from static code analysis is not required for the GitHub actions workflow to succeed. In general, fixing all warnings is very tedious and can be numerous false alerts. In these cases, we should try to add them to some ignore list.

In our continuous integration, we apply the following tools for `static code analysis <https://en.wikipedia.org/wiki/Static_program_analysis>`__ (SCA):

- `clang-tidy <https://clang.llvm.org/extra/clang-tidy/>`__
- `clang scan-build <https://clang.llvm.org/docs/analyzer/user-docs/CommandLineUsage.html#scan-build>`__
- `cppcheck <https://cppcheck.sourceforge.io/>`__
- `msvc code analysis <https://docs.microsoft.com/en-us/visualstudio/code-quality/?view=vs-2022>`__

The status of the static code analysis can be seen in this GitHub badge:

|github actions|

.. |github actions| image:: https://img.shields.io/github/actions/workflow/status/inexorgame/vulkan-renderer/static_analysis.yml?label=static%20code%20analysis
   :target: https://github.com/inexorgame/vulkan-renderer/actions/workflows/static_analysis.yml
   :alt: Static Analysis Status

Our current clang-tidy configuration can be found in the `.clang-tidy file <https://github.com/inexorgame/vulkan-renderer/blob/main/.clang-tidy>`__ file in the root folder of the `repository <https://github.com/inexorgame/vulkan-renderer>`__:

.. literalinclude:: ../../../../.clang-tidy
    :language: yaml
    :linenos:
