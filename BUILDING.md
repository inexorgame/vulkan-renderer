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

### Gentoo
* Install dependencies and tools:
```bash
emerge \
 dev-util/cmake \
 dev-util/conan \
 dev-util/vulkan-headers \
 dev-util/vulkan-tools \
 dev-vcs/git \
 media-libs/vulkan-layers \
 media-libs/vulkan-loader
```
* Install ninja build tool (optional):
```bash
emerge dev-util/ninja
```
* Clone the repository:
```bash
git clone https://github.com/inexorgame/vulkan-renderer
cd vulkan-renderer
```
* Create build directory:
```bash
mkdir build
cd $_
```
* Configure cmake:
    * `INEXOR_USE_VMA_RECORDING` is required to be `OFF` in linux builds.
    * Only pass `-GNinja` if the ninja build tool is installed.
```bash
cmake .. \
 -DCMAKE_BUILD_TYPE=Debug \
 -DINEXOR_USE_VMA_RECORDING=OFF \
 -GNinja
```
* Build and run:
```bash
cd ..
cmake --build build --target inexor-vulkan-renderer-example
./build/bin/inexor-vulkan-renderer-example
```

### Other distributions
* [We are working on it](https://github.com/inexorgame/vulkan-renderer/issues/19).

## Mac
Currently, we do not support Mac because it would require us to use [MoltenVK](https://github.com/KhronosGroup/MoltenVK) to get Vulkan running on Mac OS.

## Android
We might support Android in the future.
