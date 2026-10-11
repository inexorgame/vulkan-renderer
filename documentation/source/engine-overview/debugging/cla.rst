.. _COMMAND_LINE_ARGUMENTS:

Command Line Arguments
======================

You can start vulkan-renderer with the following command line arguments:

Options that take a value accept either ``--option=value`` or
``--option value``. Use ``--help`` to display this list from the executable.

.. option:: --gpu <index>

    Specifies which GPU to use by array index, **starting from 0**.

.. note:: The engine checks if this index is valid and if the selected GPU is suitable for the requested device level. If the GPU is not suitable, automatic GPU selection rules apply (see :ref:`_AUTOMATIC_GPU_SELECTION`).

.. option:: --vsync

    Enables `vertical synchronization <https://en.wikipedia.org/wiki/Analog_television#Vertical_synchronization>`__ (limits FPS to monitor refresh rate).

.. option:: --no-cmd-buf-cache

    Disables the secondary command buffer cache and records graphics passes directly into the primary command buffer.

.. option:: --one-cmd-buf

    Uses one command buffer for render-graph recording.

.. option:: --debug-vma

    Enables Vulkan Memory Allocator debugging and the additional VMA debug controls.

.. option:: --maxfps <fps>

    Limits the maximum frame rate to the specified value when VSync is disabled. The default is ``4000`` FPS.

.. option:: --msaa <samples>

    Sets the multisample anti-aliasing sample count. Supported values are ``1``, ``2``, ``4``, ``8``, and ``16``. Any other value falls back to ``1``.

.. option:: --frames-in-flight <count>

    Sets the number of frames that may be in flight. The value must be from ``1`` to ``3``; the default is ``2``.

.. option:: --threadpool-workers <count>

    Sets the number of Taskflow worker threads. The value must be from ``1`` to ``64``; the default is the number of hardware-concurrency threads.
