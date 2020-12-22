Continuous Integration
======================

The master branch and its build system must stay stable at all cost!
If this is not the case, people will not have the chance to build and experiment with our project.
Every merge into master branch must result in a stable build.

`Continuous Integration <https://en.wikipedia.org/wiki/Continuous_integration>`__ allows for automatic building and testing of our software.
Currently we are using `GitHub <https://github.com/features/actions>`__ actions for building with `gcc <https://gcc.gnu.org/>`__, `clang <https://clang.llvm.org/>`__ and `Microsoft Visual Studio <https://visualstudio.microsoft.com/en/downloads/>`__ on every push or pull request.
This allows us to spot broken commits easily.
