Continuous integration
======================

- The main branch and the build system must stay stable at all time.

- You can see the current status of the main branch in the build badge:

  |github actions|

- Currently we are using `GitHub Actions <https://github.com/features/actions>`__ for building with `gcc <https://gcc.gnu.org/>`__, `clang <https://clang.llvm.org/>`__ and `Microsoft Visual Studio <https://visualstudio.microsoft.com/en/downloads/>`__ on every push or pull request.

- We also have a `webhook <https://gist.github.com/jagrosh/5b1761213e33fc5b54ec7f6379034a22>`__ which directly dispatches the build status into our `Discord <https://discord.com/invite/acUW8k7>`__. This allows us to spot and fix broken code easily.

.. Badges.

.. |github actions| image:: https://img.shields.io/github/workflow/status/inexorgame/vulkan-renderer/Build
   :target: https://github.com/inexorgame/vulkan-renderer/actions?query=workflow%3A%22Build%22
