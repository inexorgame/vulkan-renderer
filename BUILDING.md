# Building vulkan-renderer

## Windows

* It is recommended to use [Visual Studio 2019](https://visualstudio.microsoft.com/). You can use any IDE that CMake can generate a project map for.
* Download [CMake](https://cmake.org/).
* Download [Python](https://www.python.org/).
* Install [Conan package manager](https://conan.io/) using `pip install conan`. You can also use the Windows installer.
* We recommend [GitHub Desktop](https://desktop.github.com/ to clone the code.
* Open CMake and select the root folder which contains `CMakeLists.txt` (not just src folder!).
* You can choose any location for the `build` folder.
* Click "Configure" and select `Visual Studio 16 2019`. Click "Finish".
* CMake will now set up dependencies automatically for you. This might take a while.
* Once it has finished, click "Generate". You can now open the Visual Studio project file in your `build`folder.

## Linux
* [We are working on it](https://github.com/inexorgame/vulkan-renderer/issues/19).

## Mac
Currently, we do not support Mac because it would require us to use [MoltenVK](https://github.com/KhronosGroup/MoltenVK) to get Vulkan running on Mac OS.

## Android
We might support Android in the future.
