.. _COMMAND_LINE_ARGUMENTS:

Command Line Arguments
======================

.. note::
    This page will be updated when `rendergraph2 pull request 533 <https://github.com/inexorgame/vulkan-renderer/pull/533>`__ will be merged. The information should be summarized in a table.

You can start vulkan-renderer with the following command line arguments:

.. option:: --gpu <index>

    Specifies which GPU to use by array index, **starting from 0**.

.. note:: The engine checks if this index is valid. If the index is invalid, automatic GPU selection rules apply.

.. option:: --no-separate-data-queue

    Disables the use of the special `data transfer queue <https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#devsandqueues-queues>`__ (forces use of the graphics queue).

.. warning:: Enabling this option could decrease the overall performance. Don't enable this option unless you have to.

.. option:: --no-validation

    Disables `Vulkan validation layers <https://github.com/KhronosGroup/Vulkan-ValidationLayers>`__.

.. warning:: You should never disable validation layers because they offer extensive error checks for debugging.

.. option:: --no-vk-debug-markers

    Disables `Vulkan debug markers <https://www.saschawillems.de/blog/2016/05/28/tutorial-on-using-vulkans-vk_ext_debug_marker-with-renderdoc/>`__ (even if ``--renderdoc`` is specified).

.. option:: --renderdoc

    Enables the `RenderDoc <https://renderdoc.org/>`__ debug layer.

.. option:: --vsync

.. warning:: Vsync is currently not implemented. The command line argument will be ignored.

    Enables `vertical synchronization <https://en.wikipedia.org/wiki/Analog_television#Vertical_synchronization>`__ (limits FPS to monitor refresh rate).
