Loading Vulkan with volk meta-loader
====================================

* We use a library called `volk <https://github.com/zeux/volk>`__ as a meta-loader for Vulkan API
* This means we load Vulkan API dynamically at runtime instead of linking statically to Vulkan
* This allows us to check for the available Vulkan API version at runtime during initialization
* If the required version of Vulkan is not available on the system, linking statically would lead to error messages by the operating system which are not in control of the program itself
* There are other `advantages of using volk <https://github.com/zeux/volk?tab=readme-ov-file#optimizing-device-calls>`__, like bypassing the Vulkan loader dispatch code, which can result in performance improvements
* Integration of volk is very easy, and it only affects the initialization of Vulkan API, not the function calls or API itself
* Initialization of volk is carried out in the constructor of the ``Instance`` wrapper class (calling ``volkInitialize`` and ``volkLoadInstanceOnly``) and the ``Device`` wrapper class (calling ``volkLoadDevice``)
