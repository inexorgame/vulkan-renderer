.. _COMMAND_LINE_ARGUMENTS:

Command Line Arguments
======================

You can start vulkan-renderer with the following command line arguments:

.. option:: --gpu <index>

    Specifies which GPU to use by array index, **starting from 0**.

.. note:: The engine checks if this index is valid. If the index is invalid, automatic GPU selection rules apply (see :ref:`_AUTOMATIC_GPU_SELECTION`).

.. option:: --vsync

    Enables `vertical synchronization <https://en.wikipedia.org/wiki/Analog_television#Vertical_synchronization>`__ (limits FPS to monitor refresh rate).

.. option:: --maxfps <fps>

    Limits the max frames per seconds to a specified value. The command line argument will be `clamped <https://en.cppreference.com/w/cpp/algorithm/clamp.html>`__ in between the values ``1 fps`` and ``2000 fps`` as lower and higher bounds.
