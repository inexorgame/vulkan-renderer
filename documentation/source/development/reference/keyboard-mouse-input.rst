Keyboard and mouse input
========================

Inexor engine uses `glfw3 <https://www.glfw.org/>`__ for window management and for keyboard and mouse input. Inexor does not use `manual polling <https://www.glfw.org/docs/3.3/group__input.html#ga67ddd1b7dcbbaff03e4a76c0ea67103a>`__ of keyboard and mouse input data, but uses `callbacks <https://www.glfw.org/docs/3.3/input_guide.html#input_keyboard>`__ instead. This way, we ensure we are not missing a key event. For more information check out `glfw's input guide <https://www.glfw.org/docs/3.3/input_guide.html>`__. Inexor uses a wrapper class for keyboard and mouse input data, called ``KeyboardMouseInputData``. This class offers an easy-to-use interface for setting and getting keyboard and mouse input.

.. note::

    Inexor redirects keyboard and mouse input events to class methods which handle it. Because glfw is a C-style API, it is not possible to use class methods directly as callbacks for keyboard and mouse input data. To fix this, we set the glfw window user pointer to the class instance which contains the input callback methods. Then, we use a lambda to set up the class method as callback. All setups are done in ``Application::setup_window_and_input_callbacks``. For more information about this workaround, check out this `Stackoverflow issue <https://stackoverflow.com/questions/7676971/pointing-to-a-function-that-is-a-class-member-glfw-setkeycallback>`__.

.. note::

    Currently, ``KeyboardMouseInputData`` is not thread safe! It is important to note that it's not possible handle glfw input data in a thread which is separate from the thread which created the corresponding window. For more information, check out this `glfw forum post <https://discourse.glfw.org/t/multithreading-glfw/573>`__.

Keyboard input
--------------

* We store the pressed keys as a ``std::unordered_map<std::int32_t, bool>`` member in ``KeyboardMouseInputData``
* If a key is pressed or released, we notify ``KeyboardMouseInputData`` by calling method ``press_key`` and ``release_key``, respectively
* Check if a key is currently pressed by calling method ``is_key_pressed``
* Check if a key was pressed once by calling method ``was_key_pressed_once``

Mouse input
-----------

* We store the pressed mouse buttons as a ``std::unordered_map<std::int32_t, bool>`` member in ``KeyboardMouseInputData``.
* If a mouse button is pressed or released, we notify ``KeyboardMouseInputData`` by calling method ``press_mouse_button`` and ``release_mouse_button``, respectively
* To update the current cursor position, we call ``set_cursor_pos``
* To get the current cursor position, we call ``get_cursor_pos``
* The change in cursor position can be queried with ``calculate_cursor_position_delta``
* Check if a mouse button is pressed by calling method ``is_mouse_button_pressed``
* Check if a mouse button was pressed once by calling method ``was_mouse_button_pressed_once``

Joysticks
---------

Inexor does not support `joysticks <https://www.glfw.org/docs/3.3/input_guide.html#joystick>`__ yet.
