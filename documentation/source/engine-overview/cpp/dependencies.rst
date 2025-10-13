Dependencies
============

* The C++ dependencies are directly downloaded by `CMake <https://cmake.org/>`__ with `FetchContent_Declare <https://cmake.org/cmake/help/latest/module/FetchContent.html#command:fetchcontent_declare>`__ and `FetchContent_MakeAvailable <https://cmake.org/cmake/help/latest/module/FetchContent.html#command:fetchcontent_makeavailable>`__
* All required C++ dependencies are listed in ``dependencies.cmake`` in the root of the `GitHub repository <https://github.com/inexorgame/vulkan-renderer.git>`__, which contains the following:

.. literalinclude:: ../../../../dependencies.cmake
   :language: cmake
   :linenos:

.. note::
   Early versions of this project used `conan package manager <https://conan.io/>`__ package manager for downloading and managing C++ dependencies. However, we removed conan package manager from the project in `pull request 528 <https://github.com/inexorgame/vulkan-renderer/pull/528>`__.

Library Selection Criteria
--------------------------

In general we try to keep the number of dependencies at a minimum. If we really need a new library, it should be selected based on the following considerations:

- Are you sure we need this library? Can't we solve the problem with C++ standard library or some other library we are already using?
- The library must have an open source license which is accepted by us (see :ref:`Licenses <LICENSES>`).
- It must be in active development.
- It must have a good documentation.
- A sufficient number of participants must have contributed so we can be sure it is reviewed.
