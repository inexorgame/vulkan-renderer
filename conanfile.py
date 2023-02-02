from conans import ConanFile, CMake


class InexorConan(ConanFile):
    settings = (
        "os",
        "compiler",
        "build_type",
        "arch"
    )

    requires = (
        "glfw/3.3.8",
        "glm/cci.20220420",
        "glslang/1.3.236.0",
        "imgui/1.89.2",
        "spdlog/1.11.0",
        "stb/cci.20220909",
        "tinygltf/2.5.0",
        "toml11/3.7.1",
        "vulkan-headers/1.3.236.0",
        "vulkan-loader/1.3.236.0",
        "vulkan-memory-allocator/3.0.1",
    )

    options = {
        "build_benchmarks": [True, False],
        "build_tests": [True, False],
    }
    default_options = {
        "build_benchmarks": False,
        "build_tests": False,
    }

    generators = "cmake_find_package"

    def configure(self):
        if self.settings.os == "Linux" and self.settings.compiler.libcxx == "libstdc++":
            raise Exception("Inexor is not compatible with libstdc++. "
                            "Please change the 'compiler.libcxx'-setting "
                            "(e.g. to libstdc++11).")

    def requirements(self):
        if self.options.build_benchmarks:
            self.requires("benchmark/1.7.1")
        if self.options.build_tests:
            self.requires("gtest/1.13.0")

    def imports(self):
        # Copies all dll files from packages bin folder to my "bin" folder (win)
        self.copy("*.dll", dst="bin", src="bin")
        # Copies all dylib files from packages lib folder to my "lib" folder (macosx)
        self.copy("*.dylib*", dst="lib", src="lib")
        # Copies all so files from packages lib folder to my "lib" folder (linux)
        self.copy("*.so*", dst="lib", src="lib")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
