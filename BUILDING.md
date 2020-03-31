# Building vulkan-renderer

## Windows

* It is recommended to use [Visual Studio 2019](https://visualstudio.microsoft.com/). You can use any IDE that CMake can generate a project map for.
* Download the latest [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/). It is updated frequently!
* Download [CMake](https://cmake.org/).
* Download [Python](https://www.python.org/).
* Install [Conan package manager](https://conan.io/) using `pip install conan`. You can also use the Windows installer.
* Clone the source code. We recommend [GitHub Desktop](https://desktop.github.com/).
* Open CMake and select the root folder which contains `CMakeLists.txt` (not just `src` folder!).
* You can choose any location for the `build` folder.
* Click "Configure" and select `Visual Studio 16 2019`. Click "Finish".
* CMake will now set up dependencies automatically for you. This might take a while. If this fails, you really should open a ticket!
* Click "Generate". You can now open the Visual Studio project file in your `build` folder.
* You must compile all `.glsl` shaders in the `shaders` folder to the [SPIR-V](https://en.wikipedia.org/wiki/Standard_Portable_Intermediate_Representation) format. Therefore, you need to execute `compile_shaders.bat`. Make sure you have found  `glslangValidator.exe` in the Vulkan SDK's bin folder and add it to your path variable.
* Please check that the root directory of the repository is set as working directory for debugging. Usually, CMake should take care of this already.
* You are now ready to start debugging! We want master branch to stay stable at all time.

## Linux
* [We are working on it](https://github.com/inexorgame/vulkan-renderer/issues/19).

## Mac
Currently, we do not support Mac because it would require us to use [MoltenVK](https://github.com/KhronosGroup/MoltenVK) to get Vulkan running on Mac OS.

## Android
We might support Android in the future.
