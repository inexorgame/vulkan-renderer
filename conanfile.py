from conans import ConanFile, CMake


class InexorConan(ConanFile):
    settings = (
        "os",
        "compiler",
        "build_type",
        "arch"
    )

    requires = (
        "glm/0.9.9.7",
        "boost/1.72.0",
        "spdlog/1.5.0",
        "glfw/3.3.2@bincrafters/stable",
        "toml11/3.1.0",
        "imgui/1.75",
        "stb/20200203",
        "nlohmann_json/3.7.3",
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

    def requirements(self):
        if self.options.build_benchmarks:
            self.requires("benchmark/1.5.0")
        if self.options.build_tests:
            self.requires("gtest/1.10.0")

    def imports(self):
        # Copies all dll files from packages bin folder to my "bin" folder (win)
        self.copy("*.dll", dst="bin", src="bin")
        # Copies all dylib files from packages lib folder to my "lib" folder (macosx)
        self.copy("*.dylib*", dst="lib", src="lib") # From lib to lib
        # Copies all so files from packages lib folder to my "lib" folder (linux)
        self.copy("*.so*", dst="lib", src="lib") # From lib to lib

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
