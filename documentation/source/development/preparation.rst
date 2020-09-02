Preparation
===========

Required Software
-----------------

`Git <https://www.git-scm.com/>`__
    Git to download and manage the source code.

`Conan <https://conan.io/>`__
    C++ Package Manager to get required libraries.

`CMake <https://cmake.org/>`__
    Build generator and used in this project.

`Vulkan SDK <https://vulkan.lunarg.com/sdk/home>`__
    Vulkan is a new generation graphics and compute API that provides high-efficiency, cross-platform access to modern GPUs used in a wide variety of devices from PCs and consoles to mobile phones and embedded platforms.
    Make sure you add the ``glslangValidator`` in the Vulkan SDK's bin folder to your path variable.

Optional Software
-----------------

`Ninja Build System <https://ninja-build.org/>`__
    Improve your build times with ninja.

`Doxygen <http://www.doxygen.nl/download.html>`__
    Required by the documentation. Must be available from command line.

`Sphinx <https://www.sphinx-doc.org>`__ & `sphinx_rtd_theme <https://github.com/readthedocs/sphinx_rtd_theme>`__ & `Exhale <https://github.com/svenevs/exhale>`__ & `sphinxcontrib-mermaid <https://github.com/mgaitan/sphinxcontrib-mermaid>`__
    Required by the documentation. Install with ``pip install sphinx sphinx_rtd_theme exhale sphinxcontrib-mermaid``.
    Do not to forget to change the hard coded ``VERSION`` value in ``Python38\Lib\site-packages\sphinxcontrib`` to the newest available mermaid version.
