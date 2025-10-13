Keyboard and Mouse Input
========================

Inexor engine uses `glfw3 <https://www.glfw.org/>`__ for window management and for keyboard and mouse input. Inexor does not use `manual polling <https://www.glfw.org/docs/3.3/group__input.html#ga67ddd1b7dcbbaff03e4a76c0ea67103a>`__ of keyboard and mouse input data, but uses `callbacks <https://www.glfw.org/docs/3.3/input_guide.html#input_keyboard>`__ instead. This way, we ensure we are not missing a key event. For more information check out `glfw's input guide <https://www.glfw.org/docs/3.3/input_guide.html>`__. Inexor uses a wrapper class for keyboard and mouse input data, called ``KeyboardMouseInputData``. This class offers an easy-to-use interface for setting and getting keyboard and mouse input. ``KeyboardMouseInputData`` is thread safe since `pull request 401. <https://github.com/inexorgame/vulkan-renderer/pull/401>`__

.. note::
    Inexor redirects keyboard and mouse input events to class methods which handle it. Because glfw is a C-style API, it is not possible to use class methods directly as callbacks for keyboard and mouse input data. To fix this, we set the glfw window user pointer to the class instance which contains the input callback methods. Then, we use a lambda to set up the class method as callback. All setups are done in ``Application::setup_window_and_input_callbacks``. For more information about this workaround, check out this `Stackoverflow issue <https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback>`__.

.. note::
    It's not possible handle glfw input data in a thread which is separate from the thread which created the corresponding window. For more information, check out this `glfw forum post <https://discourse.glfw.org/t/multithreading-glfw/573>`__.

Keyboard Input
--------------

* We store the pressed keys as a ``std::array<bool, GLFW_KEY_LAST>`` member in ``KeyboardMouseInputData``
* The maximum number of keys is defined by ``GLFW_KEY_LAST``
* If a key is pressed or released, we notify ``KeyboardMouseInputData`` by calling method ``press_key`` and ``release_key``, respectively
* Check if a key is currently pressed by calling method ``is_key_pressed``
* Check if a key was pressed once by calling method ``was_key_pressed_once``

Mouse Input
-----------

* We store the pressed mouse buttons as a ``std::array<bool, GLFW_MOUSE_BUTTON_LAST>`` member in ``KeyboardMouseInputData``
* The maximum number of mouse buttons is defined by ``GLFW_MOUSE_BUTTON_LAST``.
* If a mouse button is pressed or released, we notify ``KeyboardMouseInputData`` by calling method ``press_mouse_button`` and ``release_mouse_button``, respectively
* To update the current cursor position, we call ``set_cursor_pos``
* To get the current cursor position, we call ``get_cursor_pos``
* The change in cursor position can be queried with ``calculate_cursor_position_delta``
* Check if a mouse button is pressed by calling method ``is_mouse_button_pressed``
* Check if a mouse button was pressed once by calling method ``was_mouse_button_pressed_once``

Joysticks
---------

.. note::
    The support for joysticks is work in progress. For further information, read `GitHub pull request 518 <https://github.com/inexorgame/vulkan-renderer/pull/518>`__.

* We do not yet support `joysticks <https://www.glfw.org/docs/3.3/input_guide.html#joystick>`__, but glfw allows us to support them in the future
