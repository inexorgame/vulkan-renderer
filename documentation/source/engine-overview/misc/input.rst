Keyboard and Mouse Input
========================

We use `glfw3 <https://www.glfw.org/>`__ for window and input management with `callbacks <https://www.glfw.org/docs/3.3/input_guide.html#input_keyboard>`__ instead of `manual polling <https://www.glfw.org/docs/3.3/group__input.html#ga67ddd1b7dcbbaff03e4a76c0ea67103a>`__. This way, we ensure we are not missing a key event. For more information check out `glfw's input guide <https://www.glfw.org/docs/3.3/input_guide.html>`__. We use a wrapper class for input named ``Input``. Currently, this supports keyboard, mouse, and one gamepad.

.. note::
    We redirect input events to class methods which handle it. Because glfw is a C-style API, it is not possible to use class methods directly as callbacks for keyboard and mouse input data. To fix this, we set the glfw window user pointer to the class instance which contains the input callback methods. Then, we use a lambda to set up the class method as callback. All setups are done in ``Application::setup_window_and_input_callbacks``. For more information about this workaround, check out this `Stackoverflow issue <https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback>`__.

.. note::
    It's not possible handle glfw input data in a thread which is separate from the thread which created the corresponding window. For more information, check out this `glfw forum post <https://discourse.glfw.org/t/multithreading-glfw/573>`__.

Keyboard and Mouse Input
------------------------

- Keyboard and mouse input data is managed in the ``KeyboardMouseInputData`` class.

Gamepads
--------

- Gamepad input data is managed in the ``GamepadInputData`` class.

.. note::
    We currently support only one gamepad. Refactoring of this is required. See `issue 612 <https://github.com/inexorgame/vulkan-renderer/issues/612>`__.

Joysticks
---------

.. note::
    We do not yet support `joysticks <https://www.glfw.org/docs/3.3/input_guide.html#joystick>`__, but glfw allows us to support them in the future. See `issue 612 <https://github.com/inexorgame/vulkan-renderer/issues/612>`__.
