from conans import ConanFile, CMake


class InexorConan(ConanFile):
    settings = (
        "os",
        "compiler",
        "build_type",
        "arch"
    )

    requires = (
        "glfw/3.3.7",
        "glm/0.9.9.8",
        "glslang/11.7.0",
        "imgui/1.88",
        "spdlog/1.10.0",
        "stb/cci.20210910",
        "tinygltf/2.5.0",
        "toml11/3.7.1",
        "vulkan-headers/1.3.224.0",
        "vulkan-loader/1.3.224.0",
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

    generators = "cmake"

    def configure(self):
        if self.settings.os == "Linux" and self.settings.compiler.libcxx == "libstdc++":
            raise Exception("Inexor is not compatible with libstdc++. "
                            "Please change the 'compiler.libcxx'-setting "
                            "(e.g. to libstdc++11).")

    def requirements(self):
        if self.options.build_benchmarks:
            self.requires("benchmark/1.6.2")
        if self.options.build_tests:
            self.requires("gtest/1.12.1")

    def imports(self):
        # Copies all dll files from packages bin folder to my "bin" folder (win)
        self.copy("*.dll", dst="benchmarks/Debug", src="bin")
        self.copy("*.dll", dst="benchmarks/Release", src="bin")
        self.copy("*.dll", dst="benchmarks/MinSizeRel", src="bin")
        self.copy("*.dll", dst="benchmarks/RelWithDebInfo", src="bin")
        self.copy("*.dll", dst="example/Debug", src="bin")
        self.copy("*.dll", dst="example/Release", src="bin")
        self.copy("*.dll", dst="example/MinSizeRel", src="bin")
        self.copy("*.dll", dst="example/RelWithDebInfo", src="bin")
        self.copy("*.dll", dst="tests/Debug", src="bin")
        self.copy("*.dll", dst="tests/Release", src="bin")
        self.copy("*.dll", dst="tests/MinSizeRel", src="bin")
        self.copy("*.dll", dst="tests/RelWithDebInfo", src="bin")
        # Copies all so files from packages lib folder to my "lib" folder (linux)
        self.copy("*.so*", dst="lib", src="lib")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
