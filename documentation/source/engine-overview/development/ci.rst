.. _CONTINUOUS_INTEGRATION:

Continuous Integration (CI)
===========================

- The code in the main branch and the build system must remain stable at all time.
- You can see the current status of the main branch in the build batch on the `GitHub repository page <https://github.com/inexorgame/vulkan-renderer>`__:

  |github actions|

- Our `continuous integration <https://en.wikipedia.org/wiki/Continuous_integration>`__ setup allows for automatic building and testing of our software.
- We use `GitHub Actions <https://github.com/features/actions>`__ with the following workflows, which are automatically triggered if code is pushed to the main branch or into a pull request:

.. list-table:: Build and Analysis Workflows
   :header-rows: 1
   :widths: 20 80

   * - **Workflow**
     - **Tools**
   * - Build Code
     - Linux: `gcc <https://gcc.gnu.org/>`__ and `clang <https://clang.llvm.org/>`__ 

       Windows: `Microsoft Visual Studio (MSVC) <https://visualstudio.microsoft.com/>`__ and  
       `Clang/LLVM for MSVC <https://learn.microsoft.com/en-us/cpp/build/clang-support-msbuild?view=msvc-170>`__  

       All compilers build both debug and release configurations
   * - Build Documentation
     - `Python <https://www.python.org/>`__, `Sphinx <https://www.sphinx-doc.org/>`__, and  
       `Doxygen <https://www.doxygen.nl/>`__
   * - Static Code Analysis
     - `clang-tidy <https://clang.llvm.org/extra/clang-tidy/>`__,  
       `scan-build <https://clang.llvm.org/docs/analyzer/user-docs/CommandLineUsage.html#scan-build>`__,  
       `cppcheck <http://cppcheck.sourceforge.net/>`__, and  
       `MSVC code analysis <https://learn.microsoft.com/en-us/visualstudio/code-quality/>`__
   * - Code Formatting Check
     - `clang-format <https://clang.llvm.org/docs/ClangFormat.html>`__
   * - Commit Naming Check
     - `A Python helper script <https://github.com/inexorgame/vulkan-renderer/blob/main/.github/workflows/commit_naming.yml>`__, see also :ref:`COMMIT_NAMING`

- For the main branch, all resulting builds from the CI (so called artifacts) are released on GitHub as `nightly build <https://github.com/inexorgame/vulkan-renderer/releases/tag/nightly>`__.
- We also have a `webhook <https://gist.github.com/jagrosh/5b1761213e33fc5b54ec7f6379034a22>`__ which directly dispatches the build status into our `Discord <https://discord.com/invite/acUW8k7>`__.

.. |github actions| image:: https://img.shields.io/github/actions/workflow/status/inexorgame/vulkan-renderer/build.yml?branch=main
   :target: https://github.com/inexorgame/vulkan-renderer/actions?query=build.yml
