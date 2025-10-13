.. _AUTOMATIC_GPU_SELECTION:

Automatic GPU Selection 
=======================

- In Vulkan API, the word physical device is a more general word for all types of graphics cards, integrated gpus and more
- If multiple physical devices are available on the system, Inexor engine is able to pick the most suitable one automatically
- The user can specify a preferred graphics card index with the command line argument ``--gpu <index>`` (starting with index ``0``!)
- If a preferred index is specified, the engine will verify if the index is valid and pick the physical device if it is suitable
- If the physical device specified by the user is not suitable because of technical reasons, automatic selection rules apply
- The engine calculates a score for every available physical device based on the device type and total video memory size
- If no physical devices are available or no suitable physical device could be chosen, an exception is thrown
