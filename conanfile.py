from conans import ConanFile, CMake


class InexorConan(ConanFile):
    settings = (
        "os",
        "compiler",
        "build_type",
        "arch"
    )

    requires = (
        "glfw/3.3.2",
        "glm/0.9.9.8",
        "imgui/1.77",
        "nlohmann_json/3.8.0",
        "spdlog/1.7.0",
        "stb/20200203",
        "tinygltf/2.5.0",
        "toml11/3.4.0",
        "vulkan-headers/1.2.172",
        "vulkan-memory-allocator/2.3.0",
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
            self.requires("benchmark/1.5.2")
        if self.options.build_tests:
            self.requires("gtest/1.10.0")

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
