Error Handling with Exceptions
==============================

.. note::
    This page will be updated when rendergraph2 will be merged. Read `pull request 533 <https://github.com/inexorgame/vulkan-renderer/pull/533>`__.

We use `exceptions <https://www.cplusplus.com/reference/exception/exception/>`__ as for error handling, as it is proposed by the `C++ core guidelines <https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#S-errors>`__. It should only be used for error handling and mainly in code areas which are not performance critical. It also should only be used for *exceptional errors*, which can't be ignored easily. Exceptions are not without criticism, but all alternatives (like return codes for examples) are not free either. In some areas, as in constructors for example, no return codes can be used at all. A detailed discussion why to use exceptions can be found `here <https://isocpp.org/wiki/faq/exceptions>`__.

Custom Vulkan Exception Classes
-------------------------------

- We use a custom base class for exceptions called ``InexorException`` which inherits from ``std::runtime_error``
- For exceptions which are thrown because a Vulkan function call failed, ``VulkanException`` is used
- ``VulkanException`` constructor takes an error message as ``std::string`` and the ``VkResult`` value
- The constructor of ``VulkanException`` will turn the ``VkResult`` into a human readable error message (like ``VK_ERROR_INITIALIZATION_FAILED``) and a user friendly error description (in this case "Initialization of an object could not be completed for implementation-specific reasons.")
- In order to be able to pass the ``VkResult`` of a Vulkan function call to the exception, it should be stored in a `C++17 if statement with initializer <https://en.cppreference.com/w/cpp/language/if>`__ (see example)
- We use the representation wrapper to turn the ``VkResult`` into its corresponding error text

**Example**

.. code-block:: cpp

    if (const auto result = vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr); result != VK_SUCCESS) {
        throw VulkanException("Error: vkEnumerateInstanceLayerProperties failed!", result);
    }

Example result: ``Error: vkEnumerateInstanceLayerProperties failed! (VK_ERROR_OUT_OF_HOST_MEMORY: A host memory allocation has failed.)``
