Vulkan Validation Layers
========================

- See also: :ref:`VALIDATION_LAYERS`
- It's important to use `Vulkan Validation Layers <https://github.com/KhronosGroup/Vulkan-ValidationLayers>`__ for development.
- Validation Layers are enabled by default in the engine.
- Development without validation layers is pretty much impossible because many bugs are hard to catch otherwise.
- Validation Layers are easily enabled by passing ``VK_LAYER_KHRONOS_validation`` to the list of instance extensions when creating ``VkInstance``.

Vulkan Debug Utilities Extension
--------------------------------

- The engine also uses `Vulkan Debug Utilities Extension <https://docs.vulkan.org/samples/latest/samples/extensions/debug_utils/README.html>`__ (`VK_EXT_debug_utils <https://registry.khronos.org/vulkan/specs/latest/man/html/VK_EXT_debug_utils.html>`__) to register a callback for validation layer messages.
- The validation layer warnings and errors are directly redirected to `spdlog <https://github.com/gabime/spdlog>`__ output, which is logged both to console and to logfile.
- This extension also allows us to have an internal naming system for Vulkan resources (not to be mixed up with VMA's internal naming system).
- Furthermore, with this extension labels can be inserted into command buffers and binary tags can be inserted as well for debugging.
