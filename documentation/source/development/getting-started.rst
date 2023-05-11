.. _GETTING_STARTED:

Getting started
===============

Also see the :ref:`building instructions<BUILDING>` (:ref:`BUILDING Windows`/:ref:`BUILDING Linux`).

Required Software
-----------------

`Git <https://www.git-scm.com/>`__
    Git for cloning (downloading) the source code.

`Python <https://www.python.org/>`__ with `pip <https://pypi.org/project/pip/>`__
    Required for generating the documentation and the C++ package manager.

`CMake <https://cmake.org/>`__
    The build generator which generates project files for various IDEs.

`Vulkan SDK <https://vulkan.lunarg.com/sdk/home>`__
    Vulkan SDK contains the libraries and tools which are necessary to work with Vulkan API.

    Update your Vulkan SDK as often as possible, because new versions will be released frequently which contains new features and bug fixes.

    Make sure you add the ``glslangValidator`` in the Vulkan SDK's bin folder to your path variable.

Optional Software
-----------------

`GitKraken Git GUI <https://www.gitkraken.com/git-client>`__.
    A Git user interface with many features which is easy to use.

`GitHub Desktop <https://desktop.github.com/>`__
    An open source Git user interface which is easy to use.

`Ninja Build System <https://ninja-build.org/>`__
    Improve your build times with ninja.

`RenderDoc <https://renderdoc.org/>`__
    Powerful open source graphics debugger. Inexor has full RenderDoc integration.

`Doxygen <http://www.doxygen.nl/download.html>`__
    Required for generating the documentation.

`Notepad++ <https://notepad-plus-plus.org/downloads/>`__
    Free and open source text editor.

`Atom.io <https://atom.io/>`__
    Free and open source text editor.

`Visual Studio Code <https://code.visualstudio.com/>`__
    Free and open source text editor.


Does my graphics card support Vulkan?
-------------------------------------

- You can look up your graphics card in `Sascha Willem's Vulkan hardware database <https://vulkan.gpuinfo.org/>`__.
- Every new graphics card which is coming out these days supports Vulkan API.
- Vulkan is also supported on older graphics cards going back to `Radeon HD 7000 series <https://en.wikipedia.org/wiki/Radeon_HD_7000_series>`__ and `Nvidia Geforce 6 series <https://en.wikipedia.org/wiki/GeForce_6_series>`__.


Update your graphics drivers!
-----------------------------

- Update your graphics drivers as often as possible.
- New drivers contain new features, bug fixes, and performance improvements.
