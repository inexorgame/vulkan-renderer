.. _COMMAND_LINE_ARGUMENTS:

Command Line Arguments
======================

.. note::
    This page will be updated when `rendergraph2 pull request 533 <https://github.com/inexorgame/vulkan-renderer/pull/533>`__ will be merged. The information should be summarized in a table.

You can start vulkan-renderer with the following command line arguments:

.. option:: --gpu <index>

    Specifies which GPU to use by array index, **starting from 0**.

.. note:: The engine checks if this index is valid. If the index is invalid, automatic GPU selection rules apply.

.. option:: --vsync

.. warning:: Vsync is currently not implemented. The command line argument will be ignored.

    Enables `vertical synchronization <https://en.wikipedia.org/wiki/Analog_television#Vertical_synchronization>`__ (limits FPS to monitor refresh rate).
