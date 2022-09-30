Documentation
=============

.. note::

    This section needs some love! Open a PR on `GitHub <https://github.com/inexorgame/vulkan-renderer/pulls>`__.

You can link to the Vulkan specification inside the documentation with ``:enum:\`vulkan:<object>\``` or ``:enum:\`<object>\``` for example `` :enum:`vulkan:vkEnumerateDeviceExtensionProperties` ``.

To link in source code comments use

.. codeblock::

    /// \rst
    /// Iam linking to :enum:`vulkan:VkCommandBuffer` or :enum:`VkCommandBuffer`, which is ugly but works!
    /// \endrst

.. note::

    It is mandatory to use the enum role, as this works around some cases where no linkage is happening if it isn't.
