.. _COMMAND_LINE_ARGUMENTS:

Command Line Arguments
======================

You can start vulkan-renderer with the following command line arguments:

.. option:: --gpu <index>

    Specifies which GPU to use by array index, **starting from 0**.

.. note:: The engine checks if this index is valid and if the selected GPU is suitable for the requested device level. If the GPU is not suitable, automatic GPU selection rules apply (see :ref:`_AUTOMATIC_GPU_SELECTION`).

.. option:: --vsync

    Enables `vertical synchronization <https://en.wikipedia.org/wiki/Analog_television#Vertical_synchronization>`__ (limits FPS to monitor refresh rate).

.. option:: --no-cmd-buf-cache

    Disables the secondary command buffer cache and records graphics passes directly into the primary command buffer.

.. option:: --maxfps <fps>

    Limits the max frames per seconds to a specified value. The command line argument will be `clamped <https://en.cppreference.com/w/cpp/algorithm/clamp.html>`__ in between the values ``1 fps`` and ``2000 fps`` as lower and higher bounds.

.. option:: --msaa <samples>

    Sets the multisample anti-aliasing sample count. Supported values are ``1``, ``2``, ``4``, ``8``, and ``16``. Any other value falls back to ``1``.
