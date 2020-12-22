VMA replays
===========

- For memory management, Inexor uses `AMD's Vulkan Memory Allocator library (VMA) <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__.
- Inexor supports VMA's `internal resource naming system <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator>`__.
- This means we can give every memory region a user-defined name.
- The allocations and deallocations will be tracked in ``vma_replay.csv`` in the folder ``vma-replays``.
- The memory allocations can be replayed using `VMA's memory replay <https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator#binaries>`__.

An example dump (from the `march 2020 texture corruption bug <https://community.khronos.org/t/texture-corruption-when-window-is-resized/105456/6>`__) looks like this:

Note that some resources have already an internal name assigned to it, like ``Example vertex buffer 1``.

.. code-block:: bash

    Vulkan Memory Allocator,Calls recording
    1,8
    Config,Begin
    VulkanApiVersion,1,0
    PhysicalDevice,apiVersion,4198495
    PhysicalDevice,driverVersion,1783087104
    PhysicalDevice,vendorID,4318
    PhysicalDevice,deviceID,4052
    PhysicalDevice,deviceType,2
    PhysicalDevice,deviceName,GeForce GTX 660M
    PhysicalDeviceLimits,maxMemoryAllocationCount,4096
    PhysicalDeviceLimits,bufferImageGranularity,65536
    PhysicalDeviceLimits,nonCoherentAtomSize,64
    PhysicalDeviceMemory,HeapCount,2
    PhysicalDeviceMemory,Heap,0,size,2075918336
    PhysicalDeviceMemory,Heap,0,flags,1
    PhysicalDeviceMemory,Heap,1,size,8265048064
    PhysicalDeviceMemory,Heap,1,flags,0
    PhysicalDeviceMemory,TypeCount,11
    PhysicalDeviceMemory,Type,0,heapIndex,1
    PhysicalDeviceMemory,Type,0,propertyFlags,0
    PhysicalDeviceMemory,Type,1,heapIndex,1
    PhysicalDeviceMemory,Type,1,propertyFlags,0
    PhysicalDeviceMemory,Type,2,heapIndex,1
    PhysicalDeviceMemory,Type,2,propertyFlags,0
    PhysicalDeviceMemory,Type,3,heapIndex,1
    PhysicalDeviceMemory,Type,3,propertyFlags,0
    PhysicalDeviceMemory,Type,4,heapIndex,1
    PhysicalDeviceMemory,Type,4,propertyFlags,0
    PhysicalDeviceMemory,Type,5,heapIndex,1
    PhysicalDeviceMemory,Type,5,propertyFlags,0
    PhysicalDeviceMemory,Type,6,heapIndex,1
    PhysicalDeviceMemory,Type,6,propertyFlags,0
    PhysicalDeviceMemory,Type,7,heapIndex,0
    PhysicalDeviceMemory,Type,7,propertyFlags,1
    PhysicalDeviceMemory,Type,8,heapIndex,0
    PhysicalDeviceMemory,Type,8,propertyFlags,1
    PhysicalDeviceMemory,Type,9,heapIndex,1
    PhysicalDeviceMemory,Type,9,propertyFlags,6
    PhysicalDeviceMemory,Type,10,heapIndex,1
    PhysicalDeviceMemory,Type,10,propertyFlags,14
    Extension,VK_KHR_dedicated_allocation,0
    Extension,VK_KHR_bind_memory2,0
    Extension,VK_EXT_memory_budget,0
    Extension,VK_AMD_device_coherent_memory,0
    Macro,VMA_DEBUG_ALWAYS_DEDICATED_MEMORY,0
    Macro,VMA_DEBUG_ALIGNMENT,1
    Macro,VMA_DEBUG_MARGIN,16
    Macro,VMA_DEBUG_INITIALIZE_ALLOCATIONS,1
    Macro,VMA_DEBUG_DETECT_CORRUPTION,1
    Macro,VMA_DEBUG_GLOBAL_MUTEX,0
    Macro,VMA_DEBUG_MIN_BUFFER_IMAGE_GRANULARITY,1
    Macro,VMA_SMALL_HEAP_MAX_SIZE,1073741824
    Macro,VMA_DEFAULT_LARGE_HEAP_BLOCK_SIZE,268435456
    Config,End
    8644,0.002,0,vmaCreateAllocator
    8644,0.134,0,vmaCreateImage,0,1,126,800,600,1,1,1,1,0,32,0,0,32,1,0,0,0,0000000000000000,0000000007F66FA0,Depth buffer image.
    8644,0.302,0,vmaCreateBuffer,0,4194304,1,0,36,3,0,0,0,0000000000000000,0000000007F66FE8,example_texture_1
    8644,0.304,0,vmaCreateImage,0,1,37,1024,1024,1,1,1,1,0,6,0,0,32,1,0,0,0,0000000000000000,0000000007F67030,example_texture_1
    8644,0.388,0,vmaDestroyBuffer,0000000007F66FE8
    8644,0.652,0,vmaCreateBuffer,0,192,16,0,36,3,0,0,0,0000000000000000,0000000007F66FE8,Uniform buffer #0
    8644,0.663,0,vmaCreateBuffer,0,192,16,0,36,3,0,0,0,0000000000000000,0000000007F67078,Uniform buffer #1
    8644,0.673,0,vmaCreateBuffer,0,192,16,0,36,3,0,0,0,0000000000000000,0000000007F670C0,Uniform buffer #2
    8644,0.737,0,vmaCreateBuffer,0,128,1,0,36,2,0,0,0,0000000000000000,0000000007F67108,Example vertex buffer 1
    8644,0.752,0,vmaCreateBuffer,0,192,1,0,36,2,0,0,0,0000000000000000,0000000007F67150,Example vertex buffer 1
    8644,0.762,0,vmaCreateBuffer,0,128,130,0,36,2,0,0,0,0000000000000000,0000000007F67198,Example vertex buffer 1
    8644,0.767,0,vmaCreateBuffer,0,192,66,0,36,2,0,0,0,0000000000000000,0000000007F671E0,Example vertex buffer 1
    8644,0.817,0,vmaDestroyBuffer,0000000007F67108
    8644,0.821,0,vmaDestroyBuffer,0000000007F67150
    8644,0.842,0,vmaCreateBuffer,0,128,1,0,36,2,0,0,0,0000000000000000,0000000007F67150,Example vertex buffer 2
    8644,0.855,0,vmaCreateBuffer,0,192,1,0,36,2,0,0,0,0000000000000000,0000000007F67108,Example vertex buffer 2
    8644,0.866,0,vmaCreateBuffer,0,128,130,0,36,2,0,0,0,0000000000000000,0000000007F67228,Example vertex buffer 2
    8644,0.870,0,vmaCreateBuffer,0,192,66,0,36,2,0,0,0,0000000000000000,0000000007F67270,Example vertex buffer 2
    8644,0.920,0,vmaDestroyBuffer,0000000007F67150
    8644,0.926,0,vmaDestroyBuffer,0000000007F67108
    8644,1.113,0,vmaDestroyBuffer,0000000007F66FE8
    8644,1.113,0,vmaDestroyBuffer,0000000007F67078
    8644,1.113,0,vmaDestroyBuffer,0000000007F670C0
    8644,1.195,0,vmaCreateImage,0,1,126,800,600,1,1,1,1,0,32,0,0,32,1,0,0,0,0000000000000000,0000000007F670C0,Depth buffer image.
    8644,1.235,0,vmaCreateBuffer,0,192,16,0,36,3,0,0,0,0000000000000000,0000000007F67078,Uniform buffer #0
    8644,1.246,0,vmaCreateBuffer,0,192,16,0,36,3,0,0,0,0000000000000000,0000000007F66FE8,Uniform buffer #1
    8644,1.255,0,vmaCreateBuffer,0,192,16,0,36,3,0,0,0,0000000000000000,0000000007F67108,Uniform buffer #2
    8644,2.432,0,vmaDestroyBuffer,0000000007F67078
    8644,2.432,0,vmaDestroyBuffer,0000000007F66FE8
    8644,2.432,0,vmaDestroyBuffer,0000000007F67108
    8644,2.450,0,vmaDestroyImage,0000000007F67030
    8644,2.469,0,vmaDestroyBuffer,0000000007F67198
    8644,2.474,0,vmaDestroyBuffer,0000000007F671E0
    8644,2.484,0,vmaDestroyBuffer,0000000007F67228
    8644,2.489,0,vmaDestroyBuffer,0000000007F67270
    8644,2.617,0,vmaDestroyAllocator
